# Harris Corner Detector in PYNQ

## Introduction
In this project, we implement a **Harris Corner Detector** with low latency using Xilinx tool, and deploy our design to PYNQ.

## Major Optimization

## Folder structure
* **src/** : source code
    * **host/** : host code for pynq
    * **kernel_opt1/** : kernel code without HLS pragma
    * **kernel_opt2/** : kernel code with inline, unrolling, pipeline, dataflow

* **data/** : data for testing
    * **hls_test/** : test data for xilinx HLS tool.
    * **host_test/** : test data for pynq's jupyter notebook

* **docs/** :  pdf of our present

* **impl_result/** :  bit and hwh file and synthesis report for each kernel_optx

* **build/** : include tcl file for vivado

## Getting Started
1. Clone this project, and import kernel code to Vivado HLS (version 2019.2), generate a IP called **HCD**.
2. 
## Run Test

