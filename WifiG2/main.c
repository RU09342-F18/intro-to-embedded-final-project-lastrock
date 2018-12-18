//
// Sandbox, code is not complete or used
// for the backup sensor project
//


#include <msp430.h>
#include <stdio.h>
#include <stdlib.h>

char value; // last char before "\n" is an int
char prev; // hold previous value in case new int

void UARTSetup(void)
{
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    /* Configure Pin Muxing P1.1 RXD and P1.2 TXD */
    P1SEL = BIT1 | BIT2 ;
    P1SEL2 = BIT1 | BIT2;

    /* Place UCA0 in Reset to be configured */
    UCA0CTL1 = UCSWRST;
    /* Configure */
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 104; // 1MHz 9600
    UCA0BR1 = 0; // 1MHz 9600
    UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1

    /* Take UCA0 out of reset */
    UCA0CTL1 &= ~UCSWRST;

    /* Enable USCI_A0 RX interrupt */
    IE2 |= UCA0RXIE;
}

void subscribe(void)
{
    char pub[] = {"$home/backupSensor\n"};
    int i;
    for(i = 0; i<19; i++)
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

    subscribe();

    __bis_SR_register(LPM0_bits + GIE); // Enter LPM0, interrupts enabled
}

// Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    //char newChar = UCA0RXBUF;
    //char newLine = '\n';
    //if(newChar == newLine)
    //{
    //    value = prev;
        while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
        UCA0TXBUF = UCA0RXBUF;
    //}
    //prev = newChar;
}
