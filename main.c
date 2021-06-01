#include <msp430.h> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

volatile int temp[50];
volatile int diff[50];
volatile unsigned int i=0;
volatile unsigned int j=0;

char th_char[5];
char tl_char[5];
char hh_char[5];
char hl_char[5];

volatile int hh = 0;
volatile int hl = 0;
volatile int th = 0;
volatile int tl = 0;
volatile int check = 0;
volatile int checksum = 0;
volatile int dataok;

char temperature[] = "Temperature is: ";
char dot[] = ".";
char celcius[] = "\r\n";
char humidity[] = "Humidity is: ";
char percent[] = "\r\n";

const char *pTxByte;    //pointer to constant char

//
int adc[10] = {0}; //Sets up an array of 10 integers and zero's the values
int avg_adc = 0;

char soil_char[5];

char n_char[5];

char c[] = "\r\n";

void adc_Setup();
void adc_Sam10();
//

void ser_output(char *str);


void main(void)
{
     WDTCTL = WDTPW | WDTHOLD;
     BCSCTL1= CALBC1_1MHZ;
     DCOCTL = CALDCO_1MHZ;
     P1SEL = BIT1|BIT2;
     P1SEL2 = BIT1|BIT2;
     UCA0CTL1 |= UCSWRST+UCSSEL_2;
     UCA0BR0 = 52;
     UCA0BR1 = 0;
     UCA0MCTL = UCBRS_0;
     UCA0CTL1 &= ~UCSWRST;
     __delay_cycles(2000000);
        P2DIR |= BIT4; //P2.4 is output
        P2OUT &= ~BIT4; //initial set P2.4 to low
        __delay_cycles(20000);
        P2OUT |= BIT4; //set P2.4 to high
        __delay_cycles(20); // 20 micro second as high level
        P2DIR &= ~BIT4; // set to input port
        P2SEL |= BIT4; // time clock
        TA1CTL = TASSEL_2|MC_2 ; //timer a control
        TA1CCTL2 = CAP | CCIE | CCIS_0 | CM_2 | SCS ; // capture mode important!!!
        _enable_interrupts();

        adc_Setup();

        while (1){
            //DHT11 sill sent 40bits ,each is 8 bits ,represent as hh,hl,th,tl,checksum
            if (i>=40){
                for (j = 1; j <= 8; j++){
                    if (diff[j] >= 110)
                        hh |= (0x01 << (8-j));
                    }
                for (j = 9; j <= 16; j++){
                    if (diff[j] >= 110)
                        hl |= (0x01 << (16-j));
                }
                for (j = 17; j <= 24; j++){
                    if (diff[j] >= 110)
                        th |= (0x01 << (24-j));
                }
                for (j = 25; j <= 32; j++){
                    if (diff[j] >= 110)
                        tl |= (0x01 << (32-j));
                }
                for (j = 33; j<=40; j++){
                    if (diff[j] >= 110)
                        checksum |= (0x01 << (40-j));
                }
                check=hh+hl+th+tl;
                if (check == checksum)
                    dataok = 1;
                else
                    dataok = 0;
                ltoa(th,th_char,10); //*ltoa(long val, char * buffer, int radix); radix=10 mens use Decimal
                ltoa(tl,tl_char,10);
                ltoa(hh,hh_char,10);
                ltoa(hl,hl_char,10);

//                ser_output(temperature); ser_output(th_char); ser_output(dot); ser_output(tl_char); ser_output(celcius);
//                ser_output(humidity); ser_output(hh_char); ser_output(dot); ser_output(hl_char); ser_output(percent);
                ser_output(th_char); ser_output(dot); ser_output(tl_char);
                ser_output(hh_char); ser_output(dot); ser_output(hl_char);
                //__delay_cycles(1000000);

                adc_Sam10();      // Function call for adc_samp
                          // Add all the sampled data and divide by 10 to find average
                          avg_adc = ((adc[0]+adc[1]+adc[2]+adc[3]+adc[4]+adc[5]+adc[6]+adc[7]+adc[8]+adc[9]) / 10);

                          ltoa(avg_adc,soil_char,10);
                          ser_output(soil_char);
                          ser_output(c);

                __delay_cycles(1000);
                WDTCTL = WDT_MRST_0_064;
                }

        }
}
#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer_A1(void){
        temp[i] = TA1CCR2;
        i += 1;
        TA1CCTL2 &= ~CCIFG ;
        if (i>=2) diff[i-1]=temp[i-1]-temp[i-2];
}
void ser_output(char *str){
    while(*str != 0){
        while (!(IFG2&UCA0TXIFG));
        UCA0TXBUF = *str++;
    }
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

// ADC set-up function
void adc_Sam10()
{
    ADC10CTL0 &= ~ENC;              // Disable Conversion
    while (ADC10CTL1 & BUSY);       // Wait if ADC10 busy
    ADC10SA = (int)adc;             // Transfers data to next array (DTC auto increments address)
    ADC10CTL0 |= ENC + ADC10SC;     // Enable Conversion and conversion start
    __bis_SR_register(CPUOFF + GIE);// Low Power Mode 0, ADC10_ISR
}

void adc_Setup()
{
    ADC10CTL1 = CONSEQ_2 + INCH_0;                      // Repeat single channel, A0
    ADC10CTL0 = ADC10SHT_2 + MSC + ADC10ON + ADC10IE;   // Sample & Hold Time + ADC10 ON + Interrupt Enable
    ADC10DTC1 = 0x0A;                                   // 10 conversions
    ADC10AE0 |= 0x01;                                   // P1.0 ADC option select
}

// ADC sample conversion function


