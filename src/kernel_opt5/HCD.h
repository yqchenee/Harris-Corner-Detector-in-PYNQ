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

struct PIXEL {ap_uint<8> r, g, b;}
typedef hls::vector<PIXEL, N> PIXEL_VEC;
typedef ap_uint<10> GPIXEL;
typedef hls::vector<GPIXEL, N> GPIXEL_VEC;

typedef hls::stream<PIXEL_VEC> PIXEL_V_STREAM;
typedef hls::stream<GPIXEL_VEC> GPIXEL_V_STREAM;
typedef hls::stream<ap_int<512>> MEM_STREAM;

typedef ap_int<512> INPUT;
typedef ap_int<512> OUTPUT;


typedef ap_window<GPIXEL, 3, N+2> WINDOW;
typedef ap_window<GPIXEL, 5, N+4> WINDOW_5;
typedef ap_linebuffer<GPIXEL_VEC, 3, MAX_WIDTH/N> BUFFER_3;
typedef ap_linebuffer<GPIXEL_VEC, 5, MAX_WIDTH/N> BUFFER_5;

void HCD(
    INPUT* menInput,
    OUTPUT* menOutput,
    int row,
    int col
);

#endif
