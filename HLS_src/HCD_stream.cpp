#include "HCD.h"
#include "cmath"

#ifndef __SYNTHESIS__
    #include <iostream>
#endif

/**
 * Gaussian filter with sigma 1.5
 * **/
GRAY_PIXEL Gaussian_filter_15(WINDOW* window)
{
    char i,j;
    double sum = 0;
    GRAY_PIXEL pixel =0;

    const float op[3][3] = {
        {0.0947417, 0.118318, 0.0947417},
        {0.118318 , 0.147761, 0.118318},
        {0.0947417, 0.118318, 0.0947417}
    };

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            sum += window->getval(i,j) * op[i][j];
        }
    }
    pixel = round(sum);
    return pixel;
}

/**
 * Gaussian filter with sigma 1
 * **/
template<typename P, typename W>
P Gaussian_filter_1(W* window)
{
    char i,j;
    double sum = 0;
    P pixel =0;

    const float op[3][3] = {
        {0.0751136, 0.123841, 0.0751136},
        {0.123841, 0.20418, 0.123841},
        {0.0751136, 0.123841, 0.0751136}
    };

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            sum += window->getval(i,j) * op[i][j];
        }
    }
    pixel = round(sum);
    return pixel;
}

void process_input(stream_ti& pstrmInput, STREAM_GRAY& stream_gray)
{
    AXI_PIXEL input = pstrmInput.read();
    GRAY_PIXEL input_gray_pix;
    input_gray_pix = (input.data.range(7,0)
            + input.data.range(15,8)
            + input.data.range(23,16))/3;

    stream_gray.write(input_gray_pix);
}

template<typename T>
void blur_img(hls::stream<T>& stream_gray, hls::stream<T>& stream_blur, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    T                   blur, input;
    ap_window<T, 3, 3>  window;
    ap_linebuffer<T, 3, MAX_WIDTH>  buf;

    for (i = 0 ; i < MAX_HEIGHT+1; i++) {
        for (j = 0; j < MAX_WIDTH+1; j++) {
            if (j < MAX_WIDTH)
                buf.shift_up(j);

            if (i < MAX_HEIGHT & j < MAX_WIDTH) {
                input = stream_gray.read();
                buf.insert_bottom(input, j);
            }

            window.shift_right();

            if (j < MAX_WIDTH) {
	            window.insert(buf.getval(2,j),0,2);
	            window.insert(buf.getval(1,j),1,2);
	            window.insert(input,2,2);
            }

            if (i == 0 || j == 0)
                continue;

            if(j == 1)
                window.rreflect();
            else if (j == MAX_WIDTH)
                window.lreflect();

            if (i == 1) 
                window.dreflect();
            else if (i == MAX_HEIGHT)
                window.ureflect();

            blur = Gaussian_filter_1<T, ap_window<T, 3, 3> >(&window);
            stream_blur.write(blur);
        }
    }
}


void compute_dif(STREAM_GRAY& stream_blur, STREAM_DOUBLE& stream_Ixx,
        STREAM_DOUBLE& stream_Iyy, STREAM_DOUBLE& stream_Ixy, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    DIF_PIXEL       Ix, Iy, zero = 0;
    DOUBLE_PIXEL    Ixx, Iyy, Ixy;
    GRAY_BUFFER_3   blur_buf;

    for (j = 0 ; j < MAX_WIDTH ; j++) {
        blur_buf.insert_at(stream_blur.read(), 2, j);
    }

    for (i = 0; i < MAX_HEIGHT; ++i) {
        blur_buf.shift_down();
        for (j = 0 ; j < MAX_WIDTH ; j++) {
            blur_buf.insert_at((i == MAX_HEIGHT-1) ? GRAY_PIXEL(0) : stream_blur.read(), 2, j);
        }

        for (j = 0; j < MAX_WIDTH; ++j)
        {
            Ix = (j > 0 && j < MAX_WIDTH-1) ? blur_buf.getval(1, j-1) - blur_buf.getval(1, j+1) : zero;
            Iy = (i > 0 && i < MAX_HEIGHT-1) ? blur_buf.getval(0, j) - blur_buf.getval(2, j) : zero;

            Ixx = Ix * Ix;
            Iyy = Iy * Iy;
            Ixy = Ix * Iy;

            stream_Ixx.write(Ixx);
            stream_Iyy.write(Iyy);
            stream_Ixy.write(Ixy);
        }
    }
}

void blur_diff(STREAM_DOUBLE& stream_Ixx, STREAM_DOUBLE& stream_Iyy, STREAM_DOUBLE& stream_Ixy,
        STREAM_DOUBLE& stream_Sxx, STREAM_DOUBLE& stream_Syy, STREAM_DOUBLE& stream_Sxy,
        int32_t row, int32_t col)
{
    blur_img<DOUBLE_PIXEL>(stream_Ixx, stream_Sxx, row, col);
    blur_img<DOUBLE_PIXEL>(stream_Iyy, stream_Syy, row, col);
    blur_img<DOUBLE_PIXEL>(stream_Ixy, stream_Sxy, row, col);
}

void compute_det_trace(STREAM_DOUBLE& stream_Sxx, STREAM_DOUBLE& stream_Syy, STREAM_DOUBLE& stream_Sxy,
        STREAM_DOUBLE_DOUBLE& stream_det, STREAM_DOUBLE_DOUBLE& stream_trace,
        int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    DOUBLE_PIXEL Sxx, Sxy, Syy;
    DOUBLE_DOUBLE_PIXEL det;
    DOUBLE_DOUBLE_PIXEL trace;

    for (i = 0; i < MAX_HEIGHT; ++i) {
        for (j = 0; j < MAX_WIDTH; ++j) {
            Sxx = stream_Sxx.read();
            Sxy = stream_Sxy.read();
            Syy = stream_Syy.read();

            trace = Sxx * Syy;
            stream_trace.write(trace);

            det = trace - Sxy * Sxy;
            stream_det.write(det);
#ifndef __SYNTHESIS__
    // std::cout << i << ' ' << j << ' ' << Ixx << ' ' << Ixy << ' ' << Iyy \
    //             << ' ' << trace_buf-> getval(i, j) << ' ' << det_buf-> getval(i, j) << std::endl;
#endif
        }
    }
}

void compute_response(STREAM_DOUBLE_DOUBLE& stream_det, STREAM_DOUBLE_DOUBLE& stream_trace,
    STREAM_DOUBLE_DOUBLE& stream_response, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    for (i = 0; i < MAX_HEIGHT; ++i) {
        for (j = 0; j < MAX_WIDTH; ++j) {
            double response = double(stream_det.read()) / (double(stream_trace.read()) + 1e-12);
            stream_response.write(round(response));
        }
    }
}

void output_maxima(STREAM_DOUBLE_DOUBLE& stream_response, stream_to& pstrmOutput, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    BOOL_PIXEL ret_pixel;
    for(i = 0 ; i < MAX_HEIGHT ; i++) {
        for(j = 0 ; j < MAX_WIDTH ; j++) {
            if(stream_response.read() > 100) {
                ret_pixel.data = 1;
            } else {
                ret_pixel.data = 0;
            }
            pstrmOutput.write(ret_pixel);
        }
    }
}

void HCD(stream_ti& pstrmInput, stream_to& pstrmOutput, reg32_t row, reg32_t col)
{
#pragma HLS INTERFACE axis register both port=pstrmOutput
#pragma HLS INTERFACE axis register both port=pstrmInput
#pragma HLS INTERFACE s_axilite port=row
#pragma HLS INTERFACE s_axilite port=col
#pragma dataflow

    int32_t i;
    int32_t j;

    STREAM_GRAY stream_gray;   // 8
    STREAM_GRAY stream_blur;

    STREAM_DOUBLE stream_Ixx;  // 17
    STREAM_DOUBLE stream_Ixy;
    STREAM_DOUBLE stream_Iyy;

    STREAM_DOUBLE stream_Sxx;  // 17
    STREAM_DOUBLE stream_Sxy;
    STREAM_DOUBLE stream_Syy;

    STREAM_DOUBLE_DOUBLE stream_det;  // 34
    STREAM_DOUBLE_DOUBLE stream_trace;

    STREAM_DOUBLE_DOUBLE stream_response;

#pragma HLS stream variable=stream_gray depth=1
#pragma HLS stream variable=stream_blur depth=1
#pragma HLS stream variable=stream_Ixx depth=1
#pragma HLS stream variable=stream_Ixy depth=1
#pragma HLS stream variable=stream_Iyy depth=1
#pragma HLS stream variable=stream_Sxx depth=1
#pragma HLS stream variable=stream_Sxy depth=1
#pragma HLS stream variable=stream_Syy depth=1
#pragma HLS stream variable=stream_det depth=1
#pragma HLS stream variable=stream_trace depth=1
#pragma HLS stream variable=stream_response depth=1

    for (i = 0; i < MAX_HEIGHT; ++i) {
        for (j = 0; j < MAX_WIDTH; ++j) {
             // Step 0: Convert to gray scale
            process_input(pstrmInput, stream_gray);
        }
    }

#ifndef __SYNTHESIS__
	std::cout << "step 0 input" << std::endl;
#endif

    // Step 1: Smooth the image by Gaussian kernel
    blur_img<GRAY_PIXEL>(stream_gray, stream_blur, row, col);

#ifndef __SYNTHESIS__
	std::cout << "step 1 smooth image" << std::endl;
#endif

    // Step 2: Calculate Ix, Iy (1st derivative of image along x and y axis)
    // Step 3: Compute Ixx, Ixy, Iyy (Ixx = Ix*Ix, ...)
    compute_dif(stream_blur, stream_Ixx, stream_Iyy,
            stream_Ixy, row, col);

#ifndef __SYNTHESIS__
	std::cout << "step 2, 3 compute diff" << std::endl;
#endif

    // Step 4: Compute Sxx, Sxy, Syy (weighted summation of Ixx, Ixy, Iyy in neighbor pixels)
    blur_diff(stream_Ixx, stream_Iyy, stream_Ixy,
        stream_Sxx, stream_Syy, stream_Sxy, row, col);

#ifndef __SYNTHESIS__
	std::cout << "step 4 compute sum" << std::endl;
#endif

    // Todo:
    // Step 5: Compute the det and trace of matrix M (M = [[Sxx, Sxy], [Sxy, Syy]])
    compute_det_trace(stream_Sxx, stream_Syy, stream_Sxy, stream_det, stream_trace, row, col);

#ifndef __SYNTHESIS__
	std::cout << "step 5 compute det trace" << std::endl;
#endif

    // Step 6: Compute the response of the detector by det/(trace+1e-12)
    compute_response(stream_det, stream_trace, stream_response, row, col);

#ifndef __SYNTHESIS__
	std::cout << "step 6 compute response" << std::endl;
#endif

    // Step 7: Post processing
    output_maxima(stream_response, pstrmOutput, row, col);

#ifndef __SYNTHESIS__
	std::cout << "step 7 output" << std::endl;
#endif

}
