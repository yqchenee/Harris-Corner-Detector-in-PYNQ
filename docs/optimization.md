# HCD Optimization
In this file, we will dig into the details of how we implement and which optimization methods we use in our harris corner detector (HCD) kernel design. In addition, some results will be shown after the explanation of each optimization methods.

The details of how the kernel works can be seen in [background.md](./background.md).

* All the results below are test under image of size 256 x 256.

## Code Example
First, the implementation of HCD contains the top function (HCD) and a string of several sub-functions, you can have a clear look in figure 1.
<img src="https://i.imgur.com/QiyNkMT.png)" width="500"/>

-------
### Top function (figure 1a)
Code below is the abstract of the top function.
``` cpp
1    void HCD(stream_io* pstrmInput, stream_io* pstrmOutput, int row, int col)
2    {
3        #pragma HLS dataflow
4        process_input(pstrmInput, &stream_gray);
5        blur_img(&stream_gray, &stream_blur);
6        compute_dif(&stream_blur, &stream_Ixx, &stream_Iyy, &stream_Ixy);
7        blur_diff(&stream_Ixx, &stream_Iyy, &stream_Ixy, &stream_Sxx, &stream_Syy, &stream_Sxy);
8        compute_response(&stream_Sxx, &stream_Syy, &stream_Sxy, &stream_response);
9        find_local_maxima(&stream_response, pstrmOutput);
10   }
```
#### Interface
The top function uses four parameters, the first and second parameters are used to stream in and out of HCD kernel, and their datatype are objects of class stream, which is one of the template classes provided by the Vivado HLS libraries. The third and forth parameters are used to input the image size, namely width (col) and height (row). 
> parameters' bound
* 0 < width (col) < 1920
* 0 < height (row) < 1080
> datatype
* pstrmInput, pstrmOutput
    * **AXI_PIXEL**: ap_axiu<32,1,1,1> (pstrmInput, pstrmOutput)
    * **stream_io**: hls::stream<**AXI_PIXEL**>
* stream_xx
    * **PIXEL**: ap_int<32> 
    * **stream_t**: hls::stream<**PIXEL**>

#### Optimization
Since we want all the sub-functions to execute in parallel, that is, all the sub-functions can read in and write out the data continuouly and not be blocked by the others.  
Therefore, we use hls pragma **dataflow** in top function (**line 3**) to help us achieve our goal.

-----------
### Floating point precision comparison
In this section, we will compare the different between float and ap_fixed, which is also one of the template classes provided by the Vivado HLS libraries.  
The following code is the example of image filter used in sub-function blur_img and blur_diff.  
Code below shows that in **line 4** and **line 7**, the variables sum and op are implemented in ap_fixed.
```cpp
1    PIXEL Gaussian_filter_1(WINDOW* window)
2    {
3        ap_fixed<27, 17> sum = 0;
4        PIXEL pixel = 0;
5
6        ap_fixed<12, 1> op[3][3] =
7        {
8            {0.0751136, 0.123841, 0.0751136},
9           {0.123841, 0.20418, 0.123841},
10           {0.0751136, 0.123841, 0.0751136}
11       };

12       for (i = 0; i < 3; i++) {
13           for (j = 0; j < 3; j++) {
14               sum += window->getval(i,j) * op[i][j];
15           }
16       }
17       pixel = PIXEL(sum);
18       return pixel;
19   }
```
We compare the results of the implementation using float to ap_fixed under sub-function blur_img.
#### Result of using float
![](https://i.imgur.com/SP8l3kp.png)

#### Result of using ap_fixed
![](https://i.imgur.com/AUXwRXa.png)

Based on the results, we can see that if we calculate the variable sum using ap_fixed data type, the usage of the resources and the iteration latency can be reduced significantly, that is, it saves some unnecessary resources and time when doing this operation.  
The reason of unnecessary is that, in this application, ap_fixed can fully meet our demands, we can save some time and resources by not doing the floating point operations.

______
## Advanced Optimization

### Sub-Function Pipelining (figure 1b)
All the sub-functions follow the following code structure.

```cpp
1    void sub_function(stream_t* input, stream_t* output, int row, int col)
2    {
3        for i in row {
4            for j in col {
6                #pragma HLS PIPELINE
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

#### Interface
The sub-function uses several parameters, the first few parameters are used to stream in and out of each sub-function, their datatype are objects of class stream, which is one of the template classes provided by the Vivado HLS libraries. The last two parameters are used to input the image size, namely width (col) and height (row).  
> datatype
* **PIXEL**: ap_int<32> 
* **stream_t**: hls::stream<**PIXEL**>

#### Optimization
In additional to using the pragma **dataflow** in top function, to further increase the sub-function throughput, all the sub-functions must have the ability to read the next input when perform some operations on the previous input.  
Therefore, we use hls pragma **pipeline** in all sub-function (**line 6**) to help us achieve our goal.  
In summary, all the sub-functions can reach II=1 finally.

-------

### Compact Streaming Interface
As aforementioned, all the sub-functions' interfaces are stream of **32 bits**, but the data width we only need is
* **24 bits** for input pixel (HCD in/out put)
* **8 bits** for gray pixel (output of sub-function process_input)
* **16 bits** for Ixx, Iyy, Ixy (output of sub-function compute_dif)
* **32 bits** for response (output of sub-function compute_response)

Therefore, we used different data width for different variables to reduce the resources utilization.

-------

### Parallel Processing
In the beginning, we input 1 pixel of image to HCD at a time, and by II=1, the throughput of HCD is roughly 1 pixel/cycle.  
To futher increase the throughput, we utilize **hls::vector** datatype, that is, it helps us to process N pixels at a time parallel. (<a href="https://www.codecogs.com/eqnedit.php?latex=N=2^n" target="_blank"><img src="https://latex.codecogs.com/gif.latex?N=2^n" title="N=2^n" /></a>)

-------

|                               | sub-function pipelining | compact streaming interface | parallel processing | m_axi interface |
|-------------------------------|:-----------------------:|:----------------------:|:-------------------:|:---------------:|
| [opt1](./../src/kernel_opt1/) |              v          |                        |                     |                 |
| [opt2](./../src/kernel_opt2/) |                         |            v           |                     |                 |
| [opt3](./../src/kernel_opt3/) |              v          |            v           |                     |                 |
| [opt4](./../src/kernel_opt4/) |              v          |            v           |          v          |                 |
| [opt5](./../src/kernel_opt5/) |              v          |            v           |          v          |        v        |

### original
![](https://i.imgur.com/uX4Ui8P.png)

### opt1
![](https://i.imgur.com/FGn68QL.png)

### opt2
![](https://i.imgur.com/qlO3ITu.png)

### opt3
![](https://i.imgur.com/z3P3V5W.png)

### opt4
![](https://i.imgur.com/fktOwD9.png)

### opt5
![](https://i.imgur.com/hQCXHwP.png)

