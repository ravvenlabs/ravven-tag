-- -------------------------------------------------------------
-- 
-- File Name: hdl_prj1\hdlsrc\gaussianFilter\gaussianF_ip_src_uint32Tosingle2.vhd
-- Created: 2024-06-19 09:54:13
-- 
-- Generated by MATLAB 24.1, HDL Coder 24.1, and Simulink 24.1
-- 
-- -------------------------------------------------------------


-- -------------------------------------------------------------
-- 
-- Module: gaussianF_ip_src_uint32Tosingle2
-- Source Path: gaussianFilter/gaussianFilter/uint32Tosingle2
-- Hierarchy Level: 1
-- Model version: 8.39
-- 
-- -------------------------------------------------------------
LIBRARY IEEE;
USE IEEE.std_logic_1164.ALL;
USE IEEE.numeric_std.ALL;
USE work.gaussianF_ip_src_gaussianFilter_pkg.ALL;

ENTITY gaussianF_ip_src_uint32Tosingle2 IS
  PORT( clk                               :   IN    std_logic;
        reset                             :   IN    std_logic;
        enb                               :   IN    std_logic;
        u                                 :   IN    std_logic_vector(31 DOWNTO 0);  -- uint32
        u_valid                           :   IN    std_logic;
        Out1                              :   OUT   std_logic_vector(31 DOWNTO 0);  -- single
        Out1_valid                        :   OUT   std_logic
        );
END gaussianF_ip_src_uint32Tosingle2;


ARCHITECTURE rtl OF gaussianF_ip_src_uint32Tosingle2 IS

  -- Component Declarations
  COMPONENT gaussianF_ip_src_nfp_pow_single
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          nfp_in1                         :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          nfp_in2                         :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          nfp_out                         :   OUT   std_logic_vector(31 DOWNTO 0)  -- single
          );
  END COMPONENT;

  COMPONENT gaussianF_ip_src_nfp_convert_fix_23_En0_to_single
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          nfp_in                          :   IN    std_logic_vector(22 DOWNTO 0);  -- ufix23
          nfp_out                         :   OUT   std_logic_vector(31 DOWNTO 0)  -- single
          );
  END COMPONENT;

  COMPONENT gaussianF_ip_src_nfp_gain_pow2_single
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          nfp_in1                         :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          nfp_in2                         :   IN    std_logic;  -- ufix1
          nfp_in3                         :   IN    std_logic_vector(8 DOWNTO 0);  -- sfix9
          nfp_out                         :   OUT   std_logic_vector(31 DOWNTO 0)  -- single
          );
  END COMPONENT;

  COMPONENT gaussianF_ip_src_nfp_add_single
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          nfp_in1                         :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          nfp_in2                         :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          nfp_out                         :   OUT   std_logic_vector(31 DOWNTO 0)  -- single
          );
  END COMPONENT;

  COMPONENT gaussianF_ip_src_SimpleDualPortRAM_generic
    GENERIC( AddrWidth                    : integer;
             DataWidth                    : integer
             );
    PORT( clk                             :   IN    std_logic;
          enb                             :   IN    std_logic;
          wr_din                          :   IN    std_logic_vector(DataWidth - 1 DOWNTO 0);  -- generic width
          wr_addr                         :   IN    std_logic_vector(AddrWidth - 1 DOWNTO 0);  -- generic width
          wr_en                           :   IN    std_logic;  -- ufix1
          rd_addr                         :   IN    std_logic_vector(AddrWidth - 1 DOWNTO 0);  -- generic width
          dout                            :   OUT   std_logic_vector(DataWidth - 1 DOWNTO 0)  -- generic width
          );
  END COMPONENT;

  COMPONENT gaussianF_ip_src_nfp_mul_single
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          nfp_in1                         :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          nfp_in2                         :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          nfp_out                         :   OUT   std_logic_vector(31 DOWNTO 0)  -- single
          );
  END COMPONENT;

  COMPONENT gaussianF_ip_src_nfp_convert_fix_8_En0_to_single
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          nfp_in                          :   IN    std_logic_vector(7 DOWNTO 0);  -- uint8
          nfp_out                         :   OUT   std_logic_vector(31 DOWNTO 0)  -- single
          );
  END COMPONENT;

  COMPONENT gaussianF_ip_src_nfp_sub_single
    PORT( clk                             :   IN    std_logic;
          reset                           :   IN    std_logic;
          enb                             :   IN    std_logic;
          nfp_in1                         :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          nfp_in2                         :   IN    std_logic_vector(31 DOWNTO 0);  -- single
          nfp_out                         :   OUT   std_logic_vector(31 DOWNTO 0)  -- single
          );
  END COMPONENT;

  -- Component Configuration Statements
  FOR ALL : gaussianF_ip_src_nfp_pow_single
    USE ENTITY work.gaussianF_ip_src_nfp_pow_single(rtl);

  FOR ALL : gaussianF_ip_src_nfp_convert_fix_23_En0_to_single
    USE ENTITY work.gaussianF_ip_src_nfp_convert_fix_23_En0_to_single(rtl);

  FOR ALL : gaussianF_ip_src_nfp_gain_pow2_single
    USE ENTITY work.gaussianF_ip_src_nfp_gain_pow2_single(rtl);

  FOR ALL : gaussianF_ip_src_nfp_add_single
    USE ENTITY work.gaussianF_ip_src_nfp_add_single(rtl);

  FOR ALL : gaussianF_ip_src_SimpleDualPortRAM_generic
    USE ENTITY work.gaussianF_ip_src_SimpleDualPortRAM_generic(rtl);

  FOR ALL : gaussianF_ip_src_nfp_mul_single
    USE ENTITY work.gaussianF_ip_src_nfp_mul_single(rtl);

  FOR ALL : gaussianF_ip_src_nfp_convert_fix_8_En0_to_single
    USE ENTITY work.gaussianF_ip_src_nfp_convert_fix_8_En0_to_single(rtl);

  FOR ALL : gaussianF_ip_src_nfp_sub_single
    USE ENTITY work.gaussianF_ip_src_nfp_sub_single(rtl);

  -- Signals
  SIGNAL u_unsigned                       : unsigned(31 DOWNTO 0);  -- uint32
  SIGNAL Bit_Slice2_out1                  : std_logic;  -- ufix1
  SIGNAL Data_Type_Conversion_out1        : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Constant2_out1                   : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Data_Type_Conversion_out1_1      : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Data_Type_Conversion_out1_2      : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Math_Function_out1               : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Bit_Slice1_out1                  : unsigned(22 DOWNTO 0);  -- ufix23
  SIGNAL Data_Type_Conversion1_out1       : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL pw2_sign_const                   : std_logic;  -- ufix1
  SIGNAL pw2_shift_const                  : signed(8 DOWNTO 0);  -- sfix9
  SIGNAL Product2_out1                    : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Constant3_out1                   : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Sum_out1                         : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL delayMatch_regin                 : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL delayMatch_waddr                 : unsigned(4 DOWNTO 0);  -- ufix5
  SIGNAL delayMatch_wrenb                 : std_logic;  -- ufix1
  SIGNAL delayMatch_raddr                 : unsigned(4 DOWNTO 0);  -- ufix5
  SIGNAL delayMatch_regout                : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Sum_out1_1                       : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Product_out1                     : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL delayMatch1_reg                  : vector_of_std_logic_vector32(0 TO 5);  -- ufix32 [6]
  SIGNAL Product_out1_1                   : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Constant4_out1                   : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Bit_Slice_out1                   : unsigned(7 DOWNTO 0);  -- uint8
  SIGNAL Data_Type_Conversion2_out1       : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Constant5_out1                   : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Sum1_out1                        : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Math_Function1_out1              : std_logic_vector(31 DOWNTO 0);  -- ufix32
  SIGNAL Product1_out1                    : std_logic_vector(31 DOWNTO 0);  -- ufix32

BEGIN
  u_nfp_pow_comp : gaussianF_ip_src_nfp_pow_single
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb,
              nfp_in1 => Constant2_out1,  -- single
              nfp_in2 => Data_Type_Conversion_out1_2,  -- single
              nfp_out => Math_Function_out1  -- single
              );

  u_gaussianFilter_gaussianFilter_uint32Tosingle2_nfp_convert_fix_23_En0_to_single : gaussianF_ip_src_nfp_convert_fix_23_En0_to_single
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb,
              nfp_in => std_logic_vector(Bit_Slice1_out1),  -- ufix23
              nfp_out => Data_Type_Conversion1_out1  -- single
              );

  u_nfp_gain_pow2_single : gaussianF_ip_src_nfp_gain_pow2_single
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb,
              nfp_in1 => Data_Type_Conversion1_out1,  -- single
              nfp_in2 => pw2_sign_const,  -- ufix1
              nfp_in3 => std_logic_vector(pw2_shift_const),  -- sfix9
              nfp_out => Product2_out1  -- single
              );

  u_nfp_add_comp : gaussianF_ip_src_nfp_add_single
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb,
              nfp_in1 => Product2_out1,  -- single
              nfp_in2 => Constant3_out1,  -- single
              nfp_out => Sum_out1  -- single
              );

  u_ShiftRegisterRAM : gaussianF_ip_src_SimpleDualPortRAM_generic
    GENERIC MAP( AddrWidth => 5,
                 DataWidth => 32
                 )
    PORT MAP( clk => clk,
              enb => enb,
              wr_din => delayMatch_regin,
              wr_addr => std_logic_vector(delayMatch_waddr),
              wr_en => delayMatch_wrenb,  -- ufix1
              rd_addr => std_logic_vector(delayMatch_raddr),
              dout => delayMatch_regout
              );

  u_nfp_mul_comp : gaussianF_ip_src_nfp_mul_single
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb,
              nfp_in1 => Math_Function_out1,  -- single
              nfp_in2 => Sum_out1_1,  -- single
              nfp_out => Product_out1  -- single
              );

  u_gaussianFilter_gaussianFilter_uint32Tosingle2_nfp_convert_fix_8_En0_to_single : gaussianF_ip_src_nfp_convert_fix_8_En0_to_single
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb,
              nfp_in => std_logic_vector(Bit_Slice_out1),  -- uint8
              nfp_out => Data_Type_Conversion2_out1  -- single
              );

  u_nfp_sub_comp : gaussianF_ip_src_nfp_sub_single
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb,
              nfp_in1 => Data_Type_Conversion2_out1,  -- single
              nfp_in2 => Constant5_out1,  -- single
              nfp_out => Sum1_out1  -- single
              );

  u_nfp_pow_comp_1 : gaussianF_ip_src_nfp_pow_single
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb,
              nfp_in1 => Constant4_out1,  -- single
              nfp_in2 => Sum1_out1,  -- single
              nfp_out => Math_Function1_out1  -- single
              );

  u_nfp_mul_comp_1 : gaussianF_ip_src_nfp_mul_single
    PORT MAP( clk => clk,
              reset => reset,
              enb => enb,
              nfp_in1 => Product_out1_1,  -- single
              nfp_in2 => Math_Function1_out1,  -- single
              nfp_out => Product1_out1  -- single
              );

  u_unsigned <= unsigned(u);

  Bit_Slice2_out1 <= u_unsigned(31);

  Data_Type_Conversion_out1 <= X"00000000";

  Constant2_out1 <= X"bf800000";

  Data_Type_Conversion_out1_1 <= X"3f800000";

  
  Data_Type_Conversion_out1_2 <= Data_Type_Conversion_out1 WHEN Bit_Slice2_out1 = '0' ELSE
      Data_Type_Conversion_out1_1;

  Bit_Slice1_out1 <= u_unsigned(22 DOWNTO 0);

  pw2_sign_const <= '0';

  pw2_shift_const <= to_signed(-16#017#, 9);

  Constant3_out1 <= X"3f800000";

  -- Input register for RAM-based shift register delayMatch
  delayMatch_reginc_process : PROCESS (clk)
  BEGIN
    IF clk'EVENT AND clk = '1' THEN
      IF reset = '1' THEN
        delayMatch_regin <= X"00000000";
      ELSIF enb = '1' THEN
        delayMatch_regin <= Sum_out1;
      END IF;
    END IF;
  END PROCESS delayMatch_reginc_process;


  -- Count limited, Unsigned Counter
  --  initial value   = 0
  --  step value      = 1
  --  count to value  = 17
  -- Write address counter for RAM-based shift register delayMatch
  delayMatch_wr_process : PROCESS (clk)
  BEGIN
    IF clk'EVENT AND clk = '1' THEN
      IF reset = '1' THEN
        delayMatch_waddr <= to_unsigned(16#00#, 5);
      ELSIF enb = '1' THEN
        IF delayMatch_waddr >= to_unsigned(16#11#, 5) THEN 
          delayMatch_waddr <= to_unsigned(16#00#, 5);
        ELSE 
          delayMatch_waddr <= delayMatch_waddr + to_unsigned(16#01#, 5);
        END IF;
      END IF;
    END IF;
  END PROCESS delayMatch_wr_process;


  delayMatch_wrenb <= '1';

  -- Count limited, Unsigned Counter
  --  initial value   = 1
  --  step value      = 1
  --  count to value  = 17
  -- Read address counter for RAM-based shift register delayMatch
  delayMatch_rd_process : PROCESS (clk)
  BEGIN
    IF clk'EVENT AND clk = '1' THEN
      IF reset = '1' THEN
        delayMatch_raddr <= to_unsigned(16#01#, 5);
      ELSIF enb = '1' THEN
        IF delayMatch_raddr >= to_unsigned(16#11#, 5) THEN 
          delayMatch_raddr <= to_unsigned(16#00#, 5);
        ELSE 
          delayMatch_raddr <= delayMatch_raddr + to_unsigned(16#01#, 5);
        END IF;
      END IF;
    END IF;
  END PROCESS delayMatch_rd_process;


  -- Output register for RAM-based shift register delayMatch
  delayMatch_regoutc_process : PROCESS (clk)
  BEGIN
    IF clk'EVENT AND clk = '1' THEN
      IF reset = '1' THEN
        Sum_out1_1 <= X"00000000";
      ELSIF enb = '1' THEN
        Sum_out1_1 <= delayMatch_regout;
      END IF;
    END IF;
  END PROCESS delayMatch_regoutc_process;


  delayMatch1_process : PROCESS (clk)
  BEGIN
    IF clk'EVENT AND clk = '1' THEN
      IF reset = '1' THEN
        delayMatch1_reg <= (OTHERS => X"00000000");
      ELSIF enb = '1' THEN
        delayMatch1_reg(0) <= Product_out1;
        delayMatch1_reg(1 TO 5) <= delayMatch1_reg(0 TO 4);
      END IF;
    END IF;
  END PROCESS delayMatch1_process;

  Product_out1_1 <= delayMatch1_reg(5);

  Constant4_out1 <= X"40000000";

  Bit_Slice_out1 <= u_unsigned(30 DOWNTO 23);

  Constant5_out1 <= X"42fe0000";

  Out1 <= Product1_out1;

  Out1_valid <= u_valid;

END rtl;
