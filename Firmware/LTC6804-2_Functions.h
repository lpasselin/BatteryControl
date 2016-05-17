/*
 * LTC6804-2_Functions.h
 *
 *  Created on: Jun 19, 2015
 *      Author: Andrew Hoang
 */

/*
 * The LTC6804-2 provides a variety of functions.
 *  They are listed as follows (and to be implemented utilizing TI's Code Composer Studio IDE):
 *  - Voltage Readings for modules 1-12
 *  - Temperature
 *  - Discharge modules 1-12 passively or actively
 *  - ADC conversions
 *  - ADC Clear (AUX, voltage, flags)
 *  - Overvoltage and undervoltage flags
 *  - Analog Power Supply Voltage
 *  - RDCFG
 *  - WRCFG --> allows passive discharge of battery modules through this function usage
 *  LTC3300 write/execute balance --> allows active discharge/charge of battery modules for cells 1-6 with this function usage
 *  Note, other functions need to be setup and configured properly for battery discharge (as well as other functions)
 *  in order forthe WRCFG to work --> See LTC6804-2 Datasheet page 47
 */

#ifndef LTC6804_2_FUNCTIONS_H_
#define LTC6804_2_FUNCTIONS_H_

extern Uint16 cell_voltage[12];
extern Uint16 cell_flags[12];

//this wakes up the LTC6804-2 chip. Note however, that we are currently using a GPIO pin as the SPI CS
//This will need to be later configured to cater to the SPISTEA pin for optimal usage...
void LTC_wakeup(void);

//This turns on the ADC and waits for a specific duration before ADC conversion operations
void LTC_refon_set(void);

//This clears the ADC voltage readings from registers A, B, C, D
void LTC_ADC_clear(void);

//This clears the ADSTAT registers. This involves the flag registers of OVUV
void LTC_ADSTAT_clear(void);

//This function initializes the LTC ADC conversion and waits a delay time of ~6 ms
//in order for ADC conversions to complete
void LTC_ADC_conversion(void);

//This function initializes the LTC ADSTAT conversion and waits a delay time of 1.9ms
//before the next function is executed
void LTC_ADSTAT_conversion(void);

//These functions pulls data directly from the ADC voltage reading registers and gives out the corresponding values
//in a 5 decimal format i.e 36746 which corresponds to 3.6746 V
void LTC_read_voltages_123(void);
void LTC_read_voltages_456(void);
void LTC_read_voltages_789(void);
void LTC_read_voltages_10_11_12(void);

//All this LTC function does is change the parity bit from a 1 to a 0 to end all charge/discharge execute commands.
void LTC_spi_terminate_charge_or_discharge(void);

//ready the rx buffer by deleting all prior bytes
//to only leave a remainder of 8 bytes for reading
void ready_rxbuf(void);

//read voltages by parsing the receive buffer properly into specific settings
void read_voltage_from_receive_buffer(void);

//Write and execute command functions
void LTC3300_write_balance(Uint16 cell1, Uint16 cell2, Uint16 cell3, Uint16 cell4, Uint16 cell5, Uint16 cell6);
void LTC3300_execute_balance(Uint16 cell1, Uint16 cell2, Uint16 cell3, Uint16 cell4, Uint16 cell5, Uint16 cell6);

//UVOV flags
void LTC_UVOV_get_flags(void);
void LTC_read_UVOV_flags(void);

//Functions to be local to LTC6804-2.c Code only
//--------------------------------------------------------------------------------------------------------------------------------
//These functions were directly pulled from Bobby Backofen's code source from the LTC6804-2 PEC
//calculations and incorporated into TI's Code Composer Studio for compiliation of PEC values
Uint16 LTC_pec_calc(Uint16 *data, Uint16 length);
Uint16 LTC6804_pec_lookup(data, remainder);
Uint16 ltc3300_crc_calc(Uint16 nibble1, Uint16 nibble2, Uint16 nibble3);

#endif /* LTC6804_2_FUNCTIONS_H_ */
