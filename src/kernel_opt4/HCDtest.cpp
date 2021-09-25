#include <iostream>
#include <fstream>
#include <string>
#include "HCD.h"

using namespace std;

//Arrays to send and receive data from the accelerator
static PIXEL_vec input;
static PIXEL_vec output;
static int gold_output[MAX_HEIGHT][MAX_WIDTH];

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

    for(int i = 0; i < height; ++i) {
        for(int j = 0; j < width; j+=N) {
            for(int k = 0 ; k < N ; ++k) {
                gold_output[i][j+k] = 0;
                fin1 >> tmp;
                input[k].range(7, 0) = tmp;
                fin1 >> tmp;
                input[k].range(15, 8) = tmp;
                fin1 >> tmp;
                input[k].range(23, 16) = tmp;
            }
            strmInput.write(input);
        }
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
