#include "HCD.h"
#include "cmath"
#include "math.h"
#include <hls_stream.h>
#include <hls_vector.h>


const int unroll_factor = N;

/**
 * ##########################################################################################
 * Utility Function
 */

/**
 * Gaussian filter with sigma 1
 * **/
template<typename P, typename W>
P Gaussian_filter_1(W* window, int idx)
{
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
            int rc = idx+j;
            sum += op[i][j] * window-> getval(i, rc);
        }
    }
    pixel = P(sum);
    return pixel;
}

/**
 * Image convolution
 * **/
template <typename ST, typename DT>
void conv(ST* stream_in, ST* stream_out, int row, int col)
{
    int i;
    int j;
    hls::vector<DT, N> blur;
    hls::vector<DT, N> input;
    ap_window<DT, 3, N+2> window;
    ap_linebuffer<hls::vector<DT, N>, 3, MAX_WIDTH/N> buf;

    for (i = 0 ; i < row+1; i++) {
    #pragma HLS loop_tripcount max=257
        for (j = 0; j < col+1; j++) {
    #pragma HLS loop_tripcount max=257

    #pragma HLS UNROLL factor=unroll_factor
    #pragma HLS pipeline
    #pragma HLS dependence variable=buf

            bool factor_N = (j % N == 0) ? 1 : 0;

            if (factor_N) {
                window.shift_right_N(N);

                if (j < col) {
                    buf.shift_down(v_ind(j));
                    if (i < row) {
                        input = stream_in->read();
                        buf.insert_top(input, v_ind(j));
                    }

                    for(int k = 0 ; k < N ; k++) {
                        window.insert(buf.getval(0, v_ind(j))[k], 0, 2+k);
                        window.insert(buf.getval(1, v_ind(j))[k], 1, 2+k);
                        window.insert(buf.getval(2, v_ind(j))[k], 2, 2+k);
                    }
                }

                if(j == 0)
                    window.rreflect();
                else if (j == col)
                    window.lreflect();

                if (i == 1)
                    window.dreflect();
                else if (i == row)
                    window.ureflect();
            }

            if(i == 0 || j == 0) continue;

            blur[v_ind_ind(j-1)] = Gaussian_filter_1<DT, ap_window<DT, 3, N+2>>
                (&window, v_ind_ind(j));

            if(factor_N) {
                stream_out->write(blur);
            }
        }
    }
}

/**
 * ##########################################################################################
 * Function in Dataflow
 */

void getMem(INPUT* menInput, MEM_STREAM* str, int row, int col)
{
    int arr_size = ceil(row * col * 24.0 / 512);
    for (int i = 0; i < arr_size; ++i) {
        #pragma HLS loop_tripcount max=3072
        #pragma HLS PIPELINE
    	str->write(menInput[i]);
    }
}

void men2str(MEM_STREAM* stream_mem, PIXEL_V_STREAM* str, int row, int col)
{
    int arr_size = ceil(row * col * 24.0 / 512);

    ap_int<512+24*N> buf;
    PIXEL_VEC out_vec;

    int i, j, k, outlier, batch_size;
    int index = 0;
    int lb = 0;
    for (i = 0; i < arr_size; ++i) {
        #pragma HLS loop_tripcount max=3072

        buf.range(index+511, index) = stream_mem->read();
        index += 512;

        batch_size = index/24/N;
        for (j = 0; j < batch_size; ++j) {
            #pragma HLS loop_tripcount max=12
            #pragma HLS PIPELINE
            for (k = 0; k < N; ++k) {
                int offset = lb + 24 * k;
                out_vec[k] = { buf.range(offset+23, offset+16),
                    buf.range(offset+15, offset+8), buf.range(offset+7, offset)};
            }
            str-> write(out_vec);
            lb += 24 * N;
        }


        if (lb != index) {
            outlier = (index-lb);
            buf.range(outlier-1, 0) = buf.range(index-1, lb);
            index = outlier;
        } else {
            index = 0;
        }
        lb = 0;
    }
}


void process_input(PIXEL_V_STREAM* pstrmInput, GPIXEL_V_STREAM* stream_gray, int row, int col)
{
    int i;
    int j;
    PIXEL_VEC input ;
    GPIXEL_VEC input_gray_pix;

    for (i = 0; i < row; ++i) {
        #pragma HLS loop_tripcount max=256
        for (j = 0; j < col/N; ++j) {
            #pragma HLS loop_tripcount max=256/unroll_factor
            #pragma HLS pipeline

            input = pstrmInput->read();
            for(int k = 0 ; k < N ; ++k) {
                input_gray_pix[k] = (input[k].r + input[k].g + input[k].b)/3;
            }
            stream_gray->write(input_gray_pix);
        }
    }
}

void blur_img(GPIXEL_V_STREAM* stream_gray, GPIXEL_V_STREAM* stream_blur, int row, int col)
{
    conv<GPIXEL_V_STREAM, GPIXEL>(stream_gray, stream_blur, row, col);
}

void compute_dif(GPIXEL_V_STREAM* stream_blur, DGPIXEL_V_STREAM* stream_Ixx,
        DGPIXEL_V_STREAM* stream_Iyy, DGPIXEL_V_STREAM* stream_Ixy, int row, int col)
{
    int i;
    int j;

    GPIXEL_VEC 	input;
    GPIXEL_VEC   Ix, Iy;
    DGPIXEL_VEC   Ixx, Iyy, Ixy;
    BUFFER_3    buf;
    WINDOW      window;

    for (i = 0 ; i < row+1; i++) {
    #pragma HLS loop_tripcount max=257
        for (j = 0; j < col+1; j++) {
    #pragma HLS loop_tripcount max=257

    #pragma HLS UNROLL factor=unroll_factor
    #pragma HLS pipeline

            bool factor_N = (j % N == 0) ? 1 : 0;

            if (factor_N) {
                window.shift_right_N(N);

                if (j < col) {
                    buf.shift_down(v_ind(j));
                    if (i < row) {
                        input = stream_blur->read();
                        buf.insert_top(input, v_ind(j));
                    }

                    for(int k = 0 ; k < N ; k++) {
                        window.insert(buf.getval(0, v_ind(j))[k], 0, 2+k);
                        window.insert(buf.getval(1, v_ind(j))[k], 1, 2+k);
                        window.insert(buf.getval(2, v_ind(j))[k], 2, 2+k);
                    }
                }
            }

            if (j == 0 || i == 0) continue;
            int vec_index = v_ind_ind(j-1);
            int window_index = v_ind_ind(j);

            GPIXEL tmp_x = window.getval(1, window_index) - window.getval(1, window_index+2);
            Ix[vec_index] = (j == 1 | j == col) ? GPIXEL(0) : tmp_x;
            GPIXEL tmp_y = window.getval(0, window_index+1) - window.getval(2, window_index+1);
            Iy[vec_index] = (i == 1 | i == row) ? GPIXEL(0) : tmp_y;

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


void blur_diff(DGPIXEL_V_STREAM* stream_Ixx, DGPIXEL_V_STREAM* stream_Iyy, DGPIXEL_V_STREAM* stream_Ixy,
        DGPIXEL_V_STREAM* stream_Sxx, DGPIXEL_V_STREAM* stream_Syy, DGPIXEL_V_STREAM* stream_Sxy,
        int row, int col)
{
    conv<DGPIXEL_V_STREAM, DGPIXEL>(stream_Ixx, stream_Sxx, row, col);
    conv<DGPIXEL_V_STREAM, DGPIXEL>(stream_Iyy, stream_Syy, row, col);
    conv<DGPIXEL_V_STREAM, DGPIXEL>(stream_Ixy, stream_Sxy, row, col);
}

void compute_det_trace(DGPIXEL_V_STREAM* stream_Sxx, DGPIXEL_V_STREAM* stream_Syy, DGPIXEL_V_STREAM* stream_Sxy,
        DGPIXEL_V_STREAM* stream_response, int row, int col)
{
    int i;
    int j;
    DGPIXEL_VEC Sxx, Sxy, Syy;
    DGPIXEL_VEC output;

    hls::vector<ap_int<32>, N>  tmp, det, trace, response;

    for (i = 0; i < row; ++i) {
    #pragma HLS loop_tripcount max=256
        for (j = 0; j < col; ++j) {
    #pragma HLS loop_tripcount max=256

    #pragma HLS UNROLL factor=unroll_factor skip_exit_check
    #pragma HLS pipeline

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
            	output[vec_index] = DGPIXEL(response[vec_index]>>12);
            else
            	output[vec_index] = DGPIXEL(0);

            if((j % N) == (N-1)) {
                stream_response->write(output);
            }
        }
    }
}

void find_local_maxima(DGPIXEL_V_STREAM* stream_response, BPIXEL_V_STREAM* pstrmOutput, int row, int col)
{
    DGPIXEL_VEC  input;
    BPIXEL_VEC  output;
    DGPIXEL_VEC  center_pixel;
    DBUFFER_5    buf;
    DWINDOW_5    window;

    loop_i: for (int i = 0 ; i < row+2; i++) {
    #pragma HLS loop_tripcount max=258
        loop_j: for (int j = 0; j < col+2; j++) {
    #pragma HLS loop_tripcount max=258

    #pragma HLS UNROLL factor=unroll_factor
    #pragma HLS pipeline

            if (j % N == 0) {
                window.shift_right_N(N);

                if (j < col) {
                    buf.shift_down(v_ind(j));
                    if (i < row) {
                        input = stream_response->read();
                        buf.insert_top(input, v_ind(j));
                    }

                    for(int k = 0 ; k < N ; k++) {
                        window.insert(buf.getval(0, v_ind(j))[k], 0, 4+k);
                        window.insert(buf.getval(1, v_ind(j))[k], 1, 4+k);
                        window.insert(buf.getval(2, v_ind(j))[k], 2, 4+k);
                        window.insert(buf.getval(3, v_ind(j))[k], 3, 4+k);
                        window.insert(buf.getval(4, v_ind(j))[k], 4, 4+k);
                    }
                }
            }

            if (i < 2 || j < 2) continue;

            int vec_index = v_ind_ind(j-2);
            int window_index = v_ind_ind(j);

            center_pixel[vec_index] = window.getval(2, window_index + 2);
            BPIXEL tmp = 1;
            for(int wi = 0 ; wi < 5 ; wi++) {
                for(int wj = 0 ; wj < 5 ; wj++) {
                    if(window.getval(wi, window_index + wj) > center_pixel[vec_index])
                        tmp = 0;
                }
            }
            output[vec_index] = (center_pixel[vec_index] != 0 && i > 3 && j > 3 && i < row && j < col) ? tmp : BPIXEL(0);

            if( ((j % N) == 1) ) {
                pstrmOutput-> write(output);
            }
        }
    }
}

void str2mem(BPIXEL_V_STREAM* str, OUTPUT* memOutput,  int row, int col)
{
    int arr_index = 0;
    ap_int<512> buf;
    BPIXEL_VEC in_vec;

    int i, j, k, outlier;
    int index = 0;
    for (i = 0; i < row*col/N; ++i) {
        #pragma HLS loop_tripcount max=32768
        #pragma HLS PIPELINE
        in_vec = str->read();

        for (j = 0; j < N; ++j) {
            buf.set_bit(index, in_vec[j]);
            ++index;
        }

        if (index >= 512) {
            memOutput[arr_index] = buf;
            ++arr_index;
            index = 0;
        }
    }

    if(index != 0)
        memOutput[arr_index] = buf.range(index, 0);
}


void HCD(INPUT* memInput, OUTPUT* memOutput,  int row, int col)
{
#pragma HLS INTERFACE s_axilite port=row
#pragma HLS INTERFACE s_axilite port=col
#pragma HLS INTERFACE m_axi num_read_outstanding=1  max_read_burst_length=2  latency=100 depth=3072 port=memInput
#pragma HLS INTERFACE m_axi latency=100 depth=512 port=menOutput
#pragma HLS INTERFACE s_axilite port=return

    int i;
    int j;

    MEM_STREAM stream_mem;
    PIXEL_V_STREAM pstrmInput;
    GPIXEL_V_STREAM stream_gray;
    GPIXEL_V_STREAM stream_blur;
    DGPIXEL_V_STREAM stream_Ixx;
    DGPIXEL_V_STREAM stream_Ixy;
    DGPIXEL_V_STREAM stream_Iyy;
    DGPIXEL_V_STREAM stream_Sxx;
    DGPIXEL_V_STREAM stream_Sxy;
    DGPIXEL_V_STREAM stream_Syy;
    DGPIXEL_V_STREAM stream_response;
    BPIXEL_V_STREAM pstrmOutput;

#pragma HLS DATAFLOW
    // Step 0: get data from memory and convert to gray scale
    getMem(memInput, &stream_mem, row, col);
    men2str(&stream_mem, &pstrmInput, row, col);
    process_input(&pstrmInput, &stream_gray, row, col);

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
    // compute_det_trace(&stream_Sxx, &stream_Syy, &stream_Sxy, &stream_response, row, col);
    compute_det_trace(&stream_Sxx, &stream_Syy, &stream_Sxy, &stream_response, row, col);

    // Step 7: Post processing
    find_local_maxima(&stream_response, &pstrmOutput, row, col);

    // Step 8: write result to memory
    str2mem(&pstrmOutput, memOutput, row, col);

    return;
}
