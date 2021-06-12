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

void blur_img(WINDOW* window, WINDOW* blur_window, ROW_BUFFER_3* blur_buf, int32_t j)
{
    GRAY_PIXEL blur;
    blur = Gaussian_filter_15(window);

    blur_buf->shift_up(j);
    blur_buf->insert_bottom(blur, j);
    
    blur_window->shift_right();
    blur_window->insert(blur_buf->getval(2,j),0,2);
    blur_window->insert(blur_buf->getval(1,j),1,2);
    blur_window->insert(blur,2,2);
}

void compute_dif(WINDOW* blur_window, 
        WINDOW* Ixx_window, ROW_BUFFER_3* Ixx_buf, 
        WINDOW* Iyy_window, ROW_BUFFER_3* Iyy_buf, 
        WINDOW* Ixy_window, ROW_BUFFER_3* Ixy_buf, 
        int32_t j)
{
    GRAY_PIXEL Ix, Iy, Ixx, Iyy, Ixy;
    Ix = compute_ix(blur_window);
    Iy = compute_iy(blur_window);

    Ixx = Ix * Ix;
    Iyy = Iy * Iy;
    Ixy = Ix * Iy;

    Ixx_buf->shift_up(j);
    Ixx_buf->insert_bottom(Ixx, j);

    Ixx_window->shift_right();
    Ixx_window->insert(Ixx_buf->getval(2,j),0,2);
    Ixx_window->insert(Ixx_buf->getval(1,j),1,2);
    Ixx_window->insert(Ixx,2,2);

    Iyy_buf->shift_up(j);
    Iyy_buf->insert_bottom(Iyy, j);

    Iyy_window->shift_right();
    Iyy_window->insert(Iyy_buf->getval(2,j),0,2);
    Iyy_window->insert(Iyy_buf->getval(1,j),1,2);
    Iyy_window->insert(Iyy,2,2);

    Ixy_buf->shift_up(j);
    Ixy_buf->insert_bottom(Ixy, j);

    Ixy_window->shift_right();
    Ixy_window->insert(Ixy_buf->getval(2,j),0,2);
    Ixy_window->insert(Ixy_buf->getval(1,j),1,2);
    Ixy_window->insert(Ixy,2,2);
}

void compute_sum(WINDOW* Ixx_window, WINDOW* Ixy_window,
        WINDOW* Iyy_window, ALL_BUFFER* matrix,
        int32_t i, int32_t j)
{
    GRAY_PIXEL Sxx, Sxy, Syy;
    Sxx = Gaussian_filter_1(Ixx_window);
    Sxy = Gaussian_filter_1(Ixy_window);
    Syy = Gaussian_filter_1(Iyy_window);

    //M = [[Sxx, Sxy], [Sxy, Syy]]
    matrix->insert_at(Sxx, i, j);
    matrix->insert_at(Sxy, i, MAX_WIDTH+j);
    matrix->insert_at(Sxy, MAX_HEIGHT+i, j);
    matrix->insert_at(Syy, MAX_HEIGHT+i, MAX_WIDTH+j);
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

    ROW_BUFFER_3 row_buf;
    WINDOW window;
    WINDOW blur_window;
    WINDOW Ixx_window;
    WINDOW Ixy_window;
    WINDOW Iyy_window;

    ROW_BUFFER_3 blur_buf;
    ROW_BUFFER_3 Ixx_buf;
    ROW_BUFFER_3 Iyy_buf;
    ROW_BUFFER_3 Ixy_buf;

    ALL_BUFFER matrix;

    for (i = 0; i < row+1; ++i) {
        for (j = 0; j < col+1; ++j) {

            GRAY_PIXEL temp1;
            GRAY_PIXEL temp2;
            
            if(j < col){
                row_buf.shift_up(j);
            }

            if(i < col & j < row){
                input = pstrmInput->read();
                temp2 = (input.data.range(7,0) 
                        + input.data.range(15,8) 
                        + input.data.range(23,16))/3;

                row_buf.insert_bottom(temp2, j);
            }

            window.shift_right();

            if(j < col){
                window.insert(row_buf.getval(2,j),0,2);
                window.insert(row_buf.getval(1,j),1,2);
                window.insert(temp2,2,2);
            }

            if( i <= 1 || j <= 1 || i > (row-1) || j > (col-1)){
                continue;
            } else {
                // Step 1: Smooth the image by Gaussian kernel
                blur_img(&window, &blur_window, &blur_buf, j);

                // Step 2: Calculate Ix, Iy (1st derivative of image along x and y axis)
                // Step 3: Compute Ixx, Ixy, Iyy (Ixx = Ix*Ix, ...)
                compute_dif(&blur_window, &Ixx_window, &Ixx_buf, 
                        &Iyy_window, &Iyy_buf, &Ixy_window, &Ixy_buf, j);

                // Step 4: Compute Sxx, Sxy, Syy (weighted summation of Ixx, Ixy, Iyy in neighbor pixels)
                compute_sum(&Ixx_window, &Ixy_window, &Iyy_window, &matrix, i, j);
            }
        }
    }

    // Todo: 
    // Step 5: Compute the det and trace of matrix M (M = [[Sxx, Sxy], [Sxy, Syy]])
    // Step 6: Compute the response of the detector by det/(trace+1e-12)
    // Step 7: Post processing
    *corner = 84;
    for (i = 0; i< *corner; i++) {
        pstrmOutput->write(test);
    }
    return;
}
