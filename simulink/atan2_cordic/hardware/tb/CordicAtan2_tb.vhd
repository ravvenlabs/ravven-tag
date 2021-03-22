-- Testbench created online at:
--   https://www.doulos.com/knowhow/perl/vhdl-testbench-creation-using-perl/
-- Copyright Doulos Ltd

library IEEE;
use IEEE.Std_logic_1164.all;
use IEEE.Numeric_Std.all;
use std.textio.all;
use ieee.std_logic_textio.ALL;
USE work.CordicAtan2_pkg.ALL;

entity CordicAtan2_tb is
end;

architecture bench of CordicAtan2_tb is

  component CordicAtan2
    PORT( clk                               :   IN    std_logic;
          reset                             :   IN    std_logic;
          clk_enable                        :   IN    std_logic;
          iY                                :   IN    std_logic_vector(15 DOWNTO 0);
          iX                                :   IN    std_logic_vector(15 DOWNTO 0);
          valid_in                          :   IN    std_logic;
          ce_out                            :   OUT   std_logic;
          Angle_Mag                         :   OUT   std_logic_vector(31 DOWNTO 0);
          valid_out                         :   OUT   std_logic
          );
  end component;

  signal clk: std_logic;
  signal reset: std_logic;
  signal clk_enable: std_logic;
  signal iY: std_logic_vector(15 DOWNTO 0);
  signal iX: std_logic_vector(15 DOWNTO 0);
  signal valid_in: std_logic;
  signal ce_out: std_logic;
  signal Angle_Mag: std_logic_vector(31 DOWNTO 0);
  signal valid_out: std_logic ;

  constant clock_period: time := 20 ns;
  signal stop_the_clock: boolean;

begin

  uut: CordicAtan2 port map ( clk        => clk,
                              reset      => reset,
                              clk_enable => clk_enable,
                              iY         => iY,
                              iX         => iX,
                              valid_in   => valid_in,
                              ce_out     => ce_out,
                              Angle_Mag  => Angle_Mag,
                              valid_out  => valid_out );

  stimulus: process
    variable input_iX       : line;
    variable input_iY       : line;
    variable viX            : std_logic_vector(15 downto 0);
    variable viY            : std_logic_vector(15 downto 0);
    file iX_file            : text;
    file iY_file            : text;
  begin
    file_open(iX_file, "iX_data.txt", READ_MODE);
    file_open(iY_file, "iY_data.txt", READ_MODE);

    reset <= '1';
    wait for clock_period;
    reset <= '0';
    wait for clock_period;
    clk_enable <= '1';
    valid_in <= '1';

    while not endfile(iX_file) loop
        readline(iX_file, input_iX);
        readline(iY_file, input_iY);
        read(input_iX, viX);
        read(input_iY, viY);
        
        iX <= viX;
        iY <= viY;
        
        wait for clock_period;
    end loop;
    
    valid_in <= '0';
    
    wait until valid_out = '0';
    wait for clock_period * 2;

    stop_the_clock <= true;
    file_close(iX_file);
    file_close(iY_file);
    wait;
  end process;
  
  output_verification: process
    variable output_angle   : line;
    variable output_mag     : line;
    variable out_line       : line;
    variable angle          : std_logic_vector(15 downto 0);
    variable mag            : std_logic_vector(15 downto 0);
    variable out_ang_mag    : std_logic_vector(31 downto 0);
    file angle_file         : text;
    file mag_file           : text;
    file out_data_file      : text;
  begin
    file_open(angle_file, "angle_hex_tb_data.txt", READ_MODE);
    file_open(mag_file, "mag_hex_tb_data.txt", READ_MODE);
    file_open(out_data_file, "out_data.txt", WRITE_MODE);
    
    wait until valid_out = '1';
    while not endfile(angle_file) loop     
        wait for clock_period;
        
        if valid_out = '0' then
            exit;
        end if;
        
        readline(angle_file, output_angle);
        readline(mag_file, output_mag);
        read(output_angle, angle);
        read(output_mag, mag);
        
        if mag /= Angle_Mag(15 downto 0) then
            report "Incorrect magnitude" severity warning;
        end if;
        if angle /= Angle_Mag(31 downto 16) then
            report "Incorrect angle" severity warning;
        end if;
        
        out_ang_mag := Angle_Mag;
        write(out_line, out_ang_mag);
        writeline(out_data_file, out_line);
    end loop;
    
    file_close(angle_file);
    file_close(mag_file);
    file_close(out_data_file);
    wait;
  end process;

  clocking: process
  begin
    while not stop_the_clock loop
      clk <= '0', '1' after clock_period / 2;
      wait for clock_period;
    end loop;
    wait;
  end process;

end;