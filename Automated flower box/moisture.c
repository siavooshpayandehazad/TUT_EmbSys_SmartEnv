#include <msp430g2553.h>
#define GROVE   BIT6
#define GROVE_DIR       P1DIR
#define TRANS   BIT0
#define TRANS_DIR       P2DIR
#define TRANS_OUT       P2OUT

int value = 0;
void clockSetup1MHZ(void){
// Configure CPU clock
        if (CALBC1_1MHZ==0xFF)              // If calibration constant erased
        {
                while(1);                   // do not load, trap CPU
        }
        DCOCTL = 0;                     // Select lowest DCOx and MODx settings
        BCSCTL1 = CALBC1_1MHZ;          // Set DCO to 1MHz
        DCOCTL = CALDCO_1MHZ;
        /* END OF DO NOT MODIFY */
}

void adc_init()
{
        /*initialize ADC*/
        ADC10CTL0 = SREF_1 + REFON + ADC10SHT_3 + ADC10ON; // 1.5V ref, Ref ON, 64 clocks for sample
        ADC10CTL1 = INCH_6 + ADC10DIV_3;                                   // Temp Sensor is at channel 10 and CLK/4
        ADC10AE0 = GROVE;               //Set GROVE as ADC input
}

void UART_init()
{
        /*setting up UART*/
        P1SEL  |= BIT2;  //Enable module function on pin
        P1SEL2 |= BIT2;  //Select second module; P1.1 = RXD, P1.2=TXD

        UCA0CTL1 |= UCSSEL_2;    // Set clock source SMCLK
        UCA0BR0 = 104;           // 1MHz 9600 (N=BRCLK/baudrate)
        UCA0BR1 = 0;             // 1MHz 9600
        UCA0MCTL = UCBRS0;       // Modulation UCBRSx = 1
        UCA0CTL1 &= ~UCSWRST;    // **Initialize USCI state machine**
}


int main(void) {
    //Pin configuration
    GROVE_DIR &= ~GROVE;        //Set as input - grove sensor
    TRANS_DIR |= TRANS;
    P1IE |= GROVE;          // P1.6 interrupt enabled

    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
        clockSetup1MHZ();
    adc_init();
    //set_timer_A0();

    UART_init();

    while(1)
    {
        /*ADC conversion start*/
        __delay_cycles(100);            // Wait for reference to settle
        ADC10CTL0 |= ENC + ADC10SC;     // Sampling and conversion start
        while (ADC10CTL1 & BUSY);       // Wait..i am converting...
        value = ADC10MEM;                       // Read ADC memory

        while (!(IFG2&UCA0TXIFG));     // Wait until TX buffer is ready

        UCA0TXBUF = value/10;             // Send MCU temp value to UART
        value=value/10;
        __delay_cycles(100000);
        if(value<100)
        {
                TRANS_OUT &= ~0x01;
        }
        else
        {
                TRANS_OUT |= 0x01;
        }
    }
}

