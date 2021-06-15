#ifndef _HCD_H_
#define _HCD_H_
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include "ap_video.h"

#define MAX_WIDTH 150
#define MAX_HEIGHT 150


/*// I/O Image Settings
#define INPUT_IMAGE_BASE "test_1080p"
#define OUTPUT_IMAGE_BASE "result_1080p"*/

typedef ap_rgb<8,8,8> RGB;
typedef ap_uint<8> GRAY_PIXEL;
typedef ap_int<9> DIF_PIXEL;        // Ix, Iy
typedef ap_int<17> DOUBLE_PIXEL;    // Ixx, Ixy, Iyy

//data[0:7]:G, data[8:15]:B, data[16:23]:R
typedef ap_axiu<24,1,1,1> AXI_PIXEL;
//data[0:9]:x, data[10:19]:y
typedef ap_axiu<20,1,1,1> POS;
typedef ap_uint<32> reg32_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef hls::stream<AXI_PIXEL> stream_ti;
typedef hls::stream<POS> stream_to;

typedef ap_window<GRAY_PIXEL,3,3> WINDOW;
typedef ap_window<DOUBLE_PIXEL,3,3> DOUBLE_WINDOW;
typedef ap_linebuffer<GRAY_PIXEL, 3, MAX_WIDTH> ROW_BUFFER_3;

typedef ap_linebuffer<GRAY_PIXEL, MAX_HEIGHT, MAX_WIDTH> GRAY_BUFFER;
typedef ap_linebuffer<DIF_PIXEL, MAX_HEIGHT, MAX_WIDTH> DIF_BUFFER;         // Ix, Iy
typedef ap_linebuffer<DOUBLE_PIXEL, MAX_HEIGHT, MAX_WIDTH> DOUBLE_BUFFER;   // Ixx, Ixy, Iyy
typedef ap_linebuffer<DOUBLE_PIXEL, 2*MAX_HEIGHT, 2*MAX_WIDTH> TWICE_BUFFER;  // matrix

void HCD(
    stream_ti& pstrmInput,
    stream_to& pstrmOutput,
    reg32_t* corner,
    reg32_t row,
    reg32_t col
);

// void compute_det_trace(TWICE_BUFFER* matrix, ALL_BUFFER* det_buf,
//         ALL_BUFFER* trace_buf, int32_t row, int32_t col);

#endif
