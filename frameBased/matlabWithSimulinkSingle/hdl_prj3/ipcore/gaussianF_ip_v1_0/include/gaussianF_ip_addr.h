/*
 * File Name:         hdl_prj3\ipcore\gaussianF_ip_v1_0\include\gaussianF_ip_addr.h
 * Description:       C Header File
 * Created:           2024-06-04 12:28:03
*/

#ifndef GAUSSIANF_IP_H_
#define GAUSSIANF_IP_H_

#define  IPCore_Reset_gaussianF_ip                          0x0  //write 0x1 to bit 0 to reset IP core
#define  IPCore_Enable_gaussianF_ip                         0x4  //enabled (by default) when bit 0 is 0x1
#define  AXI4_Stream_Video_Slave_ImageWidth_gaussianF_ip    0x8  //Active pixels per line in each video frame for "AXI4-Stream Video Slave" interface, the default value is 752.
#define  AXI4_Stream_Video_Slave_ImageHeight_gaussianF_ip   0xC  //Active video lines in each video frame for "AXI4-Stream Video Slave" interface, the default value is 480.
#define  AXI4_Stream_Video_Slave_HPorch_gaussianF_ip        0x10  //Horizontal porch length in each video frame for "AXI4-Stream Video Slave" interface, the default value is 280.
#define  AXI4_Stream_Video_Slave_VPorch_gaussianF_ip        0x14  //Vertical porch length in each video frame for "AXI4-Stream Video Slave" interface, the default value is 45.
#define  IPCore_Timestamp_gaussianF_ip                      0x18  //contains unique IP timestamp (yymmddHHMM): 2406041227
#define  coeffs_Data_gaussianF_ip                           0x100  //data register for Inport coeffs. Vector with 9 elements. Register is split across a total of 9 addresses, last address is 0x120.
#define  coeffs_Strobe_gaussianF_ip                         0x140  //strobe register for port coeffs

#endif /* GAUSSIANF_IP_H_ */
