# FinalProject-HwSwCoDesign_SDK_and_HLS_Code

In this repository, you will find the complete code for the Vivado HLS IP core and the code for the software application used to run the alarm system on the Zynq platform. The code is organized into two main directories: the `HLS_IP_Core` directory where the files related to the HLS IP core are located, including the main C/C++ source file `IC_main.cpp` and accompanying header file `IC.h`. The folder also contains the files used for testing the IP core, such as `ic_tb.cpp`, the input data file `ic_input.dat`, and the golden output file `ic_golden.dat`.

The `SDK` directory contains the files made to run the alarm system on the Zynq platform, which includes the `main.cc` file that hold the main application code, the two folder `camera`and `sensors` contains the header and source files for the camera and sensor modules where the random number generation for simulating the input data is implemented. The folder `utilities` has the file `systemDefinitions.h` which contains system-wide definitions and configurations. 


