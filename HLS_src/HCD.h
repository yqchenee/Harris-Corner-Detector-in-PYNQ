#ifndef _HCD_H_
#define _HCD_H_
#include <ap_axi_sdata.h>
#include <hls_stream.h>

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080


/*// I/O Image Settings
#define INPUT_IMAGE_BASE "test_1080p"
#define OUTPUT_IMAGE_BASE "result_1080p"*/

//data[0:7]:G, data[8:15]:B, data[16:23]:R
typedef ap_axiu<24,1,1,1> AXI_PIXEL;
//data[0:9]:x, data[10:19]:y
typedef ap_axiu<20,1,1,1> POS;
typedef ap_uint<32> reg32_t;

typedef hls::stream<AXI_PIXEL> stream_ti;
typedef hls::stream<POS> stream_to;


void HCD(
    stream_ti* pstrmInput, 
    stream_to* pstrmOutput,
    reg32_t* corner,
    reg32_t row,
    reg32_t col
);

#endif
