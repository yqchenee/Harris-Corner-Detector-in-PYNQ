#ifndef _HCD_H_
#define _HCD_H_
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <ap_fixed.h>
#include "ap_video.h"

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

typedef ap_int<32> PIXEL;

// data[0:7]:G, data[8:15]:B, data[16:23]:R
typedef ap_axiu<32,1,1,1> AXI_PIXEL;
typedef ap_uint<32> reg32_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

// top function input, output
typedef hls::stream<AXI_PIXEL> stream_io;
typedef hls::stream<PIXEL> stream_t;
typedef ap_window<PIXEL,3,3> WINDOW;
typedef ap_linebuffer<PIXEL, 3, MAX_WIDTH> BUFFER_3;
typedef ap_linebuffer<PIXEL, 5, MAX_WIDTH> BUFFER_5;

void HCD(
    stream_io* pstrmInput,
    stream_io* pstrmOutput,
    reg32_t row,
    reg32_t col
);

#endif
