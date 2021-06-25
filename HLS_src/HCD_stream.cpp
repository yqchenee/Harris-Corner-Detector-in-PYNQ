#include "HCD.h"
#include "cmath"

//#ifndef __SYNTHESIS__
//    #include <iostream>
//    using namespace std;
//#endif
//
///**
// * Gaussian filter with sigma 1.5
// * **/
//GRAY_PIXEL Gaussian_filter_15(WINDOW* window)
//{
//    char i,j;
//    double sum = 0;
//    GRAY_PIXEL pixel =0;
//
//    const float op[3][3] = {
//        {0.0947417, 0.118318, 0.0947417},
//        {0.118318 , 0.147761, 0.118318},
//        {0.0947417, 0.118318, 0.0947417}
//    };
//
//    for (i = 0; i < 3; i++) {
//        for (j = 0; j < 3; j++) {
//            sum += window->getval(i,j) * op[i][j];
//        }
//    }
//    pixel = round(sum);
//    return pixel;
//}
//
///**
// * Gaussian filter with sigma 1
// * **/
//template<typename P, typename W>
//P Gaussian_filter_1(W* window)
//{
//    char i,j;
//    ap_fixed<27, 17> sum = 0;
//    P pixel =0;
//
//    // float op[3];
//    // init_gaussian_window(op);
//
//    ap_fixed<12, 1> op[3][3] =
//    {
//        {0.0751136, 0.123841, 0.0751136},
//        {0.123841, 0.20418, 0.123841},
//        {0.0751136, 0.123841, 0.0751136}
//    };
//
//    for (i = 0; i < 3; i++) {
//        for (j = 0; j < 3; j++) {
//            #pragma HLS pipeline
//            sum += window->getval(i,j) * op[i][j];
//        }
//    }
//    pixel = P(sum);
//    return pixel;
//}
//
//template<typename T>
//void blur_img(hls::stream<T>& stream_gray, hls::stream<T>& stream_blur, int32_t row, int32_t col)
//{
//    int32_t i;
//    int32_t j;
//    T                   blur, input;
//    ap_window<T, 3, 3>  window;
//    ap_linebuffer<T, 3, MAX_WIDTH>  buf;
//
//    for (i = 0 ; i < MAX_HEIGHT+1; i++) {
//        for (j = 0; j < MAX_WIDTH+1; j++) {
//            if (j < MAX_WIDTH)
//                buf.shift_up(j);
//
//            if (i < MAX_HEIGHT & j < MAX_WIDTH) {
//                input = stream_gray.read();
//                buf.insert_bottom(input, j);
//            }
//
//            window.shift_right();
//
//            if (j < MAX_WIDTH) {
//                window.insert(buf.getval(2,j),0,2);
//                window.insert(buf.getval(1,j),1,2);
//                window.insert(input,2,2);
//            }
//
//            if (i == 0 || j == 0)
//                continue;
//
//            if(j == 1)
//                window.rreflect();
//            else if (j == MAX_WIDTH)
//                window.lreflect();
//
//            if (i == 1)
//                window.dreflect();
//            else if (i == MAX_HEIGHT)
//                window.ureflect();
//
//            blur = Gaussian_filter_1<T, ap_window<T, 3, 3> >(&window);
//            stream_blur.write(blur);
//        }
//    }
//}
//
//void compute_dif(STREAM_GRAY& stream_blur, STREAM_DOUBLE& stream_Ixx,
//        STREAM_DOUBLE& stream_Iyy, STREAM_DOUBLE& stream_Ixy, int32_t row, int32_t col)
//{
//    int32_t i;
//    int32_t j;
//    DIF_PIXEL       Ix, Iy, zero = 0;
//    DOUBLE_PIXEL    Ixx, Iyy, Ixy;
//    GRAY_BUFFER_3   blur_buf;
//    GRAY_PIXEL input;
//
//    for (i = 0 ; i < MAX_HEIGHT+1; i++) {
//        for (j = 0; j < MAX_WIDTH+1; j++) {
//            if (j < MAX_WIDTH)
//                blur_buf.shift_up(j);
//
//            if (i < MAX_HEIGHT & j < MAX_WIDTH) {
//                input = stream_blur.read();
//                blur_buf.insert_bottom(input, j);
//            }
//
//            if (j == 0 || i == 0)
//                continue;
//
//            else {
//                Ix = (j == 1 | j == MAX_WIDTH) ? zero : blur_buf.getval(1, j-2) - blur_buf.getval(1, j);
//                Iy = (i == 1 | i == MAX_HEIGHT) ? zero : blur_buf.getval(2, j-1) - blur_buf.getval(0, j-1);
//
//                Ixx = Ix * Ix;
//                Iyy = Iy * Iy;
//                Ixy = Ix * Iy;
//
//                stream_Ixx.write(Ixx);
//                stream_Iyy.write(Iyy);
//                stream_Ixy.write(Ixy);
//            }
//        }
//    }
//}
//
//void blur_diff(STREAM_DOUBLE& stream_Ixx, STREAM_DOUBLE& stream_Iyy, STREAM_DOUBLE& stream_Ixy,
//        STREAM_DOUBLE& stream_Sxx, STREAM_DOUBLE& stream_Syy, STREAM_DOUBLE& stream_Sxy,
//        int32_t row, int32_t col)
//{
//    blur_img<DOUBLE_PIXEL>(stream_Ixx, stream_Sxx, row, col);
//    blur_img<DOUBLE_PIXEL>(stream_Iyy, stream_Syy, row, col);
//    blur_img<DOUBLE_PIXEL>(stream_Ixy, stream_Sxy, row, col);
//}
//
//void compute_det_trace(STREAM_DOUBLE& stream_Sxx, STREAM_DOUBLE& stream_Syy, STREAM_DOUBLE& stream_Sxy,
//        STREAM_DOUBLE_DOUBLE& stream_det, STREAM_DOUBLE_DOUBLE& stream_trace,
//        int32_t row, int32_t col)
//{
//    int32_t i;
//    int32_t j;
//    DOUBLE_PIXEL Sxx, Sxy, Syy;
//    DOUBLE_DOUBLE_PIXEL det;
//    DOUBLE_DOUBLE_PIXEL tmp;
//    DOUBLE_DOUBLE_PIXEL trace;
//
//    for (i = 0; i < MAX_HEIGHT; ++i) {
//        for (j = 0; j < MAX_WIDTH; ++j) {
//            Sxx = stream_Sxx.read();
//            Sxy = stream_Sxy.read();
//            Syy = stream_Syy.read();
//
//            #pragma HLS RESOURCE variable=tmp core=Mul
//            #pragma HLS RESOURCE variable=trace core=Mul
//            trace = Sxx * Syy;
//            tmp =  Sxy * Sxy;
//            det = trace - tmp;
//
//            stream_trace.write(trace);
//            stream_det.write(det);
//        }
//    }
//}
//
//void compute_response(STREAM_DOUBLE_DOUBLE& stream_det, STREAM_DOUBLE_DOUBLE& stream_trace,
//    STREAM_DOUBLE_DOUBLE& stream_response, int32_t row, int32_t col)
//{
//    int32_t i;
//    int32_t j;
//    ap_fixed<44, 34> det, trace, response;
//    for (i = 0; i < MAX_HEIGHT; ++i) {
//        for (j = 0; j < MAX_WIDTH; ++j) {
//            det = stream_det.read();
//            trace = ap_fixed<44, 34>(stream_trace.read());
//            response = det - ap_fixed<12, 2>(0.05) * trace;
//            stream_response.write(DOUBLE_DOUBLE_PIXEL(response));
//        }
//    }
//}
//
//void output_maxima(STREAM_DOUBLE_DOUBLE& stream_response, stream_to& pstrmOutput, int32_t row, int32_t col)
//{
//    int32_t i;
//    int32_t j;
//    BOOL_PIXEL ret_pixel;
//    for(i = 0 ; i < MAX_HEIGHT ; i++) {
//        for(j = 0 ; j < MAX_WIDTH ; j++) {
//            DOUBLE_DOUBLE_PIXEL response = stream_response.read();
//            if(response > 6000000) {
//                ret_pixel.data = 1;
//            } else {
//                ret_pixel.data = 0;
//            }
//            pstrmOutput.write(ret_pixel);
//        }
//    }
//}

void HCD(stream_ti* pstrmInput, stream_to* pstrmOutput, reg32_t row, reg32_t col)
{
#pragma HLS INTERFACE s_axilite port=row
#pragma HLS INTERFACE s_axilite port=col
#pragma HLS INTERFACE axis register both port=pstrmOutput
#pragma HLS INTERFACE axis register both port=pstrmInput
#pragma HLS INTERFACE s_axilite port=return

    int32_t i;
    int32_t j;

    /**
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
//#pragma HLS DATAFLOW
    **/

    for (i = 0; i < MAX_HEIGHT; ++i) {
        for (j = 0; j < MAX_WIDTH; ++j) {
             // Step 0: Convert to gray scale
            AXI_PIXEL input = pstrmInput->read();
            pstrmOutput->write(input);
        }
    }

    return;

    /**
    // Step 1: Smooth the image by Gaussian kernel
    blur_img<GRAY_PIXEL>(stream_gray, stream_blur, row, col);

    // Step 2: Calculate Ix, Iy (1st derivative of image along x and y axis) Step 3: Compute Ixx, Ixy, Iyy (Ixx = Ix*Ix, ...)
    compute_dif(stream_blur, stream_Ixx, stream_Iyy,
            stream_Ixy, row, col);

    // Step 4: Compute Sxx, Sxy, Syy (weighted summation of Ixx, Ixy, Iyy in neighbor pixels)
    blur_diff(stream_Ixx, stream_Iyy, stream_Ixy,
        stream_Sxx, stream_Syy, stream_Sxy, row, col);

    // Step 5: Compute the det and trace of matrix M (M = [[Sxx, Sxy], [Sxy, Syy]])
    compute_det_trace(stream_Sxx, stream_Syy, stream_Sxy, stream_det, stream_trace, row, col);

    // Step 6: Compute the response of the detector by det/(trace+1e-12)
    compute_response(stream_det, stream_trace, stream_response, row, col);

    // Step 7: Post processing
    output_maxima(stream_response, pstrmOutput, row, col);
    **/
}

