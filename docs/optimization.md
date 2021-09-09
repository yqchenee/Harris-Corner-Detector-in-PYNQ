# HCD Optimization
In this file, we will dig into the details of how we implement and which optimization methods we use in our HCD kernel design.
The details of how the kernel works can be seen in background.md.

## Code Example
First, the implementation of HCD is seperated into top function and several sub-functions, you can have a clear look in figure 1.
![](https://i.imgur.com/QiyNkMT.png)

-------
### Top function (figure 1a)
```
1    void HCD(stream_io* pstrmInput, stream_io* pstrmOutput, reg32_t row, reg32_t col)
2    {
3    #pragma HLS INTERFACE s_axilite port=row
4    #pragma HLS INTERFACE s_axilite port=col
5    #pragma HLS INTERFACE axis register both port=pstrmOutput
6    #pragma HLS INTERFACE axis register both port=pstrmInput
7    #pragma HLS INTERFACE s_axilite port=return
8    #pragma HLS DATAFLOW
9
10        int32_t i;
11        int32_t j;
12        stream_t stream_gray;
13        stream_t stream_blur;
14        stream_t stream_Ixx;
15        stream_t stream_Ixy;
16        stream_t stream_Iyy;
17        stream_t stream_Sxx;
18        stream_t stream_Sxy;
19        stream_t stream_Syy;
20        stream_t stream_response;
21
22        process_input(pstrmInput, &stream_gray, row, col);
23        blur_img(&stream_gray, &stream_blur, row, col);
24        compute_dif(&stream_blur, &stream_Ixx, &stream_Iyy, &stream_Ixy, row, col);
25        blur_diff(&stream_Ixx, &stream_Iyy, &stream_Ixy, &stream_Sxx, &stream_Syy, &stream_Sxy, row, col);
26        compute_det_trace(&stream_Sxx, &stream_Syy, &stream_Sxy, &stream_response, row, col);
27        find_local_maxima(&stream_response, pstrmOutput, row, col);
28
29        return;
30    }
```
#### Interface
The top function uses four parameters, the first and second parameters  are used to stream in and out of HCD kernel, and their datatype are objects of class stream, which is one of the template classes provided by the Vivado HLS libraries. The third and forth parameters are used to input the width (col) and height (row) of the picture which needs to go through the HCD process.
* 0 < width (col) < 1920
* 0 < height (row) < 1080
#### Optimization
Since we want all the sub-functions to execute in parallel, that is, all the sub-functions can read in and write out the data continuouly and not be blocked by the others. 
Therefore, we use **dataflow** pragma in **line 8** to help us to achieve our goal.

----------
### Sub-functions (figure 1b)
```
1    void sub_function(stream* input, stream* output, int row, int col)
2    {
3        for i in row {
4            for j in col 
5            {
6            #pragma HLS pipeline
7                tmp = input-> read();
8            
9                if(condition) {
10                   some operations on tmp;
11               }
12                
13               output-> write(tmp);
14           }
15       }
16   }
```
All the sub-functions follow this structure.
In addition, we also need some optimizations in sub-functions. Due to the resources limitation and the characteristic of dataflow, we utilize **pipeline** pragma in final, and it can be seen in **line 6**.
The following results show the difference between having pipelining or not in sub-functions.
#### Results without pipelining
![](https://i.imgur.com/pBufvgK.png)

#### Results with pipelining
![](https://i.imgur.com/FGn68QL.png)

-----------
### Floating point precision comparison
In this section, we will compare the different between float and ap_fixed, which is also one of the template classes provided by the Vivado HLS libraries.
The following code is the example of image filter used in sub-function blur_img and blur_diff. 
We can see that in **line 4** and **line 7** the variables sum and op are implemented in ap_fixed.
```
1    PIXEL Gaussian_filter_1(WINDOW* window)
2    {
3        char i,j;
4        ap_fixed<27, 17> sum = 0;
5        PIXEL pixel =0;
6
7        ap_fixed<12, 1> op[3][3] =
8        {
9            {0.0751136, 0.123841, 0.0751136},
10           {0.123841, 0.20418, 0.123841},
11           {0.0751136, 0.123841, 0.0751136}
12       };

13       for (i = 0; i < 3; i++) {
14           for (j = 0; j < 3; j++) {
15               #pragma HLS pipeline
16               sum += window->getval(i,j) * op[i][j];
17           }
18       }
19       pixel = PIXEL(sum);
20       return pixel;
21   }
```
#### Results with float
![](https://i.imgur.com/SP8l3kp.png)

#### Results with ap_fixed
![](https://i.imgur.com/AUXwRXa.png)

Based on the results, we know that if we calculate the variable sum using ap_fixed data type, the usage of the resources and the iteration latency can be reduced significantly, that is, it saves some unnecessary resources and time when doing this operation.
The reason of unnecessary is that, in this application, ap_fixed can fully meet our demands, we can save some time and resources by not doing the floating point operations.
