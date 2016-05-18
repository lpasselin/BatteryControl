/*
 * LTC6804-2_Functions.c
 *
 *  Created on: Jun 19, 2015
 *      Author: Andrew Hoang
 */

#include <stdio.h>
#include <stdbool.h>
#include "F28x_Project.h"
#include "LTC6804-2_functions.h"

Uint16 cell_voltage[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
Uint16 cell_flags[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

void LTC_wakeup()
{
  	 //Wake up serial interface
	output_low();
	DELAY_US(2);
	output_high();
	DELAY_US(14);
	output_low();
}
void LTC_refon_set()
{
	//LTC6804-2 set REFON to bit 1
	//This keeps the ADC until the watchdog timer expires
	SpiaRegs.SPITXBUF = ((unsigned)0x80) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x01) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x4D) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x7A) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xE5) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x14) << 8; //0b00010100 //these bit writings will correspond to OV flag at 3.9V and UV flag at 2.9V
	SpiaRegs.SPITXBUF = ((unsigned)0x57) << 8; //0b01010111 //for more information, refer to page 49 of the LTC6804-2 Datasheet bit value readings
	SpiaRegs.SPITXBUF = ((unsigned)0x98) << 8; //0b10011000
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;

	//calculated PECs
	Uint16 byte_CRC16[6] = {0xE5, 0x14, 0x57, 0x98, 0x00, 0x00};
	Uint16 PEC_A = 	(LTC_pec_calc(byte_CRC16, 6) & 0xFF00) >> 8;
	Uint16 PEC_B = 	(LTC_pec_calc(byte_CRC16, 6)) & 0x00FF;
	SpiaRegs.SPITXBUF = ((unsigned)PEC_A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)PEC_B) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	//time delay for ADC to warm up and turn on.
	DELAY_US(100);
	output_high();
	DELAY_US(3100); //changed to 3100 from 3900 Aug 14, 2014
	//Can immediately run next function here with LTC wake up.
}
void LTC_ADC_clear()
{
	void LTC_wakeup(void);

	LTC_wakeup();
	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x11) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xB9) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xD4) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(20);
}
void LTC_ADSTAT_clear()
{
	//ADSTAT clear
	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x13) << 8;

	Uint16 byte_CRC16[2] = {0x87, 0x13};
	Uint16 PEC_A = 	(LTC_pec_calc(byte_CRC16, 2) & 0xFF00) >> 8;
	Uint16 PEC_B = 	LTC_pec_calc(byte_CRC16, 2) & 0x00FF;
	SpiaRegs.SPITXBUF = ((unsigned)PEC_A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)PEC_B) << 8;
}
void LTC_ADC_conversion()
{
	void LTC_wakeup(void);

	LTC_wakeup();
	//ADC conversion
	SpiaRegs.SPITXBUF = ((unsigned)0x83) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x70) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xDF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x56) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	//time delay necessary for the ADC to do required conversions.
	DELAY_US(20);
	output_high();
	DELAY_US(6000);
}
void LTC_ADSTAT_conversion()
{
	void LTC_wakeup(void);

	LTC_wakeup();
	//ADSTAT conversion
	SpiaRegs.SPITXBUF = ((unsigned)0x85) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x68) << 8;

	Uint16 byte_CRC16[2] = {0x85, 0x68};
	Uint16 PEC_A = 	(LTC_pec_calc(byte_CRC16, 2) & 0xFF00) >> 8;
	Uint16 PEC_B = 	LTC_pec_calc(byte_CRC16, 2) & 0x00FF;
	SpiaRegs.SPITXBUF = ((unsigned)PEC_A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)PEC_B) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	//time delay necessary for the ADC to do required conversions.
	DELAY_US(20);
	output_high();
	DELAY_US(1900);
}


//readings stored on the voltage registers of the LTC6804-2 ADC are parsed into the
//SPIA receive buffers. Voltage readings must be extrapolated from the SPI receive buffer.
//Extracting values from receive buffer in the correct voltage order and readings can be done using the read_voltage_from_receive_buffer() code
void LTC_read_voltages_123()
{
	void LTC_wakeup(void);
	void ready_rxbuf(void);

	LTC_wakeup();
	SpiaRegs.SPITXBUF = ((unsigned)0x80) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x04) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x77) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xD6) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(20);

	//clear the receive buffer except for 8 bytes of information (the 6 voltage bytes + 2 PECs)
	ready_rxbuf();

	Uint16 rdata[8] = {0,0,0,0,0,0,0,0};
	//voltage reading buffers rdata
	rdata[0] = SpiaRegs.SPIRXBUF;
	rdata[1] = SpiaRegs.SPIRXBUF;
	rdata[2] = SpiaRegs.SPIRXBUF;
	rdata[3] = SpiaRegs.SPIRXBUF;
	rdata[4] = SpiaRegs.SPIRXBUF;
	rdata[5] = SpiaRegs.SPIRXBUF;
	rdata[6] = SpiaRegs.SPIRXBUF;
	rdata[7] = SpiaRegs.SPIRXBUF;
	//aggregates and parses the data accordingly with reference to the
	//DC2100A oscilloscope models
	Uint16 voltage_1 = rdata[1] << 8 | rdata[0];
	Uint16 voltage_2 = rdata[3] << 8 | rdata[2];
	Uint16 voltage_3 = rdata[5] << 8 | rdata[4];
	Uint16 PEC = rdata[6] << 8 | rdata[7];

	cell_voltage[0] = voltage_1;
	cell_voltage[1] = voltage_2;
	cell_voltage[2] = voltage_3;
}
void LTC_read_voltages_456()
{
	void LTC_wakeup(void);
	void ready_rxbuf(void);

	LTC_wakeup();
	SpiaRegs.SPITXBUF = ((unsigned)0x80) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x06) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xEA) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x80) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(20);

	//clear the receive buffer except for 8 bytes of information (the 6 voltage bytes + 2 PECs)
	ready_rxbuf();

	Uint16 rdata[8] = {0,0,0,0,0,0,0,0};
	//voltage reading buffers rdata
	rdata[0] = SpiaRegs.SPIRXBUF;
	rdata[1] = SpiaRegs.SPIRXBUF;
	rdata[2] = SpiaRegs.SPIRXBUF;
	rdata[3] = SpiaRegs.SPIRXBUF;
	rdata[4] = SpiaRegs.SPIRXBUF;
	rdata[5] = SpiaRegs.SPIRXBUF;
	rdata[6] = SpiaRegs.SPIRXBUF;
	rdata[7] = SpiaRegs.SPIRXBUF;
	//aggregates and parses the data accordingly with reference to the
	//DC2100A oscilloscope models
	Uint16 voltage_4 = rdata[1] << 8 | rdata[0];
	Uint16 voltage_5 = rdata[3] << 8 | rdata[2];
	Uint16 voltage_6 = rdata[5] << 8 | rdata[4];
	Uint16 PEC = rdata[6] << 8 | rdata[7];

	cell_voltage[3] = voltage_4;
	cell_voltage[4] = voltage_5;
	cell_voltage[5] = voltage_6;
}
void LTC_read_voltages_789()
{
	void LTC_wakeup(void);
	void ready_rxbuf(void);

	LTC_wakeup();
	SpiaRegs.SPITXBUF = ((unsigned)0x80) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x08) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x2E) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x46) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(20);

	//clear the receive buffer except for 8 bytes of information (the 6 voltage bytes + 2 PECs)
	ready_rxbuf();

	Uint16 rdata[8] = {0,0,0,0,0,0,0,0};
	//voltage reading buffers rdata
	rdata[0] = SpiaRegs.SPIRXBUF;
	rdata[1] = SpiaRegs.SPIRXBUF;
	rdata[2] = SpiaRegs.SPIRXBUF;
	rdata[3] = SpiaRegs.SPIRXBUF;
	rdata[4] = SpiaRegs.SPIRXBUF;
	rdata[5] = SpiaRegs.SPIRXBUF;
	rdata[6] = SpiaRegs.SPIRXBUF;
	rdata[7] = SpiaRegs.SPIRXBUF;
	//aggregates and parses the data accordingly with reference to the
	//DC2100A oscilloscope models
	Uint16 voltage_7 = rdata[1] << 8 | rdata[0];
	Uint16 voltage_8 = rdata[3] << 8 | rdata[2];
	Uint16 voltage_9 = rdata[5] << 8 | rdata[4];
	Uint16 PEC = rdata[6] << 8 | rdata[7];

	cell_voltage[6] = voltage_7;
	cell_voltage[7] = voltage_8;
	cell_voltage[8] = voltage_9;
}
void LTC_read_voltages_10_11_12()
{
	void LTC_wakeup(void);
	void ready_rxbuf(void);

	LTC_wakeup();
	SpiaRegs.SPITXBUF = ((unsigned)0x80) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x0A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xB3) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x10) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(20);

	//clear the receive buffer except for 8 bytes of information (the 6 voltage bytes + 2 PECs)
	ready_rxbuf();

	Uint16 rdata[8] = {0,0,0,0,0,0,0,0};
	//voltage reading buffers rdata
	rdata[0] = SpiaRegs.SPIRXBUF;
	rdata[1] = SpiaRegs.SPIRXBUF;
	rdata[2] = SpiaRegs.SPIRXBUF;
	rdata[3] = SpiaRegs.SPIRXBUF;
	rdata[4] = SpiaRegs.SPIRXBUF;
	rdata[5] = SpiaRegs.SPIRXBUF;
	rdata[6] = SpiaRegs.SPIRXBUF;
	rdata[7] = SpiaRegs.SPIRXBUF;
	//aggregates and parses the data accordingly with reference to the
	//DC2100A oscilloscope models
	Uint16 voltage_10 = rdata[1] << 8 | rdata[0];
	Uint16 voltage_11 = rdata[3] << 8 | rdata[2];
	Uint16 voltage_12 = rdata[5] << 8 | rdata[4];
	Uint16 PEC = rdata[6] << 8 | rdata[7];

	cell_voltage[9] = voltage_10;
	cell_voltage[10] = voltage_11;
	cell_voltage[11] = voltage_12;
}

//Serves as a precursor to the read voltage buffers and other read buffers by
//eliminating all but 8 bytes of information from the SPI receive buffer.
void ready_rxbuf()
{
	//ensures receive buffer is empty before receiving any actual data
	Uint16 data;
	while(SpiaRegs.SPIFFRX.bit.RXFFST > 8)
	{
	data = SpiaRegs.SPIRXBUF;
	}
}

//LTC pec check returns as a boolean true or false if the calculated 6 data values are accurate
//when issuing read commands back i.e reading voltages off of the ADC
//At the moment, unsure whether a PEC verification function is strictly needed for throughput calculations.
bool LTC6804_pec_verification(Uint16 *data, Uint16 PEC)
{
	Uint16 PEC_calculated = LTC_pec_calc(*data, 6);
	Uint16 PEC_given = PEC;
	if(PEC_calculated == PEC_given)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//The LTC get and read flag functions pulls data from the ADC LTC6804-2 and parses them accordingly to read out
void LTC_UVOV_get_flags()
{
	void LTC_wakeup(void);

	LTC_wakeup();
	SpiaRegs.SPITXBUF = ((unsigned)0x80) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x12) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x30) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(50);
}
void LTC_read_UVOV_flags()
{
	void ready_rxbuf(void);
	ready_rxbuf();

	Uint16 cell_flag1 = 0;
	Uint16 cell_flag2 = 0;
	Uint16 cell_flag3 = 0;
	Uint16 cell_flag4 = 0;
	Uint16 cell_flag5 = 0;
	Uint16 cell_flag6 = 0;
	Uint16 cell_flag7 = 0;
	Uint16 cell_flag8 = 0;
	Uint16 cell_flag9 = 0;
	Uint16 cell_flag10 = 0;
	Uint16 cell_flag11 = 0;
	Uint16 cell_flag12 = 0;

	//Throw first 2 receive buffer bytes away (unused)
	Uint16 datathrow1 = SpiaRegs.SPIRXBUF;
	Uint16 datathrow2 = SpiaRegs.SPIRXBUF;

	Uint16 byte1 = SpiaRegs.SPIRXBUF; //cells [4:1]
	Uint16 byte2 = SpiaRegs.SPIRXBUF; //cells [8:5]
	Uint16 byte3 = SpiaRegs.SPIRXBUF; //cells [12:9

	//-----------------------------------------------------

	//following datasheet readings of the 6 bytes (3 bytes here)
	cell_flag4 = (byte1 & 0xC0) >> 6;
	cell_flag3 = (byte1 & 0x30) >> 4;
	cell_flag2 = (byte1 & 0x0C) >> 2;
	cell_flag1 = (byte1 & 0x03);

	cell_flag8 = (byte2 & 0xC0) >> 6;
	cell_flag7 = (byte2 & 0x30) >> 4;
	cell_flag6 = (byte2 & 0x0C) >> 2;
	cell_flag5 = (byte2 & 0x03);

	cell_flag12 = (byte3 & 0xC0) >> 6;
	cell_flag11 = (byte3 & 0x30) >> 4;
	cell_flag10 = (byte3 & 0x0C) >> 2;
	cell_flag9 = (byte3 & 0x03);

	//-----------------------------------------------------

	//rather than overwrite my code, I just put the values into an array.
	cell_flags[0] = cell_flag1;
	cell_flags[1] = cell_flag2;
	cell_flags[2] = cell_flag3;
	cell_flags[3] = cell_flag4;
	cell_flags[4] = cell_flag5;
	cell_flags[5] = cell_flag6;
	cell_flags[6] = cell_flag7;
	cell_flags[7] = cell_flag8;
	cell_flags[8] = cell_flag9;
	cell_flags[9] = cell_flag10;
	cell_flags[10] = cell_flag11;
	cell_flags[11] = cell_flag12;
	Uint16 byte4 = SpiaRegs.SPIRXBUF;

	Uint16 asdf[6] = {datathrow1, datathrow2, byte1, byte2, byte3, byte4};
	Uint16 PEC_check = LTC_pec_calc(asdf, 6);

	Uint16 PEC_byte1 = SpiaRegs.SPIRXBUF << 8;
	Uint16 PEC_byte2 = SpiaRegs.SPIRXBUF;

	Uint16 PEC_VALUE = PEC_byte1 | PEC_byte2;
}

//The LTC_pec_calc function calculates a PEC value (Packet Error Code) from either 6 bytes of received
//data or 2 bytes of sent data using a lookup table. The PEC functionality has been left up to the
//higher level code and as a result, much of this code has been directly transferred over from Linear's LTC6804-2 Source code.
Uint16 LTC_pec_calc(Uint16 *data, Uint16 length)
{
    Uint16 remainder = 16; //ltc6804_pec_seed_value
    int i;
    for(i = 0; i < length; i++)
    {
        remainder = LTC6804_pec_lookup(data[i], remainder);
    }
    return (remainder * 2); //The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}
Uint16 LTC6804_pec_lookup(data, remainder)
{
	Uint16 address;
	Uint16 ltc6804_pec_table[256] =
	{   0x0000, 0xc599, 0xceab, 0x0b32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac,
	    0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1,
	    0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
	    0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b,
	    0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd,
	    0x2544, 0x02be, 0xc727, 0xcc15, 0x098c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c,
	    0x3d6e, 0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x07c2, 0xc25b, 0xc969, 0x0cf0, 0xdf0d,
	    0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
	    0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640,
	    0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
	    0x4a88, 0x8f11, 0x057c, 0xc0e5, 0xcbd7, 0xe4e,  0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b,
	    0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921,
	    0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070,
	    0x85e9, 0x0f84, 0xca1d, 0xc12f, 0x04b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528,
	    0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59,
	    0x2ac0, 0x0d3a, 0xc8a3, 0xc391, 0x0608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01,
	    0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9,
	    0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a,
	    0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25,
	    0x2fbc, 0x0846, 0xcddf, 0xc6ed, 0x0374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0x0af8, 0xcf61, 0xc453,
	    0x01ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b,
	    0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3,
	    0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095
	};

    address = ((remainder >> 7) ^ data) & 0xFF;    //calculate PEC table address
    remainder = (remainder << 8) ^ ltc6804_pec_table[address];             //get value from CRC15Table;

    return remainder;
}

//Similar to the LTC_pec_calc function, this PEC function enables the user to
//calculate the required PEC values that the LTC3300 requires to ensure throughput is accurate
Uint16 ltc3300_crc_calc(Uint16 nibble1, Uint16 nibble2, Uint16 nibble3)   // The bytes to be written to an LTC3300-1 Register
{
	Uint16 BITS_PER_NIBBLE = 4; //there are 4 bits per nibble
	Uint16 LTC3300_CRC_SIZE = 4; //CRC size to be placed into transmission command
	Uint16 LTC3300_BALANCER_CONTROL_SIZE = 2;   // 2 bits for DnA and DnB
	Uint16 ltc3300_crc_table[16] = {0x00, 0x13, 0x26, 0x35, 0x4C, 0x5F, 0x6A, 0x79,
	                                                               0x9B, 0x88, 0xBD, 0xAE, 0xD7, 0xC4, 0xF1, 0xE2};
	Uint16 nybble_num;
    Uint16 addr;
    Uint16 data[3];
    Uint16 CRC = 0;

    data[0] = nibble1;
    data[1] = nibble2;
    data[2] = nibble3;

    for (nybble_num = 0; nybble_num < 3; nybble_num++)
    {
        addr = ((CRC >> (LTC3300_CRC_SIZE - BITS_PER_NIBBLE)) ^ data[nybble_num]) & 0xF;//MASK(BITS_PER_NIBBLE, 0);   // calculate table address
        CRC = (CRC << BITS_PER_NIBBLE) ^ ltc3300_crc_table[addr];                                               // get value from table
    }

    return (~CRC) & 0x0F;
}


//Modulated code

void LTC3300_write_balance(Uint16 cell1, Uint16 cell2, Uint16 cell3, Uint16 cell4, Uint16 cell5, Uint16 cell6)
{
	if(cell1 == -1)
	{
		cell1 = 2; //2 synchronous discharge
	}
	else if(cell1 == 1)
	{
		cell1 = 3; //3 charge
	}
	else
	{
		cell1 = 0; //do nothing with that respective cell
	}

	if(cell1 == -1)
	{
		cell2 = 2; //2 synchronous discharge
	}
	else if(cell2 == 1)
	{
		cell2 = 3; //3 charge
	}
	else
	{
		cell2 = 0; //do nothing with that respective cell
	}

	if(cell3 == -1)
	{
		cell3 = 2; //2 synchronous discharge
	}
	else if(cell3 == 1)
	{
		cell3 = 3; //3 charge
	}
	else
	{
		cell3 = 0; //do nothing with that respective cell
	}

	if(cell4 == -1)
	{
		cell4 = 2; //2 synchronous discharge
	}
	else if(cell4 == 1)
	{
		cell4 = 3; //3 charge
	}
	else
	{
		cell4 = 0; //do nothing with that respective cell
	}

	if(cell5 == -1)
	{
		cell5 = 2; //2 synchronous discharge
	}
	else if(cell5 == 1)
	{
		cell5 = 3; //3 charge
	}
	else
	{
		cell5 = 0; //do nothing with that respective cell
	}

	if(cell6 == -1)
	{
		cell6 = 2; //2 synchronous discharge
	}
	else if(cell6 == 1)
	{
		cell6 = 3; //3 charge
	}
	else
	{
		cell6 = 0; //do nothing with that respective cell
	}

	Uint16 nibble1 = cell1 << 2 | cell2;
	Uint16 nibble2 = cell3 << 2 | cell4;
	Uint16 nibble3 = cell5 << 2 | cell6;
	Uint16 LTC3300_pec_value = ltc3300_crc_calc(nibble1, nibble2, nibble3);

	void LTC_wakeup(void);

	LTC_wakeup();

	//This instantiates a write command to the SPI bus to the LTC6804-2
	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x21) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x54) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xA6) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	//write command with respective ICOM/FCOM commands
	//first 2 bytes are to initiate LTC3300 broadcast with write balance in mind
	SpiaRegs.SPITXBUF = ((unsigned)0x8A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x98) << 8;
	//cell input here
	SpiaRegs.SPITXBUF = (0x8 << 4 | nibble1) << 8;
	SpiaRegs.SPITXBUF = (nibble2 << 4 | 0x8) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = (0x8 << 4 | nibble3) << 8;
	SpiaRegs.SPITXBUF = (LTC3300_pec_value << 4 | 0x9) << 8;

	Uint16 byte1 = 0x8A;
	Uint16 byte2 = 0x98;
	Uint16 byte3 = (0x8 << 4 | nibble1);
	Uint16 byte4 = (nibble2 << 4 | 0x8);
	Uint16 byte5 = (0x8 << 4 | nibble3);
	Uint16 byte6 = (LTC3300_pec_value << 4 | 0x9);


	Uint16 byte_CRC16[6] = {byte1, byte2, byte3, byte4, byte5, byte6};
	Uint16 PEC_A = 	(LTC_pec_calc(byte_CRC16, 6) & 0xFF00) >> 8;
	Uint16 PEC_B = 	LTC_pec_calc(byte_CRC16, 6) & 0x00FF;

	SpiaRegs.SPITXBUF = ((unsigned)PEC_A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)PEC_B) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(30);
	LTC_wakeup();

	//STCOMM prepare LTC6804-2 for transmission to external modules (in this case LTC3300)
	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x23) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xC9) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(30);
}
void LTC3300_execute_balance(Uint16 cell1, Uint16 cell2, Uint16 cell3, Uint16 cell4, Uint16 cell5, Uint16 cell6)
{
	if(cell1 == -1)
	{
		cell1 = 2; //2 synchronous discharge
	}
	else if(cell1 == 1)
	{
		cell1 = 3; //3 charge
	}
	else
	{
		cell1 = 0; //do nothing with that respective cell
	}

	if(cell1 == -1)
	{
		cell2 = 2; //2 synchronous discharge
	}
	else if(cell2 == 1)
	{
		cell2 = 3; //3 charge
	}
	else
	{
		cell2 = 0; //do nothing with that respective cell
	}

	if(cell3 == -1)
	{
		cell3 = 2; //2 synchronous discharge
	}
	else if(cell3 == 1)
	{
		cell3 = 3; //3 charge
	}
	else
	{
		cell3 = 0; //do nothing with that respective cell
	}

	if(cell4 == -1)
	{
		cell4 = 2; //2 synchronous discharge
	}
	else if(cell4 == 1)
	{
		cell4 = 3; //3 charge
	}
	else
	{
		cell4 = 0; //do nothing with that respective cell
	}

	if(cell5 == -1)
	{
		cell5 = 2; //2 synchronous discharge
	}
	else if(cell5 == 1)
	{
		cell5 = 3; //3 charge
	}
	else
	{
		cell5 = 0; //do nothing with that respective cell
	}

	if(cell6 == -1)
	{
		cell6 = 2; //2 synchronous discharge
	}
	else if(cell6 == 1)
	{
		cell6 = 3; //3 charge
	}
	else
	{
		cell6 = 0; //do nothing with that respective cell
	}

	Uint16 nibble1 = cell1 << 2 | cell2;
	Uint16 nibble2 = cell3 << 2 | cell4;
	Uint16 nibble3 = cell5 << 2 | cell6;
	Uint16 LTC3300_pec_value = ltc3300_crc_calc(nibble1, nibble2, nibble3);

	void LTC_wakeup(void);

	LTC_wakeup();

	//This instantiates a write command to the SPI bus to the LTC6804-2
	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x21) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x54) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xA6) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	//write command with respective ICOM/FCOM commands
	//first 2 bytes are to initiate LTC3300 broadcast with write balance in mind
	SpiaRegs.SPITXBUF = ((unsigned)0x8A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF8) << 8;
	//cell input here
	SpiaRegs.SPITXBUF = (0x8 << 4 | nibble1) << 8;
	SpiaRegs.SPITXBUF = (nibble2 << 4 | 0x8) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = (0x8 << 4 | nibble3) << 8;
	SpiaRegs.SPITXBUF = (LTC3300_pec_value << 4 | 0x9) << 8;

	Uint16 byte1 = 0x8A;
	Uint16 byte2 = 0xF8;
	Uint16 byte3 = (0x8 << 4 | nibble1);
	Uint16 byte4 = (nibble2 << 4 | 0x8);
	Uint16 byte5 = (0x8 << 4 | nibble3);
	Uint16 byte6 = (LTC3300_pec_value << 4 | 0x9);


	Uint16 byte_CRC16[6] = {byte1, byte2, byte3, byte4, byte5, byte6};
	Uint16 PEC_A = 	(LTC_pec_calc(byte_CRC16, 6) & 0xFF00) >> 8;
	Uint16 PEC_B = 	LTC_pec_calc(byte_CRC16, 6) & 0x00FF;

	SpiaRegs.SPITXBUF = ((unsigned)PEC_A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)PEC_B) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(30);
	LTC_wakeup();

	//STCOMM prepare LTC6804-2 for transmission to external modules (in this case LTC3300)
	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x23) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xC9) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
}










//-------------------------------------------------------------------------------------------------------------------
//Code to be scrapped
void LTC_i2c_read_example()
{

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x21) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x54) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xA6) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x6A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x18) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x0F) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x0F) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x47) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x38) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	DELAY_US(15);
	LTC_wakeup();


	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x23) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xC9) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;

	SpiaRegs.SPIBRR.all = 0x18; //LSPCLK/25

	DELAY_US(50);
	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	DELAY_US(50);
	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x22) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x42) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xC2) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;

	DELAY_US(100);
	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x21) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x54) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xA6) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x0F) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF9) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x7F) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x7F) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x8D) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xC4) << 8;

	DELAY_US(100);
	LTC_wakeup();
	DELAY_US(20);

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x23) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xC9) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(50);
	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;

	DELAY_US(50);
	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x22) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x42) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xC2) << 8;

	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x00) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPIBRR.all = 0x13; //LSPCLK/20

}
void LTC_spi_write_example()
{
	void LTC_wakeup(void);

	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x21) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x54) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xA6) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x8A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x98) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x8B) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xB8) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x8B) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x39) << 8;

	Uint16 PEC_calcumalatorz[6] = {0x8A, 0x98, 0x8B, 0xB8, 0x8B, 0x39};
	Uint16 PEC_A = 	(LTC_pec_calc(PEC_calcumalatorz, 6) & 0xFF00) >> 8;
	Uint16 PEC_B = 	LTC_pec_calc(PEC_calcumalatorz, 6) & 0x00FF;

	SpiaRegs.SPITXBUF = ((unsigned)PEC_A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)PEC_B) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(30);
	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x23) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xC9) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
/*
	DELAY_US(30);
	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x21) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x54) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xA6) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x8B) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xB8) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x8B) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x39) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x59) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x96) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF2) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(30);
	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x23) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xC9) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
*/
}
void LTC_spi_execute_example()
{
	void LTC_wakeup(void);

	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x21) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x54) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xA6) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x8A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF8) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x8B) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xB8) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x8B) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x39) << 8;

	Uint16 PEC_calcumalatorz[6] = {0x8A, 0xF8, 0x8B, 0xB8, 0x8B, 0x39};
	Uint16 PEC_A = 	(LTC_pec_calc(PEC_calcumalatorz, 6) & 0xFF00) >> 8;
	Uint16 PEC_B = 	LTC_pec_calc(PEC_calcumalatorz, 6) & 0x00FF;

	SpiaRegs.SPITXBUF = ((unsigned)PEC_A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)PEC_B) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(30);
	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x23) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xC9) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
}
void LTC_spi_terminate_charge_or_discharge()
{
	void LTC_wakeup(void);

	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x21) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x54) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xA6) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0x8A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x88) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x8B) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xB8) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x8B) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x39) << 8;

	Uint16 PEC_calcumalatorz[6] = {0x8A, 0x88, 0x8B, 0xB8, 0x8B, 0x39};
	Uint16 PEC_A = 	(LTC_pec_calc(PEC_calcumalatorz, 6) & 0xFF00) >> 8;
	Uint16 PEC_B = 	LTC_pec_calc(PEC_calcumalatorz, 6) & 0x00FF;

	SpiaRegs.SPITXBUF = ((unsigned)PEC_A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)PEC_B) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	DELAY_US(30);
	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x87) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x23) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xC9) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
}
void read_voltage_from_receive_buffer()
{
	void ready_rxbuf(void);

	//clear the receive buffer except for 8 bytes of information (the 6 voltage bytes + 2 PECs)
	ready_rxbuf();

	Uint16 rdata[8] = {0,0,0,0,0,0,0,0};
	//voltage reading buffers rdata
	rdata[0] = SpiaRegs.SPIRXBUF;
	rdata[1] = SpiaRegs.SPIRXBUF;
	rdata[2] = SpiaRegs.SPIRXBUF;
	rdata[3] = SpiaRegs.SPIRXBUF;
	rdata[4] = SpiaRegs.SPIRXBUF;
	rdata[5] = SpiaRegs.SPIRXBUF;
	rdata[6] = SpiaRegs.SPIRXBUF;
	rdata[7] = SpiaRegs.SPIRXBUF;
	//aggregates and parses the data accordingly with reference to the
	//DC2100A oscilloscope models
	Uint16 result1 = rdata[1] << 8 | rdata[0];
	Uint16 result2 = rdata[3] << 8 | rdata[2];
	Uint16 result3 = rdata[5] << 8 | rdata[4];
	Uint16 PEC = rdata[6] << 8 | rdata[7];
}
