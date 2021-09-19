#include "HCD.h"
#include "cmath"

/**
 * Gaussian filter with sigma 1
 * **/
template<typename P, typename W>
P Gaussian_filter_1(W* window)
{
    char i,j;
    ap_fixed<27, 17> sum = 0;
    P pixel =0;

    ap_fixed<12, 1> op[3][3] =
    {
        {0.0751136, 0.123841, 0.0751136},
        {0.123841, 0.20418, 0.123841},
        {0.0751136, 0.123841, 0.0751136}
    };

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            #pragma HLS pipeline
            sum += window->getval(i,j) * op[i][j];
        }
    }
    pixel = P(sum);
    return pixel;
}

void process_input(stream_t* pstrmInput, stream_t* stream_gray,
    int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    PIXEL_vec input;
    PIXEL_vec input_gray_pix;
    for (i = 0; i < row; ++i) {
    #pragma HLS loop_tripcount max=256
        for (j = 0; j < col/N; ++j) {
    #pragma HLS loop_tripcount max=128
    #pragma HLS pipeline
            input = pstrmInput-> read();
            for(int n_vec = 0; n_vec < N; ++n_vec) {
                input_gray_pix[n_vec] = (input[n_vec].range(7,0)
                        + input[n_vec].range(15,8)
                        + input[n_vec].range(23,16))/3;

                input[n_vec] = input_gray_pix[n_vec];
            }
            stream_gray->write(input);
        }
    }
}

void blur_img(stream_t* stream_gray, stream_t* stream_blur, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    PIXEL_vec   input;
    PIXEL_vec   blur;
    hls::vector<WINDOW, N>  window;
    BUFFER_3    buf;

    for (i = 0 ; i < row+1; i++) {
    #pragma HLS loop_tripcount max=257
        for (j = 0; j < col+1; j += N) {
    #pragma HLS loop_tripcount max=128
    #pragma HLS pipeline enable_flush

            if ( i < row & j < col )
                input = stream_gray->read();

            // buffer update
            for (int k = 0 ; k < N ; k++) {
            #pragma HLS UNROLL
                int nc = j+k; // now column
                if ( nc < col )
                    buf.shift_up(nc);
                if (i < row & nc < col) {
                    buf.insert_bottom(input[k], nc);
                }
            }

            // init window
            if (i == 0) continue;
            if (j == 0) {
                window[0].insert(buf.getval(1, 0), 1, 2);
                window[0].insert(input[0], 2, 2);

                window[1].insert(buf.getval(1, 0), 1, 1);
                window[1].insert(input[0], 2, 1);
                window[1].insert(buf.getval(1, 1), 1, 2);
                window[1].insert(input[1], 2, 2);
                continue;
            }


            for (int k = 0 ; k < N ; k++) {
            #pragma HLS UNROLL
                int nc = j+k-1; // now column

                window[k].shift_right();
                if (nc < col) {
                    window[k].insert(buf.getval(2, nc), 0, 2);
                    window[k].insert(buf.getval(1, nc), 1, 2);
                    window[k].insert(buf.getval(0, nc), 2, 2);
                }

                if(nc == 1)
                    window[k].rreflect();
                else if (nc == col)
                    window[k].lreflect();

                if (i == 1)
                    window[k].dreflect();
                else if (i == row)
                    window[k].ureflect();

                blur[k] = Gaussian_filter_1<PIXEL, WINDOW >(&window[k]);

                input[k] = blur[k];

                window[k].shift_right();
                nc++;
                if (nc < col) {
                    window[k].insert(buf.getval(2, nc), 0, 2);
                    window[k].insert(buf.getval(1, nc), 1, 2);
                    window[k].insert(buf.getval(0, nc), 2, 2);
                }
            }
            // fix me
            stream_blur->write(input);
        }
    }
}

void compute_dif(stream_t* stream_blur, stream_t* stream_Ixx,
        stream_t* stream_Iyy, stream_t* stream_Ixy, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    PIXEL       zero = 0;
    PIXEL_vec   Ix, Iy;
    PIXEL_vec   Ixx, Iyy, Ixy;
    PIXEL_vec   input;
    BUFFER_3    blur_buf;

    for (i = 0 ; i < row+1; i++) {
    #pragma HLS loop_tripcount max=257
        for (j = 0; j < col+1; j += N) {
    #pragma HLS loop_tripcount max=128
    #pragma HLS pipeline enable_flush

            if ( i < row & j < col )
                input = stream_blur->read();

            // buffer update
            for (int k = 0 ; k < N ; k++) {
            #pragma HLS UNROLL
                int nc = j+k; // now column
                if ( nc < col )
                    blur_buf.shift_up(nc);
                if (i < row & nc < col) {
                    blur_buf.insert_bottom(input[k], nc);
                }
            }

            if (j == 0 || i == 0)
                continue;

            for (int k = 0 ; k < N ; k++) {
            #pragma HLS UNROLL
                int nc = j+k-1;
                if (nc == 1 | nc == col)
                    Ix[k] = zero;
                else
                    Ix[k] = blur_buf.getval(1, nc-2) - blur_buf.getval(1, nc);
                if (i == 1 | i == row)
                    Iy[k] = zero;
                else
                    Iy[k] = blur_buf.getval(2, nc-1) - blur_buf.getval(0, nc-1);
            }

            Ixx = Ix * Ix;
            Iyy = Iy * Iy;
            Ixy = Ix * Iy;

            // fix me
            input = Ixx;
            stream_Ixx->write(input);
            input = Iyy;
            stream_Iyy->write(input);
            input = Ixy;
            stream_Ixy->write(input);

        }
    }
}
void blur_diff(stream_t* stream_Ixx, stream_t* stream_Iyy, stream_t* stream_Ixy,
        stream_t* stream_Sxx, stream_t* stream_Syy, stream_t* stream_Sxy,
        int32_t row, int32_t col)
{
    blur_img(stream_Ixx, stream_Sxx, row, col);
    blur_img(stream_Iyy, stream_Syy, row, col);
    blur_img(stream_Ixy, stream_Sxy, row, col);
}

void compute_det_trace(stream_t* stream_Sxx, stream_t* stream_Syy, stream_t* stream_Sxy,
        stream_t* stream_response, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    PIXEL_vec Sxx, Sxy, Syy;
    PIXEL_vec det;
    PIXEL_vec trace;
    PIXEL_vec tmp;

    ap_fixed<44, 34> _det[N], _trace[N], response[N];

    for (i = 0; i < row; ++i) {
    #pragma HLS loop_tripcount max=256
        for (j = 0; j < col; j += N) {
    #pragma HLS loop_tripcount max=128
    #pragma HLS pipeline enable_flush

            Sxx = stream_Sxx->read();
            Sxy = stream_Sxy->read();
            Syy = stream_Syy->read();

            trace = Sxx * Syy;
            tmp =  Sxy * Sxy;
            det = trace - tmp;

            for(int k = 0 ; k < N ; k++) {
            #pragma HLS UNROLL
                _det[k] = ap_fixed<44, 34>(det[k]);
                _trace[k] = ap_fixed<44, 34>(trace[k]);
                response[k] = _det[k] - ap_fixed<12, 2>(0.05) * _trace[k];

                if (response[k] > 6000000)
                    Sxx[k] = PIXEL(response[k]);
                else
                    Sxx[k] = 0;
            }
            stream_response->write(Sxx);

            // #include <iostream>
            // if(i < 5 && j < 5)
            //     std::cout << i << ' ' << j << ' ' << Sxx[0] << ' ' << Sxx[1] << std::endl;
            // if(i > 250 && j > 250)
            //     std::cout << i << ' ' << j << ' ' << Sxx[0] << ' ' << Sxx[1] << std::endl;
        }
    }
}

void find_local_maxima(stream_t* stream_response, stream_t* pstrmOutput, int32_t row, int32_t col)
{
    PIXEL_vec   input;
    PIXEL_vec   center_pixel;
    BUFFER_5    response_buf;
    int32_t     i, j;
    int32_t     si, sj;

    loop_i: for (i = 0 ; i < row+2; i++) {
    #pragma HLS loop_tripcount max=258
        loop_j: for (j = 0; j < col+2; j += N) {
    #pragma HLS loop_tripcount max=129
    #pragma HLS pipeline

            if (i < row & j < col)
                input = stream_response->read();

            // buffer update
            for (int k = 0 ; k < N ; k++) {
            #pragma HLS UNROLL
                int nc = j+k;
                if (nc < col)
                    response_buf.shift_up(nc);
                if (i < row & j < col) {
                    response_buf.insert_bottom(input[k], nc);
                }
            }

            if (i < 2 || j < 2)
                continue;

            for(int k = 0 ; k < N ; k++) {
            #pragma HLS UNROLL
                int nc = j+k;
                center_pixel[k] = response_buf.getval(2, nc - 2);
                input[k] = 0;
                if (center_pixel[k] != 0 && i > 3 && nc > 3 && i < row && nc < col) {
                    input[k] = 1;
                    loop_sj: for(sj = 4; sj >= 0; sj--) {
                        loop_si: for(si = 0 ; si < 5; si++) {
                            if(response_buf.getval(si, nc-sj) > center_pixel[k]) {
                                input[k] = 0;
                                break;
                            }
                        }
                    }

                }
            }
            pstrmOutput-> write(input);
        }
    }
}

void HCD(stream_t* pstrmInput, stream_t* pstrmOutput, reg32_t row, reg32_t col)
{
#pragma HLS INTERFACE s_axilite port=row
#pragma HLS INTERFACE s_axilite port=col
#pragma HLS INTERFACE axis register both port=pstrmOutput
#pragma HLS INTERFACE axis register both port=pstrmOutput2
#pragma HLS INTERFACE axis register both port=pstrmInput
#pragma HLS INTERFACE axis register both port=pstrmInput2
#pragma HLS INTERFACE s_axilite port=return

    int32_t i;
    int32_t j;

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

    // // Step 2: Calculate Ix, Iy (1st derivative of image along x and y axis)
    // // Step 3: Compute Ixx, Ixy, Iyy (Ixx = Ix*Ix, ...)
    compute_dif(&stream_blur, &stream_Ixx, &stream_Iyy,
            &stream_Ixy, row, col);

    // // Step 4: Compute Sxx, Sxy, Syy (weighted summation of Ixx, Ixy, Iyy in neighbor pixels)
    blur_diff(&stream_Ixx, &stream_Iyy, &stream_Ixy,
        &stream_Sxx, &stream_Syy, &stream_Sxy, row, col);

    // // Step 5: Compute the det and trace of matrix M (M = [[Sxx, Sxy], [Sxy, Syy]])
    // // Step 6: Compute the response of the detector by det/(trace+1e-12)
    compute_det_trace(&stream_Sxx, &stream_Syy, &stream_Sxy, &stream_response, row, col);

    // // Step 7: Post processing
    find_local_maxima(&stream_response, pstrmOutput, row, col);

    return;
}
