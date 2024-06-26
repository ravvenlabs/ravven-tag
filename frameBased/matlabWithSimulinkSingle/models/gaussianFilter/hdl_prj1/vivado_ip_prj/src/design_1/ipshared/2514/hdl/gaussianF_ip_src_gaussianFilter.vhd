-- -------------------------------------------------------------
-- 
-- File Name: hdl_prj1\hdlsrc\gaussianFilter\gaussianF_ip_src_gaussianFilter.vhd
-- Created: 2024-06-19 09:54:13
-- 
-- Generated by MATLAB 24.1, HDL Coder 24.1, and Simulink 24.1
-- 
-- 
-- -------------------------------------------------------------
-- Rate and Clocking Details
-- -------------------------------------------------------------
-- Model base rate: 2.77039e-06
-- Target subsystem base rate: 2.77039e-06
-- 
-- 
-- Clock Enable  Sample Time
-- -------------------------------------------------------------
-- ce_out        2.77039e-06
-- -------------------------------------------------------------
-- 
-- 
-- Output Signal                 Clock Enable  Sample Time
-- -------------------------------------------------------------
-- imageOut                      ce_out        2.77039e-06
-- imageOut_valid                ce_out        2.77039e-06
-- imageIn_ready                 ce_out        2.77039e-06
-- -------------------------------------------------------------
-- 
-- -------------------------------------------------------------


-- -------------------------------------------------------------
-- 
-- Module: gaussianF_ip_src_gaussianFilter
-- Source Path: gaussianFilter/gaussianFilter
-- Hierarchy Level: 0
-- Model version: 8.39
-- 
-- Simulink model description for gaussianFilter:
-- 
-- Sobel Edge Detection Using the MATLAB(R) Function Block
-- This example shows how to use HDL Coder(TM) to check, 
-- generate, and verify HDL code for a Sobel Edge Detection 
-- block built using the MATLAB Function block.
-- 
-- -------------------------------------------------------------
LIBRARY IEEE;
USE IEEE.std_logic_1164.ALL;
USE IEEE.numeric_std.ALL;
USE work.gaussianF_ip_src_gaussianFilter_pkg.ALL;

ENTITY gaussianF_ip_src_gaussianFilter IS
  PORT( clk                               :   IN    std_logic;
        reset                             :   IN    std_logic;
        clk_enable                        :   IN    std_logic;
        imageIn                           :   IN    std_logic_vector(31 DOWNTO 0);  -- uint32
        imageIn_valid                     :   IN    std_logic;
        imageOut_ready                    :   IN    std_logic;
        ce_out                            :   OUT   std_logic;
        imageOut                          :   OUT   std_logic_vector(31 DOWNTO 0);  -- uint32
        imageOut_valid                    :   OUT   std_logic;
        imageIn_ready                     :   OUT   std_logic
        );
END gaussianF_ip_src_gaussianFilter;


ARCHITECTURE rtl OF gaussianF_ip_src_gaussianFilter IS

  -- Component Declarations
  COMPONENT gaussianF_ip_src_uint32Tosingle2
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          u                               :   IN    std_logic_vector(31 DOWNTO 0);  -- uint32
          u_valid                         :   IN    std_logic;
          Out1                            :   OUT   std_logic_vector(31 DOWNTO 0);  -- single
          Out1_valid                      :   OUT   std_logic
          );
  END COMPONENT;

  COMPONENT gaussianF_ip_src_frameBasedGaussianFilter
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          ImageIn                         :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          data_0                          :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          data_1                          :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          data_2                          :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          data_3                          :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          data_4                          :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          data_5                          :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          data_6                          :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          data_7                          :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          data_8                          :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          ImageIn_valid                   :   IN    std_logic;
          Result                          :   OUT   std_logic_vector(31 DOWNTO 0);  -- single
          Result_valid                    :   OUT   std_logic
          );
  END COMPONENT;

  COMPONENT gaussianF_ip_src_singleToUint32
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          In1                             :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          In1_valid                       :   IN    std_logic;
          u                               :   OUT   std_logic_vector(31 DOWNTO 0);  -- uint32
          u_valid                         :   OUT   std_logic
          );
  END COMPONENT;

  COMPONENT gaussianF_ip_src_Input_FIFOs
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          enable_in                       :   IN    std_logic;
          imageIn_in                      :   IN    std_logic_vector(31 DOWNTO 0);  -- uint32
          imageIn_valid                   :   IN    std_logic;
          enable_out                      :   OUT   std_logic;
          imageIn_out                     :   OUT   std_logic_vector(31 DOWNTO 0);  -- uint32
          imageIn_ready                   :   OUT   std_logic;
          valid                           :   OUT   std_logic
          );
  END COMPONENT;

  COMPONENT gaussianF_ip_src_Output_FIFOs
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          enable_in                       :   IN    std_logic;
          y_in                            :   IN    std_logic_vector(31 DOWNTO 0);  -- uint32
          y_valid_in                      :   IN    std_logic;
          y_ready                         :   IN    std_logic;
          enable_out                      :   OUT   std_logic;
          y_out                           :   OUT   std_logic_vector(31 DOWNTO 0);  -- uint32
          y_valid                         :   OUT   std_logic
          );
  END COMPONENT;

  -- Component Configuration Statements
  FOR ALL : gaussianF_ip_src_uint32Tosingle2
    USE ENTITY work.gaussianF_ip_src_uint32Tosingle2(rtl);

  FOR ALL : gaussianF_ip_src_frameBasedGaussianFilter
    USE ENTITY work.gaussianF_ip_src_frameBasedGaussianFilter(rtl);

  FOR ALL : gaussianF_ip_src_singleToUint32
    USE ENTITY work.gaussianF_ip_src_singleToUint32(rtl);

  FOR ALL : gaussianF_ip_src_Input_FIFOs
    USE ENTITY work.gaussianF_ip_src_Input_FIFOs(rtl);

  FOR ALL : gaussianF_ip_src_Output_FIFOs
    USE ENTITY work.gaussianF_ip_src_Output_FIFOs(rtl);

  -- Signals
  SIGNAL enable                           : std_logic;
  SIGNAL enb_gated                        : std_logic;
  SIGNAL imageIn_1                        : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL valid                            : std_logic;
  SIGNAL uint32Tosingle2_out1             : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL uint32Tosingle2_out1_valid       : std_logic;
  SIGNAL c2_Constant_out1                 : vector_of_std_logic_vector32(0 TO 8);  -- ufix32 [9]
  SIGNAL Constant_out1                    : matrix_of_std_logic_vector32(0 TO 2, 0 TO 2);  -- ufix32 [3x3]
  SIGNAL Result                           : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Result_valid                     : std_logic;
  SIGNAL y                                : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL y_valid                          : std_logic;
  SIGNAL delayMatch_reg                   : std_logic_vector(196 DOWNTO 0);  -- ufix1 [197]
  SIGNAL y_valid_1                        : std_logic;
  SIGNAL outsHaveSpace                    : std_logic;
  SIGNAL y_1                              : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL y_valid_2                        : std_logic;

BEGIN
  u_uint32Tosingle2 : gaussianF_ip_src_uint32Tosingle2
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb_gated,
              u => imageIn_1,  -- uint32
              u_valid => valid,
              Out1 => uint32Tosingle2_out1,  -- single
              Out1_valid => uint32Tosingle2_out1_valid
              );

  u_frameBasedGaussianFilter : gaussianF_ip_src_frameBasedGaussianFilter
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb_gated,
              ImageIn => uint32Tosingle2_out1,  -- single
              data_0 => Constant_out1(0, 0),  -- single
              data_1 => Constant_out1(1, 0),  -- single
              data_2 => Constant_out1(2, 0),  -- single
              data_3 => Constant_out1(0, 1),  -- single
              data_4 => Constant_out1(1, 1),  -- single
              data_5 => Constant_out1(2, 1),  -- single
              data_6 => Constant_out1(0, 2),  -- single
              data_7 => Constant_out1(1, 2),  -- single
              data_8 => Constant_out1(2, 2),  -- single
              ImageIn_valid => uint32Tosingle2_out1_valid,
              Result => Result,  -- single
              Result_valid => Result_valid
              );

  u_singleToUint32 : gaussianF_ip_src_singleToUint32
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb_gated,
              In1 => Result,  -- single
              In1_valid => Result_valid,
              u => y,  -- uint32
              u_valid => y_valid
              );

  u_Input_FIFOs : gaussianF_ip_src_Input_FIFOs
    PORT MAP( clk => clk,
              reset => reset,
              enb => clk_enable,
              enable_in => outsHaveSpace,
              imageIn_in => imageIn,  -- uint32
              imageIn_valid => imageIn_valid,
              enable_out => enable,
              imageIn_out => imageIn_1,  -- uint32
              imageIn_ready => imageIn_ready,
              valid => valid
              );

  u_Output_FIFOs : gaussianF_ip_src_Output_FIFOs
    PORT MAP( clk => clk,
              reset => reset,
              enb => clk_enable,
              enable_in => enable,
              y_in => y,  -- uint32
              y_valid_in => y_valid_1,
              y_ready => imageOut_ready,
              enable_out => outsHaveSpace,
              y_out => y_1,  -- uint32
              y_valid => y_valid_2
              );

  enb_gated <= enable AND clk_enable;

  c2_Constant_out1(0) <= X"3d69e1b1";
  c2_Constant_out1(1) <= X"3dff9724";
  c2_Constant_out1(2) <= X"3d69e1b1";
  c2_Constant_out1(3) <= X"3dff9724";
  c2_Constant_out1(4) <= X"3e8b851f";
  c2_Constant_out1(5) <= X"3dff9724";
  c2_Constant_out1(6) <= X"3d69e1b1";
  c2_Constant_out1(7) <= X"3dff9724";
  c2_Constant_out1(8) <= X"3d69e1b1";

  Constant_out1_gen1: FOR d1 IN 0 TO 2 GENERATE
    Constant_out1_gen: FOR d0 IN 0 TO 2 GENERATE
      Constant_out1(d0, d1) <= c2_Constant_out1(d0 + (d1 * 3));
    END GENERATE;
  END GENERATE;

  delayMatch_process : PROCESS (clk)
  BEGIN
    IF clk'EVENT AND clk = '1' THEN
      IF reset = '1' THEN
        delayMatch_reg <= (OTHERS => '0');
      ELSIF enb_gated = '1' THEN
        delayMatch_reg(0) <= y_valid;
        delayMatch_reg(196 DOWNTO 1) <= delayMatch_reg(195 DOWNTO 0);
      END IF;
    END IF;
  END PROCESS delayMatch_process;

  y_valid_1 <= delayMatch_reg(196);

  imageOut_valid <= y_valid_2;

  ce_out <= clk_enable;

  imageOut <= y_1;

END rtl;

