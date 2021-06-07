#include "HCD.h"

void HCD(stream_ti* pstrmInput, stream_to* pstrmOutput, reg32_t* corner, reg32_t row, reg32_t col)
{
#pragma HLS INTERFACE axis register both port=pstrmOutput
#pragma HLS INTERFACE axis register both port=pstrmInput
#pragma HLS INTERFACE s_axilite port=row
#pragma HLS INTERFACE s_axilite port=col
#pragma HLS INTERFACE s_axilite port=corner

	int32_t i;
	int32_t j;
    AXI_PIXEL pixel;
    POS test;

    for (i = 0; i < row; ++i) {
        for (j = 0; j < col; ++j) {
            pixel = pstrmInput->read();
        }
    }

    *corner = 84;
	for (i = 0; i< *corner; i++) {
        pstrmOutput->write(test);
	}
	return;
}
