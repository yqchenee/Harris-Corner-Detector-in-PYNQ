# HCD Optimization
In this file, we will dig into the details of how we implement and which optimization methods we use in our harris corner detector (HCD) kernel design. In addition, some results will be shown after the explanation of each optimization methods.  
The details of how the kernel works can be seen in [background.md](./background.md).
* All the results below are test under image of size 256 x 256.

## Basic Optimizations
First, the implementation of HCD contains the top function (HCD) and a string of several sub-functions, you can have a clear look in figure 1.
<p align="center">
  <img src="https://i.imgur.com/QiyNkMT.png" width=60%/>
</p>

-------
### Dataflow in Top Function
Code below is the abstract of the **top function (figure 1a)**.
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
> * 0 < width (col) < 1920
> * 0 < height (row) < 1080

> data type
> * pstrmInput, pstrmOutput
>     * **AXI_PIXEL**: ap_axiu<32,1,1,1> (pstrmInput, > pstrmOutput)
>     * **stream_io**: hls::stream<**AXI_PIXEL**>
> * stream_xx
>     * **PIXEL**: ap_int<32> 
>     * **stream_t**: hls::stream<**PIXEL**>

#### Optimization
Since we want all the sub-functions to execute in parallel, that is, all the sub-functions can read in and write out the data continuouly and not be blocked by the others.  
Therefore, we use hls pragma **dataflow** in top function (**line 3**) to help us achieve our goal.

-----------
### Floating Point Precision Comparison
In this section, we will compare the different between **float** and **ap_fixed**, which is also one of the template classes provided by the Vivado HLS libraries.  
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
9            {0.123841,  0.20418,  0.123841},
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
* Result of using float (left) and ap_fixed (right)
<img align="left" src="https://i.imgur.com/SP8l3kp.png" width=47%/>
<img align="right" src="https://i.imgur.com/AUXwRXa.png" width=47%/>


Based on the results, we can see that if we calculate the variable sum using ap_fixed data type, the usage of the resources and the iteration latency can be reduced significantly, that is, it saves some unnecessary resources and time when doing this operation.  
The reason of unnecessary is that, in this application, ap_fixed can fully meet our demands, we can save some time and resources by not doing the floating point operations.

______
## Advanced Optimizations

### 1. Sub-Function Pipelining
All the **sub-functions (figure 1b)** follow the following code structure.

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
> data type
> * **PIXEL**: ap_int<32> 
> * **stream_t**: hls::stream<**PIXEL**>

#### Optimization
In additional to using the pragma **dataflow** in top function, to further increase the sub-function throughput, all the sub-functions must have the ability to read the next input when perform some operations on the previous input.  
Therefore, we use hls pragma **pipeline** in all sub-function (**line 6**) to help us achieve our goal.  
In summary, all the sub-functions can reach II=1 finally.

-------

### 2. Compact Streaming Interface
As aforementioned, all the sub-functions' interfaces are **stream_t** (stream of **32 bits**), but the data width we only need is
* **24 bits**: HCD in/output (input pixel contains RGB three channel, and each channel contains 8bits)
* **8 bits**: output of sub-function process_input (gray_pixel)
* **16 bits**: output of sub-function compute_dif (Ixx, Ixy, Iyy, Sxx, Sxy, Syy)
* **32 bits**: output of sub-function compute_response (response)

Therefore, we used different data width for different variables to reduce the resources utilization.

-------

### 3. Parallel Processing
In the beginning, we input 1 pixel of image to HCD at a time, and by II=1, the throughput of HCD is roughly 1 pixel/cycle.  
To futher increase the throughput, we utilize **hls::vector** datatype, that is, it helps us to process N pixels at a time parallel. 
<a href="https://www.codecogs.com/eqnedit.php?latex=(N&space;=&space;2^n,&space;n\epsilon&space;\mathbb{N})" target="_blank"><img src="https://latex.codecogs.com/gif.latex?(N&space;=&space;2^n,&space;n\epsilon&space;\mathbb{N})" title="(N = 2^n, n\epsilon \mathbb{N})" /></a>

* N can be defined in [HCD.h](./../src/kernel_opt4/HCD.h)
* Xilinx Vitis HLS provides a template implementation **hls::vector<T,N>** for vector type, which represents a single-instruction multiple-data (SIMD) vector of **N elements of type T**

-------

### 4. m_axi Interface
As aforementioned, we use 24 * N bits (parallel process N input pixels) for top function's in/output, but in hardware interface, the supported bandwidth is one of 32, 64, 128, 256, 512 bits.  
To fully utilize the memory bandwidth, we add some extra sub-functions at top and bottom of the original HCD algorithm.  
The following describle what we add in the whole process.
``` cpp
1    void HCD(stream_io* pstrmInput, stream_io* pstrmOutput, int row, int col)
2    {
3        #pragma HLS dataflow
4        getMem(memInput, &stream_mem, row, col);
5        men2gray_str(&stream_mem, &stream_gray, row, col);
6        blur_img(&stream_gray, &stream_blur);
7        compute_dif(&stream_blur, &stream_Ixx, &stream_Iyy, &stream_Ixy);
8        blur_diff(&stream_Ixx, &stream_Iyy, &stream_Ixy, &stream_Sxx, &stream_Syy, &stream_Sxy);
9        compute_response(&stream_Sxx, &stream_Syy, &stream_Sxy, &stream_response);
10       find_local_maxima(&stream_response, pstrmOutput);
11       str2mem(&pstrmOutput, memOutput, row, col);
12   }
```
1. In host side, we gather several image pixels (24 bits/pixel) into a block of  [_DATAWIDTH_](https://github.com/yqchenee/ACA_21S_final/blob/898023262d95db680ae0bd8d9648cbb8af128ec3/src/kernel_opt5/HCD.h#L19) bits data , and feed to kernel through m_axi interface.
2. Inside the kernel, we burst read the data, and scatter them back into the original image pixels. (line 4)
3. Go through the original HCD algorithm. (line 5 ~ 10)
4. Gather the output of the HCD algorithm and send the result back to the host side. (line 11)

_______

## Results and Comparisons
The following table summarizes the different optimize methods used in each implementations.

|                                 | basic optimizations | sub-function pipelining | compact streaming interface | parallel processing | m_axi interface |
| :-----------------------------: | :-----------------: |:-----------------------:|:---------------------------:|:-------------------:|:---------------:|
| [basic](./../src/kernel_basic/) |         v           |                         |                             |                     |                 |
| [opt1](./../src/kernel_opt1/)   |         v           |            v            |                             |                     |                 |
| [opt2](./../src/kernel_opt2/)   |         v           |                         |              v              |                     |                 |
| [opt3](./../src/kernel_opt3/)   |         v           |            v            |              v              |                     |                 |
| [opt4](./../src/kernel_opt4/)   |         v           |            v            |              v              |          v          |                 |
| [opt5](./../src/kernel_opt5/)   |         v           |            v            |              v              |          v          |       v         |

### basic
![](https://i.imgur.com/uX4Ui8P.png)

### opt1
![](https://i.imgur.com/FGn68QL.png)
> Compared to basic implementation, after **pipelining the sub-functions**:
> * achieve II=1
> * latency: 3170881 -> 66611
> * resources usage:
>     * BRAM, FF, LUT: roughly doubled
>     * DSP: not changed


### opt2
![](https://i.imgur.com/qlO3ITu.png)
> Compared to basic implementation, after applying **compact streaming interface**:
> * latency: not changed
> * resources usage:
>     * BRAM, FF, LUT: roughly 0.6x
>     * DSP: 2 -> 7

### opt3
![](https://i.imgur.com/z3P3V5W.png)
> Compared to opt1, the result is similar to opt2 compares to basic  
> Compared to opt2, the result is similar to opt1 compares to basic

### opt4 with N = 2
![](https://i.imgur.com/fktOwD9.png)
> Since we have changed the code structure used in sub-function find_local_maxima, blur_img and blur_diff to legalize the parallel processing, we don't compare it to other implementations.
> * achieve II=2
> * latency: 66622 -> 38246

### opt4 with N = 4
![](https://i.imgur.com/LOUZraa.png)
> Compared to opt4 with N=2, after **doubling N**:
> * achieve II=4
> * latency (cycles): 38246 -> 21316
> * latency (ns): 3.82E5 -> 2.98E5 (cycle time: 10ns -> 14ns)
> * resources usage:
>     * BRAM: 48 -> 56
>     * FF, LUT: roughly 1.6x
>     * DSP: 76 -> 138


### opt5
![](https://i.imgur.com/hQCXHwP.png)

