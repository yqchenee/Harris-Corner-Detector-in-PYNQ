#ifndef _HCD_H_
#define _HCD_H_
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <hls_vector.h>
#include <ap_fixed.h>
#include "ap_video.h"

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

typedef unsigned int uint32_t;

// top function input, output
// data[0:7]:G, data[8:15]:B, data[16:23]:R
#define N 2
#define v_ind(i) (i)/N
#define v_ind_ind(i) (i)%N
typedef ap_int<32> PIXEL;
typedef hls::vector<PIXEL, N> PIXEL_vec;
typedef hls::stream<PIXEL_vec> stream_t;
typedef hls::stream<ap_int<512>> Stream_mem;

typedef ap_window<PIXEL, 3, N+2> WINDOW;
typedef ap_window<PIXEL, 5, N+4> WINDOW_5;
typedef ap_linebuffer<PIXEL_vec, 3, MAX_WIDTH/N> BUFFER_3;
typedef ap_linebuffer<PIXEL_vec, 5, MAX_WIDTH/N> BUFFER_5;

void HCD(
    ap_int<512>* menInput,
    ap_int<512>* menOutput,
    int row,
    int col
);

#endif
