# Harris Corner Detector in PYNQ

## Introduction
**Harris Corner Detector (HCD)** is a technique to extract features of corners in the image. (for more info, refer to [background.md](./docs/background.md))  
In this project, we implement HCD using **Xilinx Vitis HLS**. The hardware design can be deployed to PYNQ-Z2 borad.
The objectives of our implementation are low latency and high throughput, thus we explore some optimization methods for further acceleration of the process.

## Major Features
The following list the features of this project, refer to [optimization.md](./docs/optimization.md) for more infomation.
* **Hardware implementation** of image processing algorithm (HCD) using Xilinx Vivado HLS
* Using several HLS pragma (**dataflow**, **pipeline**) to enhance the overall performance
* **Parallel processing**: Use vector type of **PE level parallelism** like SIMD to increase HCD throughput
* **m_axi** interface: Efficient use of memory - packed 24-bit datatype in memory

## Folder structure
    ./
    ├── build/              # include tcl file for vitis hls synthesis and hw implementation
    ├── data/               # data for testing
        ├── hls_test/           # test data for xilinx HLS tool
        └── host_test/          # test data for pynq's jupyter notebook
    ├── docs/               # documents of this project, including details of background, system and optimization
    └── src/                # source code
        ├── host/               # host code for pynq
        ├── kernel_basic/       # kernel code basic version
        ├── kernel_opt1/        # kernel code with optimization v1
        ├── kernel_opt2/        # kernel code with optimization v2
        ├── kernel_opt3/        # kernel code with optimization v3
        ├── kernel_opt4/        # kernel code with optimization v4
        └── kernel_opt5/        # kernel code with optimization v5




## Getting Started
### Reprequistites
* Vitis tool version 2020.2
* PYNQ-Z2 board
### HLS synthesis (Vitis tool)
1. Clone this project, and import kernel code to Vivado HLS (version 2020.2), generate a IP called **HCD**.
> CLI example for running [opt4](./src/kernel_opt4), it can be changed to optx (x = 1~5).
> ```
> git clone https://github.com/yqchenee/ACA_21S_final.git
> vitis_hls -f build/script_opt4.tcl
> ```
### HW implementation (Vitis tool)
2. Import **HCD** IP to Vivado, then add one DMA (for read and for write), after run auto connection, the block diagram should like following figure.
![image](https://github.com/yqchenee/ACA_21S_final/blob/master/docs/block_diagram.png)
3. Generate bitstream from Vivado GUI or tcl file in build folder, then test your bitstream in pynq.
4. for more information, refer to [system.md](./docs/system.md)

### Run Test on PYNQ host (PYNQ-Z2 board)
1. Upload **[src/host](./src/host/)** folder to pynq's jupyter, then put **[data/host_test](./data/host_test)** folder to the same folder.
2. Upload your bitstream and hardware file.
3. Execute **[HCDhost.ipynb](./src/host/HCDhost.ipynb)** in jupyter notebook, following is the figure after correctly execute it.
![image](https://github.com/yqchenee/ACA_21S_final/blob/master/docs/host_test_result.png)


## Result
The detailed results are in [result.md](./docs/result.md)
| design                    | basic optimizations | sub-function pipelining | compact streaming interface | parallel processing | m_axi interface | latency (ms) | BRAM | DSP | FF | LUT |
| :-----------------------------: | :-----------------: |:-----------------------:|:---------------------------:|:-------------------:|:---------------:| :------------: |:----:|:---:|:----:|:-----:|
| [basic](./../src/kernel_basic/) |         v           |                         |                             |                     |                 | 31.708  |  80  | 2 | 9214 | 10352 |
| [opt1](./../src/kernel_opt1/)   |         v           |            v            |                             |                     |                 | 0.666   |  164 | 2 | 17557| 15446 |
| [opt2](./../src/kernel_opt2/)   |         v           |                         |              v              |                     |                 | 31.708  | 54 | 7  | 6821 | 8644 |
| [opt3](./../src/kernel_opt3/)   |         v           |            v            |              v              |                     |                 |  0.666 | 116 | 38 | 14468 | 12463 |
| [opt4](./../src/kernel_opt4/)(N=2)   |         v      |            v            |              v              |          v          |                 | 0.382 | 48 | 76  | 15965 | 16721 |
| [opt4](./../src/kernel_opt4/)(N=4)   |         v      |            v            |              v              |          v          |                 | 0.298 | 56 | 138 | 23465 | 26791 |
| [opt5](./../src/kernel_opt5/)   |         v           |            v            |              v              |          v          |       v         | 0.384 | 112 | 76 | 30157 | 45530 |
| CPU [1]                         |                     |                         |                             |                     |                 | 235 | x | x | x | x |

[1] [Python code](./../src/host/HCD.py) runs on Intel(R) Xeon(R) CPU E5-2650 v2 @ 2.60GHz.
