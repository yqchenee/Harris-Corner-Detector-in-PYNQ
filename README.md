# Harris Corner Detector in PYNQ

## Introduction
**Harris Corner Detector (HCD)** is a technique to extract features of corners in the image. (for more info, refer to [background](./docs/background.md))  
In this project, we implement HCD using **Xilinx Vitis HLS**. The hardware design can be deployed to PYNQ-Z2 borad.
The objectives of our implementation are low latency and high throughput, thus we explore some optimization methods for further acceleration of the process.

## Major Features
The following list the features of this project, refer to [optimization.md](./docs/optimization.md) for more infomation.
* **Hardware implementation** of image processing algorithm (HCD) using Xilinx Vivado HLS
* Using several HLS pragma (**dataflow**, **pipeline**) to enhance the overall performance
* **Parallel processing** to increase HCD throughput
* **m_axi** interface

## Folder structure
    ./
    ├── build/              # include tcl file for vitis hls synthesis and hw implementation
    ├── data/               # data for testing
        ├── hls_test/           # test data for xilinx HLS tool
        └── host_test/          # test data for pynq's jupyter notebook
    ├── docs/               # documents of this project, including details of background, system and optimization
    ├── impl_result/        # bit and hwh file and synthesis report for each kernel_optx (Vivado_hls, Vivado 2019.2)
    └── src/                # source code
        ├── host/               # host code for pynq
        ├── kernel_opt1/        # kernel code without HLS pragma
        ├── kernel_opt2/        # kernel code with inline, unrolling
        └── kernel_opt3/        # kernel code with inline, unrolling, pipeline




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
4. for more information, see [build.pdf](./docs/build.pdf) and [system.md](./docs/system.md)

### Run Test on PYNQ host (PYNQ-Z2 board)
1. Upload **[src/host](./src/host/)** folder to pynq's jupyter, then put **[data/host_test](./data/host_test)** folder to the same folder.
2. Upload your bitstream and hardware file.
3. Execute **[HCDhost.ipynb](./src/host/HCDhost.ipynb)** in jupyter notebook, following is the figure after correctly execute it.
![image](https://github.com/yqchenee/ACA_21S_final/blob/master/docs/host_test_result.png)


## Result
Watch detailed results in [result.md](./docs/result.md)
