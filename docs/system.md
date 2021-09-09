# System
In this file, we will introduce our HCD system, including host and kernel.
## System Overview
![](https://i.imgur.com/V0vNf9l.png)
The figure above shows our overall system. We implement a host program in python, which can be run by using Jupyter Notebook. On the host side, the first step is to acquire an image and sent the image to the PL side. The interface is AXI-stream. The image resolution is at most 1920x1080. We also implement HCD using python API, which can be run on the CPU. Because our hardware implementation is based on the result of CPU code, the user can run CPU code to get a golden result. After the kernel completes the computation, the host receives the result from the kernel. The interface is AXI-stream too. Finally, the host program shows the result and performance(run time). 
## PL Block Diagram
![](https://i.imgur.com/BnUb0a2.png)
The figure above shows our PL block diagram. The inferface in our implementaion is AXI-stream. The main kernel function is HCD_0. Our hardware can be deployed on PYNQ-Z2 board. The block diagram is generated by Vivado 2019.2. 
## Kernel Flow
The figure below shows our kernel flow. The description of the flow can be seen in background.md. The code example can be seen in optimization.md.
![](https://i.imgur.com/wG9FZ1z.png)