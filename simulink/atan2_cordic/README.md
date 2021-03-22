# Angle & Magnitude Calculation Via Cordic
This folder contains the working backend of an Atan2/Magnitude Cordic
calculation co-routine. The cordic implementation was tested and verified in
Matlab, simulated in Simulink, and implemented in hardware. The projects are
broken down in the following folders:
 - **matlab**: Scripts to run and verify the atan2 cordic implementation as doubles.
 - **simulation**: Simulink implementation of the algorithm.
 - **hardware**: The Vivado project containing the generated VHDL & testbench code.

## Matlab
The matlab portion of this sub-project has one main script, a testing script,
and any number of supporting scripts. These were created in version Matlab
2020b, Update 1. Below is a file list and a brief description of them.
 - **atan2_mag_cordic.m**: Main function/implementation of the atan2 cordic 
co-routine.
 - **atan2_tester.m**: Script to stress test the atan2_mag_cordic function.
 - **cordiclut_generation.m**: Generates the cordic lut.
 - **generate_tb_txt_files.m**: Generates testbench input text files for hardware.

## Simulation/Simulink
The Simulink portion of this sub-project contains varying versions of the "same"
block diagram. The different versions illustrate the different steps of creating
the final diagram that will be used to generate the VHDL. Each step ideally has
a verification stimuli and results to ensure errors aren't propogated to
different versions and can be fixed right away. Below is a brief description of
each step.
 - **atan2_mag_cordic_simu_v1.slx**: Initial implementation of atan2 cordic
co-routine. Implements it using doubles.
 - **atan2_mag_cordic_simu_v2.slx**: Conversion of the first version into fixed
point arithmetic. Required some tweaks to logic that worked as a double vs fixed
point.
 - **atan2_mag_cordic_simu_v3.slx**: HDL Coder settings and implementation. Had to
remove the testing stimuli to get it to generate. Also required some minor
tweaks.
 - **atan2_mag_cordic_simu_v4.slx**: Testing version that will produce some outputs
for the matlab script, "generate_tb_txt_files.m".

## Hardware
The Simulink portion will generate both the Vivado project and the VHDL code for
the project. The Vivado project is mostly in the correct configuration except
some files may need to be added to the project for it to work perfectly. Below
is a brief description of the various files that are found in that directory:
 - **hdlsrc**: The HDL Coder generated VHDL code.
 - **tb**: The testbench VHDL code and supporting text files. The supporting text
files may need to be generated.
 - **vivado_prj**: The Vivado project directory.

### Manual Changes Required
Once the Vivado project is created and all the auto-generated files are created,
there is a minor change that should be done at the top level vhdl file,
*CordicAtan2.vhd*. Upon opening the file, the *Angle_Mag* output will most
likely be a custom type rather than the desired
```std_logic_vector(31 downto 0)```. Change it to that type. This will produce
errors further into the file. Replace the lines of code assigning *Angle_Mag* as
an array of 2 ```std_logic_vector(15 downto 0)``` with the following:
```
Angle_Mag(15 downto 0) <= std_logic_vector(Delay4_out1(1)); -- Magnitude
Angle_Mag(31 downto 16) <= std_logic_vector(Delay4_out1(0)); -- Angle
```
The *Delay4_out1* may be different. If so, ensure that the same registers that
were auto-generated are being concatenated together. I.e: Changing *Delay4_out1*
to *Delay10_out1*.
