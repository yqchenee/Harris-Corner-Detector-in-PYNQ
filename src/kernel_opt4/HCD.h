#ifndef _HCD_H_
#define _HCD_H_
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <hls_vector.h>
#include <ap_fixed.h>
#include "ap_video.h"

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080


typedef ap_uint<32> reg32_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;

// top function input, output
// data[0:7]:G, data[8:15]:B, data[16:23]:R
#define N 2
typedef ap_axiu<32,1,1,1> AXI_PIXEL;
typedef hls::vector<AXI_PIXEL, N> AXI_PIXEL_vec;
typedef hls::stream<AXI_PIXEL_vec> stream_io;
typedef ap_int<32> PIXEL;
typedef hls::vector<PIXEL, N> PIXEL_vec;
typedef hls::stream<PIXEL_vec> stream_t;

typedef ap_window<PIXEL,3,3> WINDOW;
typedef ap_linebuffer<PIXEL, 3, MAX_WIDTH> BUFFER_3;
typedef ap_linebuffer<PIXEL, 5, MAX_WIDTH> BUFFER_5;

#define DATAWIDTH 512
typedef ap_uint<DATAWIDTH> uint512_dt;
typedef hls::stream<uint512_dt> uint512_stream;


void HCD(
    stream_t* pstrmInput,
    stream_t* pstrmOutput,
    reg32_t row,
    reg32_t col
);

#endif
