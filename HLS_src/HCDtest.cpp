#include <iostream>
#include <fstream>
#include <string>
#include "HCD.h"

using namespace std;

//Arrays to send and receive data from the accelerator
static AXI_PIXEL input[MAX_HEIGHT][MAX_WIDTH];
static int gold_output[MAX_HEIGHT][MAX_WIDTH];

int main () {
    int x,y;
    int tmp;
    AXI_PIXEL pixel;
    unsigned int width, height;
    unsigned int corner_count = 0, out_corner;

    stream_t strmInput;
    stream_t strmOutput;

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
            gold_output[i][j] = 0;
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
        fin2 >> x;
        fin2 >> y;
        gold_output[x][y] = 1;
    }
    //for empty last line in the file
    //corner_count = corner_count - 1;

    // Hardware Function
    HCD(&strmInput, &strmOutput, height, width);
    for(int i = 0; i < width; ++i) {
    	for(int j = 0; j < height; ++j) {
    		tmp = strmOutput.read().data;
    		if(gold_output[i][j] != tmp) {
    			cout << "(" << i << ',' << j << ") Your output: " << tmp << ", Golden: " << gold_output[i][j] << endl;
    		}
    	}
    }

    //check answer
    /*if(out_corner != corner_count) {
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
    }*/

    return 0;
}
