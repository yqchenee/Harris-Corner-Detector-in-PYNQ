#ifndef _IMAGE_DEMO_H_
#define _IMAGE_DEMO_H_
#include "ap_axi_sdata.h"
//#include "ap_interfaces.h"





#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080


/*// I/O Image Settings
#define INPUT_IMAGE_BASE "test_1080p"
#define OUTPUT_IMAGE_BASE "result_1080p"*/

//data[0:7]:G, data[8:15]:B, data[16:23]:R
typedef ap_axiu<24,1,1,1> AXI_PIXEL;

//data[0:9]:x, data[10:19]:y
typedef ap_axiu<20,1,1,1> POS;


void HCD(
		  AXI_PIXEL input_img[MAX_WIDTH][MAX_HEIGHT],
		  POS out[MAX_HEIGHT * MAX_WIDTH],
		  int row,
		  int col,
		  int corner
		  );

#endif
