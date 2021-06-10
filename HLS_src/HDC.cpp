#include "HCD.h"
#include "cmath"


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

            GRAY_PIXEL Ix, Iy, Ixx, Ixy, Iyy, blur, Sxx, Sxy, Syy;
            if( i <= 1 || j <= 1 || i > (row-1) || j > (col-1)){
                continue;
            } else{
                // Todo : 
                // Step 1: Smooth the image by Gaussian kernel
                // Unsolved : need buffer
                blur = Gaussian_filter_15(&window);

                // Step 2: Calculate Ix, Iy (1st derivative of image along x and y axis)
                // Unsolved : window is from blur 
                Ix = compute_ix(&window);
                Iy = compute_iy(&window);

                // Step 3: Compute Ixx, Ixy, Iyy (Ixx = Ix*Ix, ...)
                // Unsolved : need buffer
                Ixx = Ix * Ix;
                Iyy = Iy * Iy;
                Ixy = Ix * Iy;

                // Step 4: Compute Sxx, Sxy, Syy (weighted summation of Ixx, Ixy, Iyy in neighbor pixels)
                // Unsolved : window is from Ixx buffer
                Sxx = Gaussian_filter_1(&window);
                Sxy = Gaussian_filter_1(&window);
                Syy = Gaussian_filter_1(&window);

                // Step 5: Compute the det and trace of matrix M (M = [[Sxx, Sxy], [Sxy, Syy]])
                // Step 6: Compute the response of the detector by det/(trace+1e-12)
            }

        }
    }

    *corner = 84;
    for (i = 0; i< *corner; i++) {
        pstrmOutput->write(test);
    }
    return;
}
