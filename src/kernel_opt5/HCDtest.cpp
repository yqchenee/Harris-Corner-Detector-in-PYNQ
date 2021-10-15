#include <iostream>
#include <fstream>
#include <string>
#include "math.h"
#include "HCD.h"
#include "malloc.h"

using namespace std;

//Arrays to send and receive data from the accelerator
static PIXEL_VEC input;
static PIXEL_VEC output;
static int gold_output[MAX_HEIGHT][MAX_WIDTH] = {0};

int main () {
    int x,y;
    int tmp;
    unsigned int width, height;

    //read file
    fstream fin1;
    fstream fin2;

    // change path if needed !
    fin1.open("../../../../data/hls_test/1_in.txt", ios::in);
    fin2.open("../../../../data/hls_test/1_gold.txt", ios::in);

    if (!fin1.is_open() || !fin2.is_open()) {
      cout << "fail reading file" << endl;
      return 1;
    }

    fin1 >> width;
    fin1 >> height;

    if(width % N != 0) {
        cout << "image width must be factor of " << N << endl;
        return 1;
    }

    int chunk_num = BUS_WIDTH / 24 / N;
    int remain_chunk_num = (width * height * 24) % (chunk_num * 24 * N) / 24 /N;
    int input_size = ceil(width * height * 1.0 / (chunk_num * N));
    int output_size = ceil(width * height /512.0);

    INPUT mem_input[input_size];
    OUTPUT mem_output[output_size];

    ap_int<BUS_WIDTH> buf = 0;

    int i, j, k, l, lb=0;
    for (i = 0; i < input_size-1; ++i) {
    	if (i == input_size-1)
    		chunk_num = remain_chunk_num;

        // consume 24 * N * chunk_num bit
        for (j = 0; j < chunk_num; ++j) {
             // consume 24 * N bit
            for (k = 0; k < N; ++k) {
                // consume 24 bit
                for (l = 0; l < 3; ++l) {
                    fin1 >> tmp;
                    buf.range(lb+7, lb) = tmp;
                    lb += 8;
                }
            }
        }
        mem_input[i] = buf;
        lb =0;
    }

    while(!fin2.eof()){
        fin2 >> x;
        fin2 >> y;
        gold_output[x][y] = 1;
    }

    // Hardware Function
    HCD(mem_input, mem_output, height, width);

    int unmatch = 0;
    bool success = true;
    for(int i = 0; i < width; ++i) {
    	for(int j = 0; j < height; j += N) {
            int arr_index = (width*i+j) / 512;
            int arr_offset = (width*i+j) % 512;
            if(mem_output[arr_index][arr_offset] != gold_output[i][j]) {
                cout << "(" << i << ',' << j << ") Your output: " << mem_output[arr_index][arr_offset] << ", Golden: " << gold_output[i][j] << endl;
                unmatch++;
                success = false;
            }
    	}
    }
    cout << "total unmatch number :" << unmatch << endl;

    return success ? 0 : 1;
}
