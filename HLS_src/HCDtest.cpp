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
  int           	x,y;
  int tmp;
  int			width, height;
  int corner_count = 0, out_corner;


  //read file
  fstream fin1;
  fstream fin2;

  fin1.open("../../../1_in.txt", ios::in);
  fin2.open("../../../1_gold.txt", ios::in);

  fin1 >> width;
  fin1 >> height;
  for(int i = 0; i < width; ++i) {
	  for(int j = 0; j < height; ++j) {
		  fin1 >> tmp;
		  input[i][j].data.range(7, 0) = tmp;
		  fin1 >> tmp;
		  input[i][j].data.range(15, 8) = tmp;
		  fin1 >> tmp;
		  input[i][j].data.range(23, 16) = tmp;
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
  //HCD(input, output, width, height, out_corner);

  //check answer
  if(out_corner != corner_count) {
	  cout << "FAILED! wrong corner count" << endl;
	  cout << "Golden: " << corner_count << endl;
	  cout << "Your output: " << out_corner << endl;
	  return 1;
  }

  for(int i = 0; i < corner_count; ++i) {
	  if(gold_output[corner_count].data.range(9,0) != output[corner_count].data.range(9,0)
		|| gold_output[corner_count].data.range(19,10) != output[corner_count].data.range(19,10)) {
		  cout << "FAILED! wrong corner coordinate!" << endl;
		  cout << "In corner " << i << endl;
		  cout << "Golden: (" << gold_output[corner_count].data.range(9,0) << "," << gold_output[corner_count].data.range(19,10) << ")" << endl;
		  cout << "Your output: (" << output[corner_count].data.range(9,0) << "," << output[corner_count].data.range(19,10) << ")" << endl;
		  return 1;
	  }


  }

  return 0;
}
