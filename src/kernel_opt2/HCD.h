#ifndef _HCD_H_
#define _HCD_H_
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <ap_fixed.h>
#include "ap_video.h"

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080


// data[0:7]:G, data[8:15]:B, data[16:23]:R
// top function input, output
typedef ap_axiu<32,1,1,1> AXI_PIXEL;
typedef hls::stream<AXI_PIXEL> stream_io;

// pixel
typedef ap_int<32> PIXEL;
typedef ap_uint<1> BPIXEL; // binary pixel
typedef ap_int<10> GPIXEL; // gray pixel
typedef ap_int<20> DGPIXEL; // double gray pixel

// pixel vector stream
typedef hls::stream<PIXEL> PIXEL_STREAM;
typedef hls::stream<BPIXEL> BPIXEL_STREAM;
typedef hls::stream<GPIXEL> GPIXEL_STREAM;
typedef hls::stream<DGPIXEL> DGPIXEL_STREAM;

// window and line buffer used in image processing
typedef ap_linebuffer<GPIXEL, 3, MAX_WIDTH> BUFFER_3;
typedef ap_linebuffer<DGPIXEL, 3, MAX_WIDTH> DBUFFER_3;
typedef ap_linebuffer<DGPIXEL, 5, MAX_WIDTH> DBUFFER_5;

void HCD(
    stream_io* pstrmInput,
    stream_io* pstrmOutput,
    int row,
    int col
);

#endif
