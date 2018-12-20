#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>

int miliseconds;
int distance;
long sensor;
int transmit = 1; // transmit unless greater than 100 cm

void UARTSetup(void)
{
    P1SEL  |=  BIT1 + BIT2;  // P1.1 UCA0RXD input
    P1SEL2 |=  BIT1 + BIT2;  // P1.2 UCA0TXD output
    P1DIR  &=  ~BIT1;
    P1DIR  |=  BIT2;
    UCA0CTL1 |= UCSWRST;  // reset state machine
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 6; // 9600 baud
    UCA0BR1 = 0;
    UCA0MCTL |= UCBRS_0|UCBRF_13|UCOS16; // modulation UCBRSx=0, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST; // initialize state machine
}

void publish(void)
{
    char pub[] = {"#backupSensor 0\n"}; // replace the 0
    pub[14] = distance; // char just stores int ASCII value
    int i;
    for(i = 0; i<16; i++)
    {
        while(!(IFG2&UCA0TXIFG));
        UCA0TXBUF = pub[i];
    }
}

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;           // disable watch dog timer
    TA0CCTL0 = CCIE;                    // CCR0 interrupt enabled
    TA0CCR0 = 1000;                     // 1ms at 1mhz
    TA0CTL = MC_1 | TASSEL_2 | ID_0;    // up count | SMCLK | 2^0 division

    P1IFG  = 0x00;              // clear all interrupt flags
    P1DIR |= BIT0;              // P1.0 as output for LED
    P1OUT &= ~BIT0;             // turn LED off

    UARTSetup();

    _BIS_SR(GIE);               // global interrupt enable
    while(1)
    {
        long values[3] = {0, 0, 0};
        int i;
        for(i = 0; i<3; i++) // drop any bad reads
        {
            P1IE &= ~BIT0;          // disable interupt
            P2DIR |= BIT0;          // trigger pin as output
            P2OUT |= BIT0;          // generate pulse
            __delay_cycles(10);     // for 10us
            P2OUT &= ~BIT0;         // stop pulse
            P1DIR &= ~BIT3;         // make pin P1.2 ECHO
            P1IFG &= ~BIT3;         // clear flag just in case anything happened before
            P1IE |= BIT3;           // enable interupt on ECHO pin
            P1IES &= ~BIT3;         // rising edge on ECHO pin
            __delay_cycles(30000);  // delay for 30ms (after this time echo times out if there is no object detected)
            values[i] = sensor;
        }
        if((abs(values[0] - values[1]) < abs(values[1] - values[2])) | (abs(values[0] - values[2]) < abs(values[1] - values[2])))
        {
            distance = values[0]/58;   // sensor input to cm
        }
        else
        {
            distance = values[1]/58;   // sensor input to cm
        }

        if((distance > 100) | (distance <= 0)) // only need up to 100 cm
        {
            distance = 100;
            if(transmit == 1)
            {
                P1OUT &= ~BIT0;
                transmit = 0;
                publish();
            }
            __delay_cycles(100000); // .1 second delay
        }
        else if(distance < 20 && distance != 0)
        {
            P1OUT |= BIT0;
            transmit = 1;
            publish();
        }
        else
        {
            P1OUT &= ~BIT0;
            transmit = 1;
            publish();
        }
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(P1IFG & BIT3)        // if interrupt
    {
        if(!(P1IES&BIT3))   // if pos edge
        {
            TA0CTL|=TACLR;   // clear timer
            miliseconds = 0;
            P1IES |= BIT3;  // set interrupt falling edge
        }
        else
        {
            sensor = (long)miliseconds*1000 + (long)TA0R;    // ECHO time

        }
        P1IFG &= ~BIT3; //clear flag
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  miliseconds++;
}

