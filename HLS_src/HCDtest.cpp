#include <iostream>
#include <fstream>
#include <string>
#include "HCD.h"

using namespace std;

//Arrays to send and receive data from the accelerator
AXI_PIXEL input[MAX_WIDTH][MAX_HEIGHT];
int gold_output[MAX_WIDTH][MAX_HEIGHT];

int main () {
    int x,y;
    int tmp;
    reg32_t width, height;

    stream_ti strmInput;
    stream_to strmOutput;

    //read file
    fstream fin1;
    fstream fin2;

    // change path if needed !
    fin1.open("../../../../HLS_src/1_in.txt", ios::in);
    fin2.open("../../../../HLS_src/1_gold_img.txt", ios::in);

    if (!fin1.is_open() || !fin2.is_open()) {
      cout << "fail reading file" << endl;
      return 1;
    }

    fin1 >> width;
    fin1 >> height;
    width = MAX_WIDTH;
    height = MAX_HEIGHT;
    for(int i = 0; i < width; ++i) {
        for(int j = 0; j < height; ++j) {
            fin1 >> tmp;
            input[i][j].data.range(7, 0) = tmp;
            fin1 >> tmp;
            input[i][j].data.range(15, 8) = tmp;
            fin1 >> tmp;
            input[i][j].data.range(23, 16) = tmp;
            strmInput.write(input[i][j]);
        }
    }

    for(int i = 0 ; i < width ; ++i) {
        for(int j = 0 ; j < height ; ++j) {
            fin2 >> tmp;
            gold_output[i][j] = tmp;
        }
    }

    // Hardware Function
    HCD(strmInput, strmOutput, width, height);

    // for(int i = 0 ; i < width ; ++i) {
    //     for(int j = 0 ; j < height ; ++j) {
    //         auto tmp = strmOutput.read().data;
    //         if(gold_output[i][j] == 1 || tmp == 1) {
    //             cout << '(' << i << ", " << j <<  "), gold: " << gold_output[i][j] << ", ANS: " << tmp << endl;
    //         }
    //     }
    // }

    return 0;
}
