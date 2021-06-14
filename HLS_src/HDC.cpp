#include "HCD.h"
#include "cmath"


/**
 * Calculate 1st derivative of image along x
 * **/
GRAY_PIXEL compute_ix(WINDOW *window)
{
    GRAY_PIXEL pixel = 0;
    char i;

    const char op[3] = {1,0,-1};

    for(i=0; i < 3; i++){
        pixel = window->getval(i,0) * op[i];
    }

    return pixel;
}

/**
 * Calculate 1st derivative of image along y
 * **/
GRAY_PIXEL compute_iy(WINDOW *window)
{
    GRAY_PIXEL pixel = 0;
    char i;

    const char op[3] = {1,0,-1};

    for(i=0; i < 3; i++){
        pixel = window->getval(0,i) * op[i];
    }

    return pixel;
}

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
GRAY_PIXEL Gaussian_filter_1(WINDOW* window)
{
    char i,j;
    double sum = 0;
    GRAY_PIXEL pixel =0;

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

void process_input(AXI_PIXEL* input, ALL_BUFFER* gray_buf, int32_t i, int32_t j)
{

    GRAY_PIXEL input_gray_pix;
    input_gray_pix = (input->data.range(7,0)
            + input->data.range(15,8)
            + input->data.range(23,16))/3;

    gray_buf->insert_at(input_gray_pix, i, j);
}

void blur_img(ALL_BUFFER* gray_buf, ALL_BUFFER* blur_buf, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    GRAY_PIXEL blur;
    WINDOW gray_window;

    for (i = 1; i < row-1; ++i) {
        for (j = 0; j < col; ++j) {

            gray_window.shift_right();
	        gray_window.insert(gray_buf->getval(i-1,j),0,2);
	        gray_window.insert(gray_buf->getval(i,j),1,2);
	        gray_window.insert(gray_buf->getval(i+1,j),2,2);

            if (j < 2) {
                continue;
            }

            blur = Gaussian_filter_15(&gray_window);
            blur_buf->insert_at(blur, i, j-1);
        }
    }
}


void compute_dif(ALL_BUFFER* blur_buf, ALL_BUFFER* Ixx_buf,
        ALL_BUFFER* Iyy_buf, ALL_BUFFER* Ixy_buf, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    GRAY_PIXEL Ix, Iy, Ixx, Iyy, Ixy;
    WINDOW blur_window;
    for (i = 1; i < row-1; ++i) {
        for (j = 0; j < col; ++j) {
            blur_window.shift_right();
	        blur_window.insert(blur_buf->getval(i-1,j),0,2);
	        blur_window.insert(blur_buf->getval(i,j),1,2);
	        blur_window.insert(blur_buf->getval(i+1,j),2,2);

            if (j < 2) {
                continue;
            }

            Ix = compute_ix(&blur_window);
            Iy = compute_iy(&blur_window);
            Ixx = Ix * Ix;
            Iyy = Iy * Iy;
            Ixy = Ix * Iy;
            Ixx_buf->insert_at(Ixx, i, j-1);
            Iyy_buf->insert_at(Iyy, i, j-1);
            Ixy_buf->insert_at(Ixy, i, j-1);
        }
    }
}

void compute_sum(ALL_BUFFER* Ixx_buf, ALL_BUFFER* Ixy_buf,
        ALL_BUFFER* Iyy_buf, TWICE_BUFFER* matrix, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;

    GRAY_PIXEL Sxx, Sxy, Syy;
    WINDOW Ixx_window;
    WINDOW Iyy_window;
    WINDOW Ixy_window;

    for (i = 1; i < row-1; ++i) {
        for (j = 0; j < col; ++j) {
            Ixx_window.shift_right();
	        Ixx_window.insert(Ixx_buf->getval(i-1,j),0,2);
	        Ixx_window.insert(Ixx_buf->getval(i,j),1,2);
	        Ixx_window.insert(Ixx_buf->getval(i+1,j),2,2);

            if (j < 2) {
                continue;
            }
            Sxx = Gaussian_filter_1(&Ixx_window);
            matrix->insert_at(Sxx, i, j-1);
        }
    }

    for (i = 1; i < row-1; ++i) {
        for (j = 0; j < col; ++j) {
            Iyy_window.shift_right();
	        Iyy_window.insert(Iyy_buf->getval(i-1,j),0,2);
	        Iyy_window.insert(Iyy_buf->getval(i,j),1,2);
	        Iyy_window.insert(Iyy_buf->getval(i+1,j),2,2);

            if (j < 2) {
                continue;
            }
            Syy = Gaussian_filter_1(&Iyy_window);
            matrix->insert_at(Syy, MAX_HEIGHT + i, MAX_WIDTH + j-1);
        }
    }

    for (i = 1; i < row-1; ++i) {
        for (j = 0; j < col; ++j) {
            Ixy_window.shift_right();
	        Ixy_window.insert(Ixy_buf->getval(i-1,j),0,2);
	        Ixy_window.insert(Ixy_buf->getval(i,j),1,2);
	        Ixy_window.insert(Ixy_buf->getval(i+1,j),2,2);

            if (j < 2) {
                continue;
            }
            Sxy = Gaussian_filter_1(&Ixy_window);
            matrix->insert_at(Sxy, MAX_HEIGHT + i, j-1);
            matrix->insert_at(Sxy, i, MAX_WIDTH + j-1);
        }
    }
}

void compute_det_trace(TWICE_BUFFER* matrix, ALL_BUFFER* det_buf,
        ALL_BUFFER* trace_buf, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    GRAY_PIXEL Ixx, Ixy, Iyy, det;
    for (i = 0; i < row; ++i) {
        for (j = 0; j < col; ++j) {
            Ixx = matrix-> getval(i, j);
            Ixy = matrix-> getval(row, col + j);
            Ixx = matrix-> getval(row + i, col + j);
            // TODO: float precision
            det = Ixx * Iyy;
            trace_buf-> insert_at(det, i, j);
            // TODO: float precision
            det -= Ixy * Ixy;
            det_buf->insert_at(det, i, j);
        }
    }
}

void compute_response(ALL_BUFFER* det_buf, ALL_BUFFER* trace_buf,
        ALL_BUFFER* response_buf, int32_t row, int32_t col)
{
    int32_t i;
    int32_t j;
    for (i = 0; i < row; ++i) {
        for (j = 0; j < col; ++j) {
            double response = double(det_buf-> getval(i, j)) / double(trace_buf-> getval(i, j));
            response_buf->insert_at(GRAY_PIXEL(response), i, j);
        }
    }
}

void HCD(stream_ti* pstrmInput, stream_to* pstrmOutput, reg32_t* corner, reg32_t row, reg32_t col)
{
#pragma HLS INTERFACE axis register both port=pstrmOutput
#pragma HLS INTERFACE axis register both port=pstrmInput
#pragma HLS INTERFACE s_axilite port=row
#pragma HLS INTERFACE s_axilite port=col
#pragma HLS INTERFACE s_axilite port=corner

    int32_t i;
    int32_t j;
    AXI_PIXEL input;
    POS test;

    ALL_BUFFER gray_buf;
    ALL_BUFFER blur_buf;
    ALL_BUFFER Ixx_buf;
    ALL_BUFFER Iyy_buf;
    ALL_BUFFER Ixy_buf;
    ALL_BUFFER det_buf;
    ALL_BUFFER trace_buf;
    ALL_BUFFER response_buf;

    TWICE_BUFFER matrix;

    for (i = 0; i < row; ++i) {
        for (j = 0; j < col; ++j) {
            input = pstrmInput->read();
             // Step 0: Convert to gray scale
            process_input(&input, &gray_buf, i, j);
        }
    }

    // Step 1: Smooth the image by Gaussian kernel
    blur_img(&gray_buf, &blur_buf, row, col);

    // Step 2: Calculate Ix, Iy (1st derivative of image along x and y axis)
    // Step 3: Compute Ixx, Ixy, Iyy (Ixx = Ix*Ix, ...)
    compute_dif(&blur_buf, &Ixx_buf, &Iyy_buf,
            &Ixy_buf, row, col);

    // Step 4: Compute Sxx, Sxy, Syy (weighted summation of Ixx, Ixy, Iyy in neighbor pixels)
    compute_sum(&Ixx_buf, &Ixy_buf, &Iyy_buf, &matrix, row, col);

    // Todo:
    // Step 5: Compute the det and trace of matrix M (M = [[Sxx, Sxy], [Sxy, Syy]])
    compute_det_trace(&matrix, &det_buf, &trace_buf, row, col);

    // Step 6: Compute the response of the detector by det/(trace+1e-12)
    compute_response(&det_buf, &trace_buf, &response_buf, row, col);

    // Step 7: Post processing


    *corner = 84;
    for (i = 0; i< *corner; i++) {
        pstrmOutput->write(test);
    }
    return;
}
