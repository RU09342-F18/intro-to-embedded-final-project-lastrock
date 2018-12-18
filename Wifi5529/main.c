#include <msp430.h>

int value; // last char before "\n" is an int
char prev; // hold previous value in case new int
char curr; // current value

int brightness[50] = {0xFF, 0xD2, 0xB7, 0xA4, 0x96,
                      0x8A, 0x80, 0x77, 0x70, 0x69,
                      0x62, 0x5D, 0x58, 0x53, 0x4E,
                      0x4A, 0x46, 0x42, 0x3F, 0x3B,
                      0x38, 0x35, 0x32, 0x2F, 0x2D,
                      0x2A, 0x28, 0x25, 0x23, 0x21,
                      0x1F, 0x1D, 0x1B, 0x19, 0x17,
                      0x15, 0x13, 0x11, 0x10, 0x0E,
                      0x0C, 0x0B, 0x09, 0x08, 0x06,
                      0x05, 0x04, 0x02, 0x01, 0x00};

void UARTSetup(void)
{
    P3SEL |= (BIT4|BIT3); //RX|TX peripheral mode
    UCA0CTL1 |= UCSWRST; // reset state machine
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 6; // 9600 baud
    UCA0BR1 = 0;
    UCA0MCTL |= UCBRS_0|UCBRF_13|UCOS16; // modulation UCBRSx=0, UCBRFx=0
    UCA0CTL1 &= ~UCSWRST; // initialize state machine
    UCA0IE |= UCRXIE; // enable UART interrupt
}

void PWMSetup(void)
{
    TA0CCR0 = 0xFF; // reset register
    TA0CCR1 = 0x00; // red 1
    TA0CCR2 = 0x00; // red 2
    TA0CCR3 = 0x00; // red 3

    TA0CCTL1 = OUTMOD_7; // set/reset mode
    TA0CCTL2 = OUTMOD_7;
    TA0CCTL3 = OUTMOD_7;

    TA0CTL = MC_1 | TASSEL_2 | ID_2; // up | SMCLK | 2^1 division
}

void RGBLEDSetup(void)
{
    P1DIR |= (BIT2|BIT3|BIT4); // P1.(2-4) RGB LED output pins
    P1SEL |= (BIT2|BIT3|BIT4); // P1.(2-4) = CCR(1-3)
}

void LEDSetup(void)
{
    P1DIR |= BIT0; // status LED
    P1OUT &= ~BIT0; // off
}

void initSetup(void)
{
    UARTSetup();
    LEDSetup();
    PWMSetup();
    RGBLEDSetup();
}

void subscribe(void)
{
    char pub[] = {"$home/backupSensor\n"};
    int i;
    for(i = 0; i<19; i++)
    {
        while(!(UCA0IFG & UCTXIFG)); // wait while transmitting
        UCA0TXBUF = pub[i];
    }
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog
    initSetup();

    subscribe();
    __delay_cycles(1000000);

    __bis_SR_register(LPM0_bits + GIE); // low power mode + enable global interrupt
}

int byteCount = 0;
int byteLn = 0;
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0 (void)
{
    P1OUT |= BIT0; // status on
    // If there is data (i.e. not Null)
    switch(__even_in_range(UCA0IV, USCI_UCTXIFG)) // UCA0IV == USCI_UCRXIFG
    {
        case USCI_UCRXIFG:
            curr = UCA0RXBUF;
            if('\n' == curr)
            {
                value = prev;
                //while(!(UCA0IFG & UCTXIFG)); // wait while transmitting
                //UCA0TXBUF = value; // byte length
            }
            prev = curr;
            break;

        case USCI_UCTXIFG :
            break;
        default:
            break;
    }

    if(value >= 80)
    {
        TA0CCR1 = 0x00; // red 1
        TA0CCR2 = 0x00; // red 2
        TA0CCR3 = 0x00; // red 3
    }
    else if(value >= 60)
    {
        TA0CCR1 = brightness[value-30]; // red 1
        TA0CCR2 = 0x00; // red 2
        TA0CCR3 = 0x00; // red 3
    }
    else if(value >= 30)
    {
        TA0CCR1 = brightness[value-30]; // red 1
        TA0CCR2 = brightness[value-10]; // red 2
        TA0CCR3 = 0x00; // red 3
    }
    else if(value >= 10)
    {
        TA0CCR1 = 0xFF; // red 1
        TA0CCR2 = brightness[value-10]; // red 2
        TA0CCR3 = brightness[value]; // red 3
    }
    else
    {
        TA0CCR1 = 0xFF; // red 1
        TA0CCR2 = 0xFF; // red 2
        TA0CCR3 = brightness[value]; // red 3
    }
}
