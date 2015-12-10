/******************************************************************************************************************************
 *                                                                                                                            *
 *                                         COPYRIGHT (C) 2015    Tallinn University of Technology                             *
 *                                                                                                                            *
 ******************************************************************************************************************************
 *                                                                                                                            *
 *                                                                                                                            *
 *                                              CCCCC   FFFFFF    AAA                                                         *
 *                                             C        F        A   A                                                        *
 *                                             C        FFFF    A     A                                                       *
 *                                             C        F       AAAAAAA                                                       *
 *                                             C        F       A     A                                                       *
 *                                              CCCCC   F       A     A                                                       *
 *                                                                                                                            *
 *                                           Common firmware application SW                                                   *
 *                                                                                                                            *
 *                                                                                                                            *
 $                 $Workfile::   main.c                                                                                       $
 $                 $Revision::   1.0                                                                                          $
 $                     $Date::   13.10.2015                                                                                   $
 *                                                        								      								  *
 *                    Manager:   Yuanxu Xu                                                                                    *
 *                 Developers:   Martin Grosberg									      *
 *                  		 Aivar Koodi										      *
 *				 Marek Aare										      *
 *		                 Sander Sinijärv									      *
 *		                 Kätlin Lahtvee                                                                               *
 *                     Target:   Fishfeeder Project 2015 for sensor MSP430                                                                      *
 *                                                                                                                            *
 *****************************************************************************************************************************/

/***************************************************************************************************
 *	main.c  - Template for the MSP430 Launchpad project   								   		   *
 ***************************************************************************************************/

/***************************************************************************************************
 *	        Include section					                       		   					       *
 ***************************************************************************************************/
#include <msp430.h>
#include <math.h>
#include <stdio.h>

/***************************************************************************************************
 *	        Define section					                       		   					       *
 ***************************************************************************************************/
#define LED_0 BIT0
#define LED_1 BIT6
#define LED_OUT P1OUT
#define LED_DIR P1DIR
#define S2 BIT3

/***************************************************************************************************
 *	        Prototype section					                       							   *
 ***************************************************************************************************/
//sensor data
void send_systemdata(int data);
unsigned int measure_adc(void);
float calculate_lightlevel(unsigned int adc_value);
int calculate_waterlevel(unsigned int adc_value);
signed int ResistToTermo(int Resistance);
int calculate_temperature(unsigned int adc_value);
float temperature_state(float model_output);
float calculate_feedlevel(unsigned int adc_value);

//Motor for feeding
void enable_motor(int feed);

//Filter reminder
int filter_counter(int cleaning_time);


/***************************************************************************************************
 *	        Global Variable section  				                            				   *
 ***************************************************************************************************/
//Flags
int send_data = 0;
int feed_fishes = 0;
int clean_tank = 0;

//Counter
int counter = 0;
int motor_counter = 0;
int cleaning_counter = 0;

//Sensors
int model_output = 0;
unsigned int adc_value = 0;
int temp_state = 0;
//Motor
unsigned int crt_step = 0;
unsigned int step[8] = {0x20, 0x30, 0x10, 0x18, 0x08, 0x0C, 0x04, 0x24}; //step sequence for pins 2-5
int i = 0;
int power = 0;
int feed = 0;

//Cleaning reminder
int clean = 0;

/***************************************************************************************************
 *         Main section                                                                            *
 ***************************************************************************************************/
int main(void) 
{
//Stops Watchdog
WDTCTL = WDTPW | WDTHOLD;

//Calibrate timer
if(CALBC1_1MHZ==0xFF)
{
	while(1);
}
DCOCTL = 0;
BCSCTL1 = CALBC1_1MHZ;
DCOCTL = CALDCO_1MHZ;

//Timer 0
TA0CCTL0 = CCIE;
TA0CCR0 = 32768;
TA0CTL = TASSEL_1 + MC_1;

//Enable pushbutton S2
P1DIR &= ~S2; // pushbutton is configured as input
P1IE |= S2;   // port interrupt enabled
P1OUT |= S2;
P1IES &= ~S2;  // interrupt happens when button is released
P1REN |= S2;  // to avoid some weird toggling (and LED blinking) when touching the P1.3
P1IFG &= ~S2;

//Enable red and green LED
LED_DIR = LED_1; //LEDs are set as output
LED_OUT &= ~(LED_1); //LEDs are turned off

//Enable ports for UART connection
P1SEL  |= BIT1 + BIT2;	 	//Enable module function on pin
P1SEL2 |= BIT1 + BIT2;  	//Select second module; P1.1 = RXD, P1.2=TXD

//Enable clock
UCA0CTL1 |= UCSSEL_2;    	// Set clock source SMCLK
UCA0BR0 = 104;           	// 1MHz 9600 (N=BRCLK/baudrate)
UCA0BR1 = 0;             	// 1MHz 9600
UCA0MCTL = UCBRS0;       	// Modulation UCBRSx = 1
UCA0CTL1 &= ~UCSWRST;    	// **Initialize USCI state machine**

//Enable motor
P2DIR = 0x3C; // Allow P2 pins 2 to 5 to output for motor and
P2OUT = 0x3C; // Disable everything.

IE2 |= UCA0RXIE;         	//enable RX interrupt

__enable_interrupt();

	while(1)
	{
		__bis_SR_register(GIE);

		if(clean_tank == 1)
		{
			//Filter reminders
			P1OUT |= LED_1;			//Kuidas initializeda ja kinni panna led ilma toggleita
			clean = 1;
			clean_tank = 0;
		}

		if(feed_fishes)
		{
			//Motor operations
			enable_motor(feed);
			feed_fishes = 0;
		}

		if(send_data)
		{
			//start bit (255)
			model_output = 0xFF;
			send_systemdata((int) (model_output));

			//light (0/1)
			ADC10CTL0 = ADC10SHT_3 + ADC10ON; //64 clocks for sample
			ADC10CTL1 = INCH_0 + ADC10DIV_3;	// Sensor is at channel X and CLK/4
			__delay_cycles(3000);	// settle time 30ms
			adc_value = measure_adc();
			model_output = calculate_lightlevel(adc_value);
			send_systemdata((int) (model_output));

			//water (0/1)
			ADC10CTL0 = ADC10SHT_3 + ADC10ON; //64 clocks for sample
			ADC10CTL1 = INCH_7 + ADC10DIV_3;	// Sensor is at channel X and CLK/4
			__delay_cycles(3000);	// settle time 30ms
			adc_value = measure_adc();
			model_output = calculate_waterlevel(adc_value);
			send_systemdata((int) (model_output));

			//feed (0/1)
			ADC10CTL0 = ADC10SHT_3 + ADC10ON; //64 clocks for sample
			ADC10CTL1 = INCH_5 + ADC10DIV_3;	// Sensor is at channel X and CLK/4
			__delay_cycles(3000);	// settle time 30ms
			adc_value = measure_adc();
			model_output = calculate_feedlevel(adc_value);
			send_systemdata((int) (model_output));

			//temperature value (0..40)
			ADC10CTL0 = ADC10SHT_3 + ADC10ON; //64 clocks for sample
			ADC10CTL1 = INCH_4 + ADC10DIV_3;	// Temp Sensor is at channel X and CLK/4
			__delay_cycles(3000);	// settle time 30ms
			adc_value = measure_adc();
			__delay_cycles(3000);	// settle time 30ms
			model_output = calculate_temperature(adc_value);
			send_systemdata((int) (model_output));

			//temperature state (0/1/2)
			temp_state = temperature_state(model_output);
			send_systemdata((int) (temp_state));

			//Filter cleaning (0/1)
			send_systemdata((int) (clean));

			//exit send_data
			send_data = 0;
		}
	}
}
/* END: main */

unsigned int measure_adc(void)
{
	unsigned int raw_value = 0;
	ADC10CTL0 |= ENC + ADC10SC;	// Sampling and conversion start
	while(ADC10CTL1 & BUSY); // Wait..i am converting...
	raw_value = ADC10MEM;	// Read ADC memory
	return raw_value;
}

float calculate_lightlevel(unsigned int adc_value)
{
	adc_value = adc_value/3;
	if(adc_value < 130)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int calculate_waterlevel(unsigned int adc_value)
{
	if (adc_value == 0)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

signed int ResistToTermo(int Resistance)
{
	signed int termo=0;

	if(Resistance>31308)
	{
		termo=0;
	}
	else if ((Resistance<=31308)&&(Resistance>29749))
	{
		termo=1;
	}
	else if ((Resistance<=29749)&&(Resistance>28276))
	{
		termo=2;
	}
	else if ((Resistance<=28276)&&(Resistance>26885))
	{
		termo=3;
	}
	else if ((Resistance<=26885)&&(Resistance>25570))
	{
		termo=4;
	}
	else if ((Resistance<=25570)&&(Resistance>24327))
	{
		termo=5;
	}
	else if ((Resistance<=24327)&&(Resistance>23152))
	{
		termo=6;
	}
	else if ((Resistance<=23152)&&(Resistance>22141))
	{
		termo=7;
	}
	else if ((Resistance<=22141)&&(Resistance>20989))
	{
		termo=8;
	}
	else if ((Resistance<=20989)&&(Resistance>19993))
	{
		termo=9;
	}
	else if ((Resistance<=19993)&&(Resistance>19051))
	{
		termo=10;
	}
	else if ((Resistance<=19051)&&(Resistance>18158))
	{
		termo=11;
	}
	else if ((Resistance<=18158)&&(Resistance>17312))
	{ //20-25C
		termo=12;
	}
	else if ((Resistance<=17312)&&(Resistance>16511))
	{ //25-30C
		termo=13;
	}
	else if ((Resistance<=16511)&&(Resistance>15751))
	{ //30-40C
		termo=14;
	}
	else if ((Resistance<=15751)&&(Resistance>15031))
	{ //40-50C
		termo=15;
	}
	else if ((Resistance<=15031)&&(Resistance>14347))
	{ //50-60C
		termo=16;
	}
	else if ((Resistance<=14347)&&(Resistance>13698))
	{ //60-70C
		termo=17;
	}
	else if ((Resistance<=13698)&&(Resistance>13083))
	{ //70-80C
		termo=18;
	}
	else if ((Resistance<=13083)&&(Resistance>12499))
	{ //80-90C
		termo=19;
	}
	else if ((Resistance<=12499)&&(Resistance>11944))
	{ //80-90C
		termo=20; //I don't care if it's above 91 either
	}
	else if ((Resistance<=11944)&&(Resistance>11417))
	{
		termo=21;
	}
	else if ((Resistance<=11417)&&(Resistance>10916))
	{ //20-25C
		termo=22;
	}
	else if ((Resistance<=10916)&&(Resistance>10440))
	{ //25-30C
		termo=23;
	}
	else if ((Resistance<=10440)&&(Resistance>10000))
	{ //30-40C
		termo=24;
	}
	else if ((Resistance<=10000)&&(Resistance>9556))
	{ //40-50C
		termo=25;
	}
	else if ((Resistance<=9556)&&(Resistance>9147))
	{ //50-60C
		termo=26;
	}
	else if ((Resistance<=9147)&&(Resistance>8387))
	{ //60-70C
		termo=27;
	}
	else if ((Resistance<=8758)&&(Resistance>8387))
	{ //70-80C
		termo=28;
	}
	else if ((Resistance<=8387)&&(Resistance>8034))
	{ //80-90C
		termo=29;
	}
	else if ((Resistance<=8034)&&(Resistance>7700))
	{ //80-90C
		termo=30; //I don't care if it's above 91 either
	}
	else if ((Resistance<=7700)&&(Resistance>7377))
	{
		termo=31;
	}
	else if ((Resistance<=7377)&&(Resistance>7072))
	{ //20-25C
		termo=32;
	}
	else if ((Resistance<=7072)&&(Resistance>6781))
	{ //25-30C
		termo=33;
	}
	else if ((Resistance<=6781)&&(Resistance>6504))
	{ //30-40C
		termo=34;
	}
	else if ((Resistance<=6504)&&(Resistance>6239))
	{ //40-50C
		termo=35;
	}
	else if ((Resistance<=6239)&&(Resistance>5987))
	{ //50-60C
		termo=36;
	}
	else if ((Resistance<=5987)&&(Resistance>5746))
	{ //60-70C
		termo=37;
	}
	else if ((Resistance<=5746)&&(Resistance>5517))
	{ //70-80C
		termo=38;
	}
	else if ((Resistance<=5517)&&(Resistance>5298))
	{ //80-90C
		termo=39;
	}
	else if (Resistance<5298)
	{ //80-90C
		termo=40; //I don't care if it's above 91 either
	}
	 return termo;
}

int calculate_temperature(unsigned int adc_value)
{
	float Voltage = 0;
	float Resistance = 0;
	int Temperature = 0;

	Voltage = ((1023-adc_value)*3.6)/1023; // Convert 256-bit value to voltage
	Resistance = ((float)(10000*3.6/Voltage)-10000); // Convert output voltage to resistance
	Temperature = ResistToTermo(Resistance);
	return Temperature;
}

float temperature_state(float model_output)
{
	if (model_output < 22)
	{
		return 0;
	}
	else if (model_output > 26)
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

float calculate_feedlevel(unsigned int adc_value)
{
	if(adc_value > 20)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void enable_motor(int feed)
{
	for(i=0;i<8000;i++)
	{    //4000 cycles ~ full turn, 2000 cycles ~ half turn
			P2OUT = step[crt_step++];
			if(crt_step == 8) crt_step = 0;
			__delay_cycles(1500);//99Hz for 28BYJ-48
	}
}

void send_systemdata(int data)
{
	while (!(IFG2&UCA0TXIFG));  // Wait until TX buffer is ready
	UCA0TXBUF = data;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	counter = counter + 1;
//	send_systemdata((int) (counter));
	if((counter)%2 == 0)
	{
		motor_counter++;
//		send_systemdata((int) (motor_counter));
		send_data = 1;
		if((motor_counter)%2 == 0)
		{
			cleaning_counter++;
//			send_systemdata((int) (cleaning_counter));
			feed_fishes = 1;
			if((cleaning_counter%2) == 0)
			{
				clean_tank = 1;
				counter = 0;
				motor_counter = 0;
				cleaning_counter = 0;
			}
		}
	}
}

#pragma vector=PORT1_VECTOR    //interrupt service for port 1
__interrupt void Port_1(void)
{
	P1OUT &= ~LED_1;
	clean = 0;
	cleaning_counter = 0;
	P1IFG &= ~S2; //clears the interrupt
}



/***************************************************************************************************
 *         History of changes                                                                       *
 ***************************************************************************************************/

//[Week 8] Added empty function slots for future code.
//[Week 11] Added function init_pins whisch is inizilizing the water level sensor pin as input
//[Week 12] Added waterlevel function which is returning eather 1 or 0
//[Week 12] Added Uart inizilation function which will start communication bridge between uart
//[Week 13] Added Calibration function for clock -- main function is still empty because we have no idea how to arrange yet everything.
//[Week 14] Added all sensor and data sending over UART functionalities 
//[Week 15] Added motor functionalities
