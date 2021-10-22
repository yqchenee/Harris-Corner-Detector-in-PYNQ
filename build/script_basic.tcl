############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
############################################################
open_project basic
set_top HCD
add_files src/kernel_basic/HCD.cpp
add_files src/kernel_basic/HCD.h
add_files src/kernel_basic/ap_video.h
add_files -tb src/kernel_basic/HCDtest.cpp
open_solution "solution1" -flow_target vivado
set_part {xc7z020clg484-1}
create_clock -period 10 -name default
set_clock_uncertainty 2
csim_design
csynth_design
cosim_design -trace_level all
export_design -format ip_catalog
