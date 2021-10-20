#ifndef _HCD_H_
#define _HCD_H_
#define AP_INT_MAX_W 4096
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <hls_vector.h>
#include <ap_fixed.h>
#include "ap_video.h"

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

typedef unsigned int uint32_t;

#define N 2
#define v_ind(i) (i)/N
#define v_ind_ind(i) (i)%N

#define DATA_WIDTH 512
#define BURST_SIZE 3

// interface
typedef ap_int<DATA_WIDTH> INPUT;
typedef ap_int<DATA_WIDTH> OUTPUT;
typedef ap_int<DATA_WIDTH*BURST_SIZE> DATA;

// pixel
struct PIXEL {ap_uint<8> r, b, g;};
typedef ap_uint<1> BPIXEL; // binary pixel
typedef ap_int<10> GPIXEL; // gray pixel
typedef ap_int<20> DGPIXEL; // double gray pixel

// pixel vector
typedef hls::vector<PIXEL, N> PIXEL_VEC;
typedef hls::vector<BPIXEL, N> BPIXEL_VEC;
typedef hls::vector<GPIXEL, N> GPIXEL_VEC;
typedef hls::vector<DGPIXEL, N> DGPIXEL_VEC;

// pixel vector stream
typedef hls::stream<PIXEL_VEC> PIXEL_V_STREAM;
typedef hls::stream<BPIXEL_VEC> BPIXEL_V_STREAM;
typedef hls::stream<GPIXEL_VEC> GPIXEL_V_STREAM;
typedef hls::stream<DGPIXEL_VEC> DGPIXEL_V_STREAM;
typedef hls::stream<DATA> MEM_STREAM;

// sliding window
typedef ap_window<GPIXEL, 3, N+2> WINDOW;
typedef ap_window<GPIXEL, 5, N+4> WINDOW_5;
typedef ap_window<DGPIXEL, 3, N+2> DWINDOW;
typedef ap_window<DGPIXEL, 5, N+4> DWINDOW_5;

// row buffer
typedef ap_linebuffer<GPIXEL_VEC, 3, MAX_WIDTH/N> BUFFER_3;
typedef ap_linebuffer<GPIXEL_VEC, 5, MAX_WIDTH/N> BUFFER_5;
typedef ap_linebuffer<DGPIXEL_VEC, 3, MAX_WIDTH/N> DBUFFER_3;
typedef ap_linebuffer<DGPIXEL_VEC, 5, MAX_WIDTH/N> DBUFFER_5;

void HCD(
    INPUT* menInput,
    OUTPUT* menOutput,
    int row,
    int col
);

#endif

