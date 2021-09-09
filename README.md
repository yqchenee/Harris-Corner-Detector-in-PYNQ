# Harris Corner Detector in PYNQ

## Introduction
**Harris Corner Detector** is a technique to extract features of corners in the image. (for more info, refer to [background.pdf](./docs/background.pdf)) <br>
In this project, we implement a **Harris Corner Detector** using Xilinx Vivado HLS. The hardware design can be deployed to PYNQ-Z2 borad.
Our implementation is low latency and low calculation error, and we explore some optimization methods for further acceleration of the process.

## Reprequistites
* Vitis tool version 2020.2 
* Evaluation platofrm (PYNQ-Z2)

## Folder structure
    ./
    ├── src/                # source code
        ├── host/           # host code for pynq
        ├── kernel_opt1/    # kernel code without HLS pragma
        ├── kernel_opt2/    # kernel code with inline, unrolling
        └── kernel_opt3/    # kernel code with inline, unrolling, pipeline
    ├── data/               # data for testing
    ├── hls_test/           # test data for xilinx HLS tool.
    ├── host_test/          # test data for pynq's jupyter notebook
    ├── docs/               # pdf of our present
    ├── impl_result/        # bit and hwh file and synthesis report for each kernel_optx
    └── build/              # include tcl file for vivado

## Getting Started 
1. Clone this project, and import kernel code to Vivado HLS (version 2019.2), generate a IP called **HCD**.
2. Import **HCD** IP to Vivado (version 2019.2), then add one DMA (for read and for write), after run auto connection, the block diagram should like following figure.
    ![image](https://github.com/yqchenee/ACA_21S_final/blob/master/docs/block_diagram.png)
3. Generate bitstream from Vivado GUI or tcl file in build folder, then test your bitstream in pynq.
4. for more information, see [build.pdf](./docs/build.pdf) and [system.md](./docs/system.md)

## Run Test
* HLS Vivado
    1. Import HCDtest.cpp to test bench
    2. Put **[data/hls_test](./data/hls_test)** folder to the same directory of hls project root.
    3. Run simulation.

* PYNQ host
    1. Upload **[src/host](./src/host/)** folder to pynq's jupyter, then put **[data/host_test](./data/host_test)** folder to the same folder.
    2. Upload your bitstream and hardware file.
    3. Execute **[HCDhost.ipynb](./src/host/HCDhost.ipynb)** in jupyter notebook, following is the figure after correctly execute it.
        ![image](https://github.com/yqchenee/ACA_21S_final/blob/master/docs/host_test_result.png)
        

## Result
* [kerner_opt1](./src/kernel_opt1) : origin design without HLS pragma
* [kerner_opt2](./src/kernel_opt2) : optimize delay with unrolling
* [kerner_opt3](./src/kernel_opt3) : optimize delay with unrolling and pipeline

Watch detail result in [result.md](./docs/result.md)
