/*
 * In order to utilize SPISTEA, TXDLY must be set to 0 (SpiaRegs.SPIFFCT.bit.TXDLY = 0x0;)
 * However, if a general GPIO is used for the SPI chip select (CS) then SpiaRegs.SPIFFCT.bit.TXDLY can be set to any number (CS always being held low whenever utilized)
 *
 */

/*
 * PIN CONFIGURATION SETUP
 * PIN 1 - 6 left to right on the DC2100A board are as follows:
 * 1C - 3.3V Enable, 2C SPIMOSI, 3C SPIMISO, 4C SPICLK, 5C SPICS, 6C Ground
 * Pin Match up with DC2100A board from TMS320F28377D
 * PIN 67 - SPIMOSI, PIN 69 SPI MISO, PIN 71 SPICLK, PIN 89 SPICS (GPIO)
 * Therefore:
 * 2C with pin 67, 3C with pin 69, 4C with pin 71, and 5C with pin 89
*/

#include <stdio.h>
#include <math.h>

#include "F28x_Project.h"
#include "LTC6804-2_Functions.h"

//delete this if using RAM
//allows the use of printf for Uint16 types
#include <inttypes.h>
//--------------------------------

const Uint16 V_2_Soc_LUT[] = {
56,25210,
89,25330,
121,26220,
131,26430,
154,26560,
156,26720,
161,26890,
186,27230,
196,27570,
219,27910,
251,28120,
261,28410,
284,28750,
317,29090,
349,29340,
382,29680,
447,30010,
479,30350,
512,30600,
577,30900,
610,31150,
707,31450,
740,31660,
805,31870,
870,32040,
935,32290,
1033,32500,
1131,32670,
1196,32840,
1294,33010,
1392,33130,
1457,33260,
1554,33390,
1620,33470,
1685,33550,
1782,33640,
1848,33760,
1945,33850,
2010,33930,
2108,34020,
2173,34100,
2238,34190,
2336,34230,
2434,34310,
2499,34400,
2597,34440,
2629,34480,
2727,34560,
2760,34610,
2857,34690,
2955,34730,
3053,34810,
3150,34900,
3248,34940,
3313,34980,
3346,35020,
3444,35070,
3509,35110,
3574,35150,
3672,35190,
3704,35230,
3769,35280,
3900,35360,
3965,35400,
4030,35440,
4095,35490,
4193,35530,
4258,35570,
4323,35610,
4388,35650,
4453,35700,
4551,35780,
4649,35820,
4714,35910,
4812,35950,
4877,35990,
4975,36070,
5072,36120,
5137,36200,
5235,36240,
5300,36330,
5398,36410,
5463,36450,
5561,36540,
5659,36620,
5724,36700,
5821,36790,
5854,36830,
5952,36870,
6017,36960,
6049,37000,
6147,37080,
6212,37170,
6277,37210,
6343,37290,
6440,37380,
6538,37420,
6603,37500,
6668,37590,
6734,37630,
6815,37710,
6883,37750,
6965,37840,
7023,37880,
7118,37960,
7199,38050,
7294,38130,
7365,38170,
7434,38260,
7506,38340,
7597,38430,
7681,38470,
7773,38550,
7867,38680,
7962,38760,
8056,38850,
8137,38930,
8229,39060,
8313,39140,
8395,39270,
8463,39350,
8558,39440,
8616,39520,
8698,39600,
8792,39730,
8861,39810,
8955,39940,
9027,40020,
9085,40070,
9177,40190,
9235,40280,
9330,40360,
9424,40450,
9519,40530,
9600,40610,
9636,40660,
9694,40700,
9752,40780,
9810,40820,
9904,40990,
9997,41160,
10044,41290
};

int cell_OOO[4] = {0,0,0,0};

void output_high()
{
	//change GPIO when hooking up full MOLEX connection (GPIO19)
	GpioDataRegs.GPBDAT.bit.GPIO40 = 1;
}
void output_low()
{
	//change GPIO when hooking up full MOLEX connection (GPIO19)
	GpioDataRegs.GPBDAT.bit.GPIO40 = 0;
}
void spi_init()
{
	SpiaRegs.SPICCR.bit.SPISWRESET = 0;
	SpiaRegs.SPICCR.bit.CLKPOLARITY = 1; //important that CPHA = 0 and CPOL = 1 reference to LTC6804-2 timing datasheet
	SpiaRegs.SPICCR.bit.SPILBK = 0;
	SpiaRegs.SPICCR.bit.SPICHAR = 0x7; //0x7; //8bit/char for 2 hex values
	//use of 16 bit seems like the only way SPI bus is able to transfer.. need to hear back from bobby
	//SpiaRegs.SPICCR.bit.SPICHAR = 0xF;
	SpiaRegs.SPICCR.bit.HS_MODE = 0;

	SpiaRegs.SPICTL.bit.OVERRUNINTENA = 0;
	SpiaRegs.SPICTL.bit.CLK_PHASE = 0; //important that CPHA = 0 and CPOL = 1 reference to LTC6804-2 timing datasheet
	SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1; //master = 1, slave = 0
	SpiaRegs.SPICTL.bit.TALK = 1;
	SpiaRegs.SPICTL.bit.SPIINTENA = 1; //enable/disable interrupts

	//SpiaRegs.SPISTS --> not used..yet

	//SpiaRegs.SPIBRR.all = 0x31; //LSPCLK/50
	//SpiaRegs.SPIBRR.all = 0x63; //LSPCLK/100
	//SpiaRegs.SPIBRR.all = 0x3E; //LSPCLK/63
	SpiaRegs.SPIBRR.all = 0x13; //LSPCLK/20

	SpiaRegs.SPIPRI.bit.FREE = 1;
	SpiaRegs.SPICCR.bit.SPISWRESET = 1;
}
void spi_fifo()
{
	SpiaRegs.SPIFFTX.bit.SPIRST = 1;
	SpiaRegs.SPIFFTX.bit.TXFIFO = 1;
	SpiaRegs.SPIFFTX.bit.SPIFFENA = 1;
	//SpiaRegs.SPIFFTX.bit.TXFFST
	SpiaRegs.SPIFFTX.bit.TXFFINTCLR = 0;
	SpiaRegs.SPIFFTX.bit.TXFFIENA = 1; //when ISR is used, enable this!
	SpiaRegs.SPIFFTX.bit.TXFFIL = 0x6;

	//SpiaRegs.SPIFFRX.bit.RXFFOVF
	SpiaRegs.SPIFFRX.bit.RXFFOVFCLR = 1;
	SpiaRegs.SPIFFRX.bit.RXFIFORESET = 1;
	//SpiaRegs.SPIFFRX.bit.RXFFST //read status
	SpiaRegs.SPIFFRX.bit.RXFFINT = 1;
	SpiaRegs.SPIFFRX.bit.RXFFINTCLR = 1; //initially clear receive flag
	SpiaRegs.SPIFFRX.bit.RXFFIENA = 1; //when ISR is used, enable this!
	//it is an interrupt flag enabler
	//SpiaRegs.SPIFFRX.bit.RXFFIL = 0x1;

	//TXDLY must be set to 0x0 when using SPISTE as the chip select
	SpiaRegs.SPIFFCT.bit.TXDLY = 0x1;
}

//prototype functions in progress

__interrupt void cpu_timer0_isr(void)
{
	CpuTimer0.InterruptCount++;
	LTC3300_write_balance(0,0,cell_OOO[0],cell_OOO[1],cell_OOO[2],cell_OOO[3]);
	LTC3300_execute_balance(0,0,cell_OOO[0],cell_OOO[1],cell_OOO[2],cell_OOO[3]);

	//CpuTimer0.InterruptCount = 0;

	// Acknowledge this interrupt to receive more interrupts from group 1
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
__interrupt void cpu_timer1_isr(void)
{
   LTC3300_write_balance(0,0,0,0,0,0);
   LTC3300_execute_balance(0,0,0,0,0,0);
   CpuTimer1.InterruptCount++;
   // The CPU acknowledges the interrupt.

    cell_OOO[0] = 0;
    cell_OOO[1] = 0;
    cell_OOO[2] = 0;
    cell_OOO[3] = 0;

	LTC_refon_set();
	LTC_ADC_clear();
	LTC_ADC_conversion();

	LTC_read_voltages_123();
	read_voltage_from_receive_buffer();
	LTC_read_voltages_456();
	read_voltage_from_receive_buffer();
	LTC_read_voltages_789();
	read_voltage_from_receive_buffer();
	LTC_read_voltages_10_11_12();

	Uint16 cell3_soc = soc_lookup(cell_voltage[2]);
	Uint16 cell4_soc = soc_lookup(cell_voltage[3]);
	Uint16 cell5_soc = soc_lookup(cell_voltage[4]);
	Uint16 cell6_soc = soc_lookup(cell_voltage[5]);

	Uint16 minVal = 60000;
	Uint16 i = 1;
	for (i = 1; i < 5; i++)
	{
		if (cell3_soc < minVal)
		{
			minVal = cell3_soc;
		}
		else if (cell4_soc < minVal)
		{
			minVal = cell4_soc;
		}
		else if (cell5_soc < minVal)
		{
			minVal = cell5_soc;
		}
		else if (cell6_soc < minVal)
		{
			minVal = cell6_soc;
		}
	}

	if (cell3_soc > minVal)
	{
		cell_OOO[0] = -1;
	}

	if (cell4_soc > minVal)
	{
		cell_OOO[1] = -1;
	}

	if (cell5_soc > minVal)
	{
		cell_OOO[2] = -1;
	}

	if (cell6_soc > minVal)
	{
		cell_OOO[3] = -1;
	}


//	printf("%" PRIu16 "\n", cell_voltage[0]);
//	printf("%" PRIu16 "\n", cell_voltage[1]);
//	printf("%" PRIu16 "\n", cell_voltage[2]);
//	printf("%" PRIu16 "\n", cell_voltage[3]);
//	printf("%" PRIu16 "\n", cell_voltage[4]);
//	printf("%" PRIu16 "\n", cell_voltage[5]);
//	printf("%" PRIu16 "\n", cell_voltage[6]);
//	printf("%" PRIu16 "\n", cell_voltage[7]);
//	printf("%" PRIu16 "\n", cell_voltage[8]);
//	printf("%" PRIu16 "\n", cell_voltage[9]);
//	printf("%" PRIu16 "\n", cell_voltage[10]);
//	printf("%" PRIu16 "\n", cell_voltage[11]);

//	printf("%" PRIu16 "\n", cell3_soc);
//	printf("%" PRIu16 "\n", cell4_soc);
//	printf("%" PRIu16 "\n", cell5_soc);
//	printf("%" PRIu16 "\n", cell5_soc);
//	printf("%" PRIu16 "\n", minVal);

//	printf("%d \n", cell_OOO[0]);
//	printf("%d \n", cell_OOO[1]);
//	printf("%d \n", cell_OOO[2]);
//	printf("%d \n", cell_OOO[3]);

	//OV will output value of 2
	//UV will out a value of 1
	//if you get 3... UV and OV
	printf("%" PRIu16 "\n", cell_flags[0]);
	printf("%" PRIu16 "\n", cell_flags[1]);
	printf("%" PRIu16 "\n", cell_flags[2]);
	printf("%" PRIu16 "\n", cell_flags[3]);
	printf("%" PRIu16 "\n", cell_flags[4]);
	printf("%" PRIu16 "\n", cell_flags[5]);
	printf("%" PRIu16 "\n", cell_flags[6]);
	printf("%" PRIu16 "\n", cell_flags[7]);
	printf("%" PRIu16 "\n", cell_flags[8]);
	printf("%" PRIu16 "\n", cell_flags[9]);
	printf("%" PRIu16 "\n", cell_flags[10]);
	printf("%" PRIu16 "\n", cell_flags[11]);

	PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;
}
__interrupt void cpu_timer2_isr(void)
{
   CpuTimer2.InterruptCount++;
   // The CPU acknowledges the interrupt.

}

//LTC passive discharge option found readily available on the LTC6804-2 board.
void LTC_discharge_ALL_CELLS()
{
	//The discharge duration only lasts for a couple of seconds on the LED. Need confirmation.
	void LTC_wakeup(void);

	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x80) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x01) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x4D) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x7A) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0xE1) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xE2) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x44) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x9C) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	//SpiaRegs.SPITXBUF = ((unsigned)0x1F) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFF) << 8;
	//calculated PECs
	//SpiaRegs.SPITXBUF = ((unsigned)0x66) << 8;
	//SpiaRegs.SPITXBUF = ((unsigned)0x0A) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x73) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xFA) << 8;

	DELAY_US(100);
}
void LTC_discharge_cells_3_4()
{
	//The discharge duration only lasts for a couple of seconds on the LED. Need confirmation.
	void LTC_wakeup(void);

	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x80) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x01) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x4D) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x7A) << 8;

	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait

	SpiaRegs.SPITXBUF = ((unsigned)0xE1) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xE2) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x44) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x9C) << 8;
	while(SpiaRegs.SPIFFTX.bit.TXFFST != 0) {}	//wait
	SpiaRegs.SPITXBUF = ((unsigned)0x0C) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xF0) << 8;
	//calculated PECs
	SpiaRegs.SPITXBUF = ((unsigned)0xD2) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0xA4) << 8;

	DELAY_US(100);
}
void LTC_read_cfg()
{
	void LTC_wakeup(void);

	LTC_wakeup();

	SpiaRegs.SPITXBUF = ((unsigned)0x80) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x02) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x5B) << 8;
	SpiaRegs.SPITXBUF = ((unsigned)0x1E) << 8;

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
	DELAY_US(100);
}
void receive_all_data()
{
	Uint16 data[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	ready_rxbuf();
	data[0] = SpiaRegs.SPIRXBUF;
	data[1] = SpiaRegs.SPIRXBUF;
	data[2] = SpiaRegs.SPIRXBUF;
	data[3] = SpiaRegs.SPIRXBUF;
	data[4] = SpiaRegs.SPIRXBUF;
	data[5] = SpiaRegs.SPIRXBUF;
	data[6] = SpiaRegs.SPIRXBUF;
	data[7] = SpiaRegs.SPIRXBUF;
	data[8] = SpiaRegs.SPIRXBUF;
	data[9] = SpiaRegs.SPIRXBUF;
	data[10] = SpiaRegs.SPIRXBUF;
	data[11] = SpiaRegs.SPIRXBUF;
	data[12] = SpiaRegs.SPIRXBUF;
	data[13] = SpiaRegs.SPIRXBUF;
	data[14] = SpiaRegs.SPIRXBUF;
	data[15] = SpiaRegs.SPIRXBUF;
}

Uint16 soc_lookup(Uint16 voltage_value)
{
	Uint16 i = 0;
	Uint16 idx = 0;
	Uint16 soc_value = 0;
	double run = 0;
	double rise = 0;
	double slope = 0;
	double b = 0;


	for(i = 1; i < 284; i = i+2)
	{
		if(voltage_value < V_2_Soc_LUT[1])
		{
			soc_value = V_2_Soc_LUT[1];
			break;
		}

		if(voltage_value > V_2_Soc_LUT[283])
		{
			soc_value = V_2_Soc_LUT[283];
			break;
		}

		if(voltage_value == V_2_Soc_LUT[i])
		{
			soc_value = V_2_Soc_LUT[i - 1];
			break;
		}

		if(voltage_value < V_2_Soc_LUT[i])
		{
			run = V_2_Soc_LUT[i] - V_2_Soc_LUT[i - 2];
			rise = V_2_Soc_LUT[i-1] - V_2_Soc_LUT[i - 3];
		    slope = rise/run;

			b = V_2_Soc_LUT[i - 3] - slope*V_2_Soc_LUT[i - 2];

			soc_value = floor(slope*voltage_value + b);
			break;
		}

	}

   return soc_value;
}

void main(void)
{
	//Enable peripheral clocks and other things
	//F2837xD_SysCtrl.c
	InitSysCtrl();

    EALLOW;
    GpioCtrlRegs.GPAMUX1.all = 0x00000000;  // All GPIO
    GpioCtrlRegs.GPAMUX2.all = 0x00000000;  // All GPIO
    GpioCtrlRegs.GPBMUX1.all = 0x00000000;  // All GPIO

    GpioCtrlRegs.GPADIR.all = 0xFFFFFFFF;   // All outputs
    GpioCtrlRegs.GPBDIR.all = 0x00001FFF;   // All outputs
    EDIS;

	//InitGpio();
	//initalize GPIO
	InitSpiaGpio();

	DINT;
	InitPieCtrl();

	// Disable CPU __interrupts and clear all CPU __interrupt flags:
	IER = 0x0000;
	IFR = 0x0000;

	InitPieVectTable();

	EALLOW; //This is needed to write to EALLOW protected registers
	ClkCfgRegs.LOSPCP.bit.LSPCLKDIV = 0x5; //LSPCLK (200 Mhz)/10


	//-----------------------------------------------------------------------------------
	//Instantiate CPU timer control
	//CPU timer interrupt services in prototype phase
	EALLOW;
	PieVectTable.TIMER0_INT = &cpu_timer0_isr;
	PieVectTable.TIMER1_INT = &cpu_timer1_isr;
	PieVectTable.TIMER2_INT = &cpu_timer2_isr;
	EDIS; //This is needed to disable write to EALLOW protected registers

	InitCpuTimers();

    // Configure CPU-Timer 0, 1, and 2 to interrupt every second:
    // 200MHz CPU Freq, 1 second Period (in uSeconds)
	ConfigCpuTimer(&CpuTimer0, 200, 100000);
    ConfigCpuTimer(&CpuTimer1, 200, 10000000);
    ConfigCpuTimer(&CpuTimer2, 200, 1000000);

    // To ensure precise timing, use write-only instructions to write to the entire register. Therefore, if any
    // of the configuration bits are changed in ConfigCpuTimer and InitCpuTimers (in F2837xD_cputimervars.h), the
    // below settings must also be updated.
	CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
    CpuTimer1Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
    CpuTimer2Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

    // Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
    // which is connected to CPU-Timer 1, and CPU int 14, which is connected
    // to CPU-Timer 2:
	IER |= M_INT1;
    IER |= M_INT13;
    IER |= M_INT14;

    // Enable TINT0 in the PIE: Group 1 interrupt 7
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    // Enable global Interrupts and higher priority real-time debug events:
    EINT; // Enable Global interrupt INTM
    ERTM; // Enable Global realtime interrupt DBGM
	//Enable interrupts

    //End CPU timer configuration
	//-----------------------------------------------------------------------------------

    spi_init();
	spi_fifo();
	output_high();
	DELAY_US(1000000);
	LTC_wakeup();

	/*
	//LTC ADCV read voltage functions here ---
	//Note: Additional ADC readings may be possible i.e Sum of Measurement cells,
	LTC_refon_set();
	LTC_ADC_clear();
	//LTC_ADC_conversion();
	//----------------------------------------

	//need to somehow clear receive buffer
	SpiaRegs.SPIFFRX.bit.RXFFOVFCLR = 1;

	//LTC_read_voltages_123();
	//LTC_read_voltages_456();
	//LTC_read_voltages_789();
	//LTC_read_voltages_10_11_12();

	//LTC_UVOV_get_flags();

	//ready_rxbuf();
	//read_voltage_from_receive_buffer();
	//receive_all_data();
	//LTC_read_UVOV_flags();
	//read_voltage_from_receive_buffer();

	//Uint16 checker[6] = {rdata[0], rdata[1], rdata[2], rdata[3], rdata[4], rdata[5]};
	//Uint16 PEC_check = LTC_pec_calc(checker, 6);
	*/
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	for(;;) //infinite loop
	{
		//incorporate a regular LTC_wakeup procedure (< 2.0 seconds)
		//incorporate a regular LTC pulse signal (< 1.5 seconds)
		//It is important to regularly pulse wake up and pulse signals to the
		//LTC6804-2 and LTC3300 boards so that the Watchdog Timer does not expire
		//LTC6804-2 ADC reference ON
		//LTC3300 72 pulse cycle of an execute command repeatedly operating
	}
}
















/*//method saved as: read_voltage_from_receive_buffer();
//for this chunk of code.
//Will need to incorporate a return feature
//that gives back the respective voltage readings.
Uint16 rdata[8] = {0,0,0,0,0,0,0,0};
rdata[0] = SpiaRegs.SPIRXBUF;
rdata[1] = SpiaRegs.SPIRXBUF;
rdata[2] = SpiaRegs.SPIRXBUF;
rdata[3] = SpiaRegs.SPIRXBUF;
rdata[4] = SpiaRegs.SPIRXBUF;
rdata[5] = SpiaRegs.SPIRXBUF;
rdata[6] = SpiaRegs.SPIRXBUF;
rdata[7] = SpiaRegs.SPIRXBUF;
//voltage reading buffers rdata
Uint16 result1 = rdata[1] << 8 | rdata[0];
Uint16 result2 = rdata[3] << 8 | rdata[2];
Uint16 result3 = rdata[5] << 8 | rdata[4];
Uint16 PEC = rdata[6] << 8 | rdata[7];
*/


//use this if reverting back to SPISTEA as chip select as opposed to GPIO
//SpiaRegs.SPITXBUF = ((unsigned)0x00)<<8;


/* Example of PEC calculation use in here
Uint16 lel[2] = {0x87, 0x23};
Uint16 PEC1 = LTC_pec_calc(lel, 2);
Uint16 PEC2 = LTC_pec_calc(0x04, 1);
*/

//matlab fliplr --> left to right justified

//not used anymore --> replaced by DELAY_US(#) function
/*
void delay_loop()
{
    short      i;
    for (i = 0; i < 1000; i++) {}
}
 */


/*
	Uint16 result1 = rdata[0] << 8 | rdata[1];
	Uint16 result2 = rdata[2] << 8 | rdata[3];
	Uint16 result3 = rdata[4] << 8 | rdata[5];
	Uint16 PEC = rdata[6] << 8 | rdata[7];
*/
/*

	//confirmation of PEC values based on PEC lookup table
	//--> Bobby Backofen's PEC calculation
	Uint16 fill[6] = {rdata[0], rdata[1], rdata[2], rdata[3], rdata[4], rdata[5]};
	Uint16 PEC_confirmation = LTC_pec_calc(fill, 6);

	//Uint16 fill2[6] = {0xE1, 0xE2, 0x44, 0x9C, 0x00, 0x00};
	Uint16 fill2[6] = {0x8A, 0xF8, 0x8B, 0xB8, 0x8B, 0x38};
	Uint16 PEC_value = LTC_pec_calc(fill2, 6);

	Uint16 LTC3300_PEC_value = ltc3300_crc_calc(0b0000, 0b0001, 0b0000);


	Uint16 PEC_calcumalatorz[6] = {0x8A, 0xF8, 0x8B, 0xB8, 0x8B, 0x38};
	Uint16 PEC_A_6804 = (LTC_pec_calc(PEC_calcumalatorz, 6) & 0xFF00) >> 8;
	Uint16 PEC_B_6804 = LTC_pec_calc(PEC_calcumalatorz, 6) & 0x00FF;

*/
