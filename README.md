midterm report link:https://docs.google.com/document/d/1g4Snl0hTN3Jz93VdDQSKA9s5Hmh7P3_vjrUKr-yqeNw/edit?usp=sharing

final report: https://docs.google.com/presentation/d/1u7tLKmdXmiCifG3KyYDpXHv1JPC1e1xm4NNQEObnwC0/edit#slide=id.p

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

## Getting Started
1. Clone this project, and import kernel code to Vivado HLS (version 2019.2), generate a IP called **HCD**.
2. 
## Run Test

