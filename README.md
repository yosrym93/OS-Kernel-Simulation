# OS Kernel Simulation
A coursework project simulating the interactions between a kernel process, a disk manager process, and user processes using interprocess communication &amp; signals.


Requirements:
Unix system, g++ compiler

Instructions:
1. Compile "process.cpp" and "disk.cpp" files using g++, name the outputs "process" and "disk" respectively.
2. Compile "kernel.cpp", put the compiled file in the same directiory as "process" and "disk" and run it.
3. Create input files for the user processes with the name specified in kernel.cpp
  
  Example: If the name is "process_input", the input files should be named "process_input0", "process_input1", ...

* Several parameters such as the number of user processes to be spawned and the common name of the input files for the user processes can be modified through the predecessor definitions at the top of kernel.cpp
