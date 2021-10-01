#include <iostream>
#include <fstream>
#include <string>
#include "math.h"
#include "HCD.h"
#include "malloc.h"

using namespace std;

//Arrays to send and receive data from the accelerator
static PIXEL_vec input;
static PIXEL_vec output;
static int gold_output[MAX_HEIGHT][MAX_WIDTH] = {0};

int main () {
    int x,y;
    int tmp;
    unsigned int width, height;

    stream_t strmInput;
    stream_t strmOutput;

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

    int size = ceil(width * height * 24.0 / 512);
    ap_int<512> mem_input[size];
    ap_int<512+24*N> buf;

    int batch_size = width * height / N;
    int batch_count = 0;
    int lb = 0;
    int rb = 512;
    int arr_index = 0;

    while (batch_count < batch_size) {
        while (lb < rb) {
            // consume 24 * N bit
            for(int k = 0 ; k < N ; ++k) {
                for (l = 0; l < 3; ++l) {
                    fin1 >> tmp;
                    buf.range(lb+7, lb) = tmp;
                    lb += 8;
                }
            }
            ++batch_count;
        }
        mem_input[arr_index] = buf.range(511,0);
        if (lb > rb) {
            buf.range(lb-rb,0) = buf.range(lb,rb);
            lb = lb - rb;
        } else {
            lb = 0;
        }
        arr_index++;
    }

    while(!fin2.eof()){
        fin2 >> x;
        fin2 >> y;
        gold_output[x][y] = 1;
    }

    // Hardware Function
    HCD(&strmInput, &strmOutput, height, width);

    int unmatch = 0;
    bool success = true;
    for(int i = 0; i < width; ++i) {
    	for(int j = 0; j < height; j += N) {
            output = strmOutput.read();

            for(int k = 0 ; k < N ; ++k) {
                if(output[k] != gold_output[i][j+k]) {
                    cout << "(" << i << ',' << j+k << ") Your output: " << output[k] << ", Golden: " << gold_output[i][j+k] << endl;
                    unmatch++;
                    success = false;
                }
            }
    	}
    }
    cout << "total unmatch number :" << unmatch << endl;

    return success ? 0 : 1;
}
