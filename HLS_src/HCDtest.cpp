#include <iostream>
#include <fstream>
#include <string>
#include "HCD.h"

using namespace std;

//Arrays to send and receive data from the accelerator
AXI_PIXEL input[MAX_WIDTH][MAX_HEIGHT];
POS output[MAX_HEIGHT * MAX_WIDTH];
POS gold_output[MAX_HEIGHT * MAX_WIDTH];

int main () {
    int x,y;
    int tmp;
    AXI_PIXEL pixel;
    reg32_t width, height;
    reg32_t corner_count = 5, out_corner = 10;

    stream_ti strmInput;
    stream_to strmOutput;

    //read file
    fstream fin1;
    fstream fin2;

    // change path if needed !
    fin1.open("../../../../HLS_src/1_in.txt", ios::in);
    fin2.open("../../../../HLS_src/1_gold.txt", ios::in);

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

    while(!fin2.eof()){
        fin2 >> tmp;
        gold_output[corner_count].data.range(9,0) = tmp;
        fin2 >> tmp;
        gold_output[corner_count].data.range(19,10) = tmp;
        ++corner_count;
    }
    //for empty last line in the file
    corner_count = corner_count - 1;


    // Hardware Function
    HCD(strmInput, strmOutput, &out_corner, width, height);

    //check answer
    if(out_corner != corner_count) {
        cout << "FAILED! wrong corner count" << endl;
        cout << "Golden: " << corner_count << endl;
        cout << "Your output: " << out_corner << endl;
    }

    for(int i = 0 ; i < corner_count ; i++) {
        auto tmp = strmOutput.read().data;
        if(gold_output[i].data.range(9,0) != tmp.range(9,0)
          || gold_output[i].data.range(19,10) != tmp.range(19,10)) {
            cout << "FAILED! wrong corner coordinate!" << endl;
            cout << "In corner " << i << endl;
            cout << "Golden: (" << gold_output[i].data.range(9,0) << "," << gold_output[i].data.range(19,10) << ")" << endl;
            cout << "Your output: (" << tmp.range(9,0) << "," << tmp.range(19,10) << ")" << endl;
        }
    }

    return 0;
}
