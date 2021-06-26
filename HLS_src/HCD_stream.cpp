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

void process_input(stream_t* pstrmInput, stream_t* stream_gray, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    AXI_PIXEL input ;
    PIXEL input_gray_pix;
    for (i = 0; i < MAX_HEIGHT; ++i) {
        for (j = 0; j < MAX_WIDTH; ++j) {
            input = pstrmInput->read();
            input_gray_pix = (input.data.range(7,0)
                    + input.data.range(15,8)
                    + input.data.range(23,16))/3;

            // fix me
            input.data = input_gray_pix;
            stream_gray->write(input);
        }
    }
}

void blur_img(stream_t* stream_gray, stream_t* stream_blur, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    PIXEL   tmp, blur;
    AXI_PIXEL input;
    WINDOW  window;
    BUFFER_3  buf;


    for (i = 0 ; i < MAX_HEIGHT+1; i++) {
        for (j = 0; j < MAX_WIDTH+1; j++) {
            if (j < MAX_WIDTH)
                buf.shift_up(j);

            if (i < MAX_HEIGHT & j < MAX_WIDTH) {
                input = stream_gray->read();
                tmp = input.data;
                buf.insert_bottom(tmp, j);
            }

            window.shift_right();

            if (j < MAX_WIDTH) {
                window.insert(buf.getval(2,j),0,2);
                window.insert(buf.getval(1,j),1,2);
                window.insert(tmp,2,2);
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

            blur = Gaussian_filter_1<PIXEL, WINDOW >(&window);

            // fix me
            input.data = blur;
            stream_blur->write(input);
        }
    }
}

void compute_dif(stream_t* stream_blur, stream_t* stream_Ixx,
        stream_t* stream_Iyy, stream_t* stream_Ixy, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    PIXEL    Ix, Iy, zero = 0;
    PIXEL    tmp, Ixx, Iyy, Ixy;
    BUFFER_3   blur_buf;
    AXI_PIXEL input;

    for (i = 0 ; i < MAX_HEIGHT+1; i++) {
        for (j = 0; j < MAX_WIDTH+1; j++) {
            if (j < MAX_WIDTH)
                blur_buf.shift_up(j);

            if (i < MAX_HEIGHT & j < MAX_WIDTH) {
                input = stream_blur->read();
                tmp = input.data;
                blur_buf.insert_bottom(tmp, j);
            }

            if (j == 0 || i == 0)
                continue;

            else {
                if (j == 1 | j == MAX_WIDTH)
                    Ix = zero;
                else 
                    Ix = blur_buf.getval(1, j-2) - blur_buf.getval(1, j);
                if (i == 1 | i == MAX_HEIGHT)
                    Iy = zero;
                else 
                    Iy = blur_buf.getval(2, j-1) - blur_buf.getval(0, j-1);

                Ixx = Ix * Ix;
                Iyy = Iy * Iy;
                Ixy = Ix * Iy;

                // fix me
                input.data = Ixx;
                stream_Ixx->write(input);
                input.data = Iyy;
                stream_Iyy->write(input);
                input.data = Ixy;
                stream_Ixy->write(input);
            }
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
        stream_t* stream_det, stream_t* stream_trace,
        int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    AXI_PIXEL input[3];
    PIXEL Sxx, Sxy, Syy;
    PIXEL det;
    PIXEL tmp;
    PIXEL trace;

    for (i = 0; i < MAX_HEIGHT; ++i) {
        for (j = 0; j < MAX_WIDTH; ++j) {
            input[0] = stream_Sxx->read();
            input[1] = stream_Sxy->read();
            input[2] = stream_Syy->read();

            Sxx = input[0].data;
            Sxy = input[1].data;
            Syy = input[2].data;

            #pragma HLS RESOURCE variable=tmp core=Mul
            #pragma HLS RESOURCE variable=trace core=Mul
            trace = Sxx * Syy;
            tmp =  Sxy * Sxy;
            det = trace - tmp;
    
            // fix me
            input[0].data = trace;
            stream_trace->write(input[0]);

            // fix me
            input[1].data = det;
            stream_det->write(input[1]);
        }
    }
}

void compute_response(stream_t* stream_det, stream_t* stream_trace,
    stream_t* pstrmOutput, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    AXI_PIXEL input[2];
    ap_fixed<44, 34> det, trace, response;
    for (i = 0; i < MAX_HEIGHT; ++i) {
        for (j = 0; j < MAX_WIDTH; ++j) {

            input[0] = stream_det->read();
            input[1] = stream_trace->read();

            det = ap_fixed<44, 34>(input[0].data);
            trace = ap_fixed<44, 34>(input[1].data);

            response = det - ap_fixed<12, 2>(0.05) * trace;

            // fix me
            if (response > 6000000)
                input[0].data = 1;
            else 
                input[0].data = 0;
            pstrmOutput->write(input[0]);
        }
    }
}

void HCD(stream_t* pstrmInput, stream_t* pstrmOutput, reg32_t row, reg32_t col)
{
#pragma HLS INTERFACE s_axilite port=row
#pragma HLS INTERFACE s_axilite port=col
#pragma HLS INTERFACE axis register both port=pstrmOutput
#pragma HLS INTERFACE axis register both port=pstrmInput
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
    stream_t stream_det;
    stream_t stream_trace;

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
    compute_det_trace(&stream_Sxx, &stream_Syy, &stream_Sxy, &stream_det, &stream_trace, row, col);

    // Step 6: Compute the response of the detector by det/(trace+1e-12)
    // Step 7: Post processing
    compute_response(&stream_det, &stream_trace, pstrmOutput, row, col);

    return;
}
