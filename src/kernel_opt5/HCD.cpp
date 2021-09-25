#include "HCD.h"
#include "cmath"

#include <iostream>
using namespace std;

const int unroll_factor = N;

/**
 * Gaussian filter with sigma 1
 * **/
template<typename P, typename BUF>
P Gaussian_filter_1(BUF* buffer, int col, int max_col)
{
    // #pragma HLS function_instantiate variable=row
    #pragma HLS INLINE
    char i,j;
    ap_fixed<27, 17> sum = 0;
    P pixel;

    ap_fixed<12, 1> op[3][3] =
    {
        {0.0751136, 0.123841, 0.0751136},
        {0.123841, 0.20418, 0.123841},
        {0.0751136, 0.123841, 0.0751136}
    };

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            int rc = col-j;
            sum += op[i][j] * ((rc == max_col || rc == -1) ? P(0) : buffer-> getval(i, rc/N)[rc%N] );
        }
    }
    pixel = P(sum);
    return pixel;
}

template<typename P, typename BUF>
P getval_N(BUF* buffer, int i, int j)
{
#pragma HLS INLINE
    return buffer-> getval(i, j/N)[j%N];
}

void process_input(stream_t* pstrmInput, stream_t* stream_gray, int row, int col)
{
    int i;
    int j;
    PIXEL_vec input ;
    PIXEL_vec input_gray_pix;

    for (i = 0; i < row; ++i) {
    #pragma HLS loop_tripcount max=256
        for (j = 0; j < col; ++j) {
    #pragma HLS loop_tripcount max=256

    #pragma HLS pipeline
    #pragma HLS UNROLL factor=unroll_factor skip_exit_check
            if (j % N != 0) continue;

            input = pstrmInput->read();
            for(int k = 0 ; k < N ; ++k) {
                input_gray_pix[k] = (input[k].range(7,0)
                        + input[k].range(15,8)
                        + input[k].range(23,16))/3;
            }
            input = input_gray_pix;
            stream_gray->write(input);
        }
    }
}

void blur_img(stream_t* stream_gray, stream_t* stream_blur, int row, int col)
{
    int i;
    int j;
    PIXEL_vec   blur;
    PIXEL_vec   input;
    BUFFER_3    buf;

    for (i = 0 ; i < row+1; i++) {
    #pragma HLS loop_tripcount max=257
        for (j = 0; j < col+1; j++) {
    #pragma HLS loop_tripcount max=257

    #pragma HLS UNROLL factor=unroll_factor
    #pragma HLS pipeline enable_flush
    #pragma HLS dependence variable=buf type=inter dependent=false

            bool factor_N = (j % N == 0) ? 1 : 0;

            if (factor_N & (j < col)) {
                    buf.shift_down(v_ind(j));
                if (i < row) {
                    input = stream_gray->read();
                    buf.insert_top(input, v_ind(j));
                }
            }

            if(i == 0 || j == 0) continue;

            blur[v_ind_ind(j-1)] = Gaussian_filter_1<PIXEL, BUFFER_3>(&buf, j, col);

            if(factor_N)
                stream_blur->write(blur);
        }
    }
}

void compute_dif(stream_t* stream_blur, stream_t* stream_Ixx,
        stream_t* stream_Iyy, stream_t* stream_Ixy, int row, int col)
{
    int i;
    int j;
    PIXEL       zero = 0;
    PIXEL_vec 	input;
    PIXEL_vec   Ix, Iy;
    PIXEL_vec   Ixx, Iyy, Ixy;
    BUFFER_3    buf;

    for (i = 0 ; i < row+1; i++) {
    #pragma HLS loop_tripcount max=257
        for (j = 0; j < col+1; j++) {
    #pragma HLS loop_tripcount max=257

    #pragma HLS UNROLL factor=unroll_factor
    #pragma HLS pipeline enable_flush

            bool factor_N = (j % N == 0) ? 1 : 0;

            if (factor_N & (j < col)) {
                    buf.shift_down(v_ind(j));
                if (i < row) {
                    input = stream_blur->read();
                    buf.insert_top(input, v_ind(j));
                }
            }

            if (j == 0 || i == 0) continue;
            int vec_index = v_ind_ind(j-1);
            if (j == 1 | j == col)
                Ix[vec_index] = zero;
            else
                Ix[vec_index] = getval_N<PIXEL, BUFFER_3>(&buf, 1, j-2) - getval_N<PIXEL, BUFFER_3>(&buf, 1, j);
            if (i == 1 | i == row)
                Iy[vec_index] = zero;
            else
                Iy[vec_index] = getval_N<PIXEL, BUFFER_3>(&buf, 2, j-1) - getval_N<PIXEL, BUFFER_3>(&buf, 0, j-1);

                Ixx[vec_index] = Ix[vec_index] * Ix[vec_index];
                Iyy[vec_index] = Iy[vec_index] * Iy[vec_index];
                Ixy[vec_index] = Ix[vec_index] * Iy[vec_index];

            if(factor_N) {
                stream_Ixx->write(Ixx);
                stream_Iyy->write(Iyy);
                stream_Ixy->write(Ixy);
            }
        }
    }
}


void blur_diff(stream_t* stream_Ixx, stream_t* stream_Iyy, stream_t* stream_Ixy,
        stream_t* stream_Sxx, stream_t* stream_Syy, stream_t* stream_Sxy,
        int row, int col)
{
    blur_img(stream_Ixx, stream_Sxx, row, col);
    blur_img(stream_Iyy, stream_Syy, row, col);
    blur_img(stream_Ixy, stream_Sxy, row, col);
}

void compute_det_trace(stream_t* stream_Sxx, stream_t* stream_Syy, stream_t* stream_Sxy,
        stream_t* stream_response, int row, int col)
{
    int i;
    int j;
    PIXEL_vec Sxx, Sxy, Syy;
    PIXEL_vec tmp;
    PIXEL_vec det, trace, response;
    for (i = 0; i < row; ++i) {
    #pragma HLS loop_tripcount max=256
        for (j = 0; j < col; ++j) {
    #pragma HLS loop_tripcount max=256

    #pragma HLS UNROLL factor=unroll_factor
    #pragma HLS pipeline enable_flush

            if(j % N == 0) {
                Sxx = stream_Sxx->read();
                Sxy = stream_Sxy->read();
                Syy = stream_Syy->read();
            }

            int vec_index = v_ind_ind(j);

            trace[vec_index] = Sxx[vec_index] * Syy[vec_index];
            tmp[vec_index] =  Sxy[vec_index] * Sxy[vec_index];
            det[vec_index] = trace[vec_index] - tmp[vec_index];

            response[vec_index] = det[vec_index] - ap_fixed<12, 2>(0.05) * trace[vec_index];

            if (response[vec_index] > 6000000)
                response[vec_index] = PIXEL(response[vec_index]);
            else
                response[vec_index] = PIXEL(0);

            if((j % N) == (N-1)) {
                stream_response->write(response);
            }
        }
    }
}

void find_local_maxima(stream_t* stream_response, stream_t* pstrmOutput, int row, int col)
{
    PIXEL_vec   input, output;
    PIXEL_vec   center_pixel;
    BUFFER_5    buf;

    loop_i: for (int i = 0 ; i < row+2; i++) {
    #pragma HLS loop_tripcount max=258
        loop_j: for (int j = 0; j < col+2; j++) {
    #pragma HLS loop_tripcount max=258

    #pragma HLS UNROLL factor=unroll_factor
    #pragma HLS pipeline

            if ( (j % N == 0) & (j < col)) {
                    buf.shift_down(v_ind(j));
                if (i < row) {
                    input = stream_response->read();
                    buf.insert_top(input, v_ind(j));
                }
            }

            if (i < 2 || j < 2) continue;

            int vec_index = v_ind_ind(j-2);

            center_pixel[vec_index] = getval_N<PIXEL, BUFFER_5>(&buf, 2, j-2);
            output[vec_index] = 0;
            if (center_pixel[vec_index] != 0 && i > 3 && j > 3 && i < row && j < col) {
                output[vec_index] = 1;
                loop_sj: for(int sj = 4; sj >= 0; sj--) {
                    loop_si: for(int si = 0 ; si < 5; si++) {
                        if(getval_N<PIXEL, BUFFER_5>(&buf, si, j-sj) > center_pixel[vec_index]) {
                            output[vec_index] = 0;
                            break;
                        }
                    }
                }
            }

            if( ((j % N) == 1) ) {
                pstrmOutput-> write(output);
            }
        }
    }
}

void HCD(stream_t* pstrmInput, stream_t* pstrmOutput, int row, int col)
{
#pragma HLS INTERFACE s_axilite port=row
#pragma HLS INTERFACE s_axilite port=col
#pragma HLS INTERFACE axis register both port=pstrmOutput
#pragma HLS INTERFACE axis register both port=pstrmInput
#pragma HLS INTERFACE s_axilite port=return

    int i;
    int j;

    stream_t stream_gray;
    stream_t stream_blur;
    stream_t stream_Ixx;
    stream_t stream_Ixy;
    stream_t stream_Iyy;
    stream_t stream_Sxx;
    stream_t stream_Sxy;
    stream_t stream_Syy;
    stream_t stream_response;

#pragma HLS DATAFLOW
    process_input(pstrmInput, &stream_gray, row, col);

    // Step 1: Smooth the image by Gaussian kernel
    blur_img(&stream_gray, &stream_blur, row, col);

    // Step 2: Calculate Ix, Iy (1st derivative of image along x and y axis)
    // Step 3: Compute Ixx, Ixy, Iyy (Ixx = Ix*Ix, ...)
    compute_dif(&stream_blur, &stream_Ixx, &stream_Iyy,
            &stream_Ixy, row, col);

    // Step 4: Compute Sxx, Sxy, Syy (weighted summation of Ixx, Ixy, Iyy in neighbor pixels)
    blur_diff(&stream_Ixx, &stream_Iyy, &stream_Ixy,
        &stream_Sxx, &stream_Syy, &stream_Sxy, row, col);

    // Step 5: Compute the det and trace of matrix M (M = [[Sxx, Sxy], [Sxy, Syy]])
    // Step 6: Compute the response of the detector by det/(trace+1e-12)
    compute_det_trace(&stream_Sxx, &stream_Syy, &stream_Sxy, &stream_response, row, col);

    // Step 7: Post processing
    find_local_maxima(&stream_response, pstrmOutput, row, col);

    return;
}