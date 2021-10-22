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
            sum += window->getval(i,j) * op[i][j];
        }
    }
    pixel = P(sum);
    return pixel;
}

void process_input(stream_io* pstrmInput, stream_t* stream_gray, int row, int col)
{
    int i;
    int j;
    PIXEL input ;
    PIXEL input_gray_pix;
    for (i = 0; i < row; ++i) {
    #pragma HLS loop_tripcount max=256
    #pragma HLS PIPELINE off
        for (j = 0; j < col; ++j) {
    #pragma HLS loop_tripcount max=256
    #pragma HLS PIPELINE off
            input = pstrmInput->read().data;
            input_gray_pix = (input.range(7,0)
                    + input.range(15,8)
                    + input.range(23,16))/3;

            // fix me
            input = input_gray_pix;
            stream_gray->write(input);
        }
    }
}

void blur_img(stream_t* stream_gray, stream_t* stream_blur, int row, int col)
{
    int i;
    int j;
    PIXEL   tmp, blur;
    PIXEL   input;
    WINDOW  window;
    BUFFER_3  buf;

    for (i = 0 ; i < row+1; i++) {
    #pragma HLS loop_tripcount max=257
    #pragma HLS PIPELINE off
        for (j = 0; j < col+1; j++) {
    #pragma HLS loop_tripcount max=257
    #pragma HLS PIPELINE off
            if (j < col)
                buf.shift_up(j);

            if (i < row & j < col) {
                input = stream_gray->read();
                tmp = input;
                buf.insert_bottom(tmp, j);
            }

            window.shift_right();

            if (j < col) {
                window.insert(buf.getval(2,j),0,2);
                window.insert(buf.getval(1,j),1,2);
                window.insert(tmp,2,2);
            }

            if (i == 0 || j == 0)
                continue;

            if(j == 1)
                window.rreflect();
            else if (j == col)
                window.lreflect();

            if (i == 1)
                window.dreflect();
            else if (i == row)
                window.ureflect();

            blur = Gaussian_filter_1<PIXEL, WINDOW >(&window);

            stream_blur->write(blur);
        }
    }
}

void compute_dif(stream_t* stream_blur, stream_t* stream_Ixx,
        stream_t* stream_Iyy, stream_t* stream_Ixy, int row, int col)
{
    int i;
    int j;
    PIXEL    Ix, Iy, zero = 0;
    PIXEL    tmp, Ixx, Iyy, Ixy;
    BUFFER_3   blur_buf;
    PIXEL 	input;

    for (i = 0 ; i < row+1; i++) {
    #pragma HLS loop_tripcount max=257
    #pragma HLS PIPELINE off
        for (j = 0; j < col+1; j++) {
    #pragma HLS loop_tripcount max=257
    #pragma HLS PIPELINE off
            if (j < col)
                blur_buf.shift_up(j);

            if (i < row & j < col) {
                input = stream_blur->read();
                tmp = input;
                blur_buf.insert_bottom(tmp, j);
            }

            if (j == 0 || i == 0)
                continue;

            else {
                if (j == 1 | j == col)
                    Ix = zero;
                else
                    Ix = blur_buf.getval(1, j-2) - blur_buf.getval(1, j);
                if (i == 1 | i == row)
                    Iy = zero;
                else
                    Iy = blur_buf.getval(2, j-1) - blur_buf.getval(0, j-1);

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
}

void blur_diff(stream_t* stream_Ixx, stream_t* stream_Iyy, stream_t* stream_Ixy,
        stream_t* stream_Sxx, stream_t* stream_Syy, stream_t* stream_Sxy,
        int row, int col)
{
    blur_img(stream_Ixx, stream_Sxx, row, col);
    blur_img(stream_Iyy, stream_Syy, row, col);
    blur_img(stream_Ixy, stream_Sxy, row, col);
}

void compute_response(stream_t* stream_Sxx, stream_t* stream_Syy, stream_t* stream_Sxy,
        stream_t* stream_response, int row, int col)
{
    int i;
    int j;
    PIXEL input[3];
    PIXEL Sxx, Sxy, Syy;
    PIXEL det;
    PIXEL tmp;
    PIXEL trace;

    ap_fixed<44, 34> _det, _trace, response;

    for (i = 0; i < row; ++i) {
    #pragma HLS loop_tripcount max=256
    #pragma HLS PIPELINE off
        for (j = 0; j < col; ++j) {
    #pragma HLS loop_tripcount max=256
    #pragma HLS PIPELINE off
            input[0] = stream_Sxx->read();
            input[1] = stream_Sxy->read();
            input[2] = stream_Syy->read();

            Sxx = input[0];
            Sxy = input[1];
            Syy = input[2];

            trace = Sxx * Syy;
            tmp =  Sxy * Sxy;
            det = trace - tmp;

            _det = ap_fixed<44, 34>(det);
            _trace = ap_fixed<44, 34>(trace);
            response = _det - ap_fixed<12, 2>(0.05) * _trace;

            if (response > 6000000)
                input[0] = PIXEL(response);
            else
                input[0] = 0;

            stream_response->write(input[0]);
        }
    }
}

void find_local_maxima(stream_t* stream_response, stream_io* pstrmOutput, int row, int col)
{
    AXI_PIXEL   input;
    PIXEL       center_pixel;
    BUFFER_5    response_buf;
    int     i, j;
    int     si, sj;
    int     l_bound, r_bound;

    for (i = 0 ; i < row+2; i++) {
    #pragma HLS loop_tripcount max=258
    #pragma HLS PIPELINE off
        for (j = 0; j < col+2; j++) {
    #pragma HLS loop_tripcount max=258
    #pragma HLS PIPELINE off
            if (j < col)
                response_buf.shift_up(j);

            if (i < row & j < col) {
                input.data = stream_response->read();
                response_buf.insert_bottom(input.data, j);
            }

            if (i < 2 || j < 2)
                continue;

            center_pixel = response_buf.getval(2, j - 2);
            input.data = 0;
            if (center_pixel != 0 && i > 3 && j > 3 && i < row && j < col) {
                input.data =1;
                loop_sj: for(sj = 4; sj >= 0; sj--) {
                    loop_si: for(si = 0 ; si < 5; si++) {
                        if(response_buf.getval(si, j - sj) > center_pixel) {
                            input.data = 0;
                            break;
                        }
                    }
                }
            }
            pstrmOutput-> write(input);
        }
    }
}

void HCD(stream_io* pstrmInput, stream_io* pstrmOutput, int row, int col)
{
#pragma HLS INTERFACE s_axilite port=row
#pragma HLS INTERFACE s_axilite port=col
#pragma HLS INTERFACE axis register both port=pstrmOutput
#pragma HLS INTERFACE axis register both port=pstrmInput
#pragma HLS INTERFACE s_axilite port=return

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
    compute_response(&stream_Sxx, &stream_Syy, &stream_Sxy, &stream_response, row, col);

    // Step 7: Post processing
    find_local_maxima(&stream_response, pstrmOutput, row, col);

    return;
}
