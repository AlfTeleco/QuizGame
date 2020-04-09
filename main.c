//******************************************************************************
//  MSP430G2553 - QuizGame for the students of the school Sagrado Corazon de Jesus
//  The game consists of 6 arcade machine style buttons which must be pushed when
//  as soon as the game master cast a question.
//  The first button to be pushed will light until the game master reset it,
//  showing which player was the fastest answering the question.
//
//  Meanwhile, the firmware will be sending info to the PC through a serial
//  connection. This info will be received in a QT GUI application that will
//  let the teacher control the game.
//
//  Timer_A, Ultra-Low Pwr UART 9600 Echo, 32kHz ACLK
//
//  Description:

//  ACLK = TACLK = LFXT1 = 32768Hz, MCLK = SMCLK = default DCO
//  //* An external watch crystal is required on XIN XOUT for ACLK *//
//
//               MSP430G2553
//            -----------------
//        /|\|              XIN|-
//         | |                 | 32kHz
//         --|RST          XOUT|-
//           |                 |
//           |   CCI0B/TXD/P1.1|-------->
//           |                 | 9600 8N1
//           |   CCI0A/RXD/P1.2|<--------
//
//  A.Guzman
//  March 2020
//   Built with Code Composer Studio Version: 8.2.0.00007
//******************************************************************************


#include <msp430.h>
//------------------------------------------------------------------------------
// Hardware-related definitions
//------------------------------------------------------------------------------
#define UART_TXD   0x02                     // TXD on P1.1 (Timer0_A.OUT0)
#define UART_RXD   0x04                     // RXD on P1.2 (Timer0_A.CCI1A)

//------------------------------------------------------------------------------
// Conditions for 9600 Baud SW UART, SMCLK = 1MHz
//------------------------------------------------------------------------------
#define UART_TBIT_DIV_2     (1000000 / (9600 * 2))
#define UART_TBIT           (1000000 / 9600)

//------------------------------------------------------------------------------
// Global variables used for full-duplex UART communication
//------------------------------------------------------------------------------
unsigned int txData;                        // UART internal variable for TX
unsigned char rxBuffer;                     // Received UART character

//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------
void TimerA_UART_init(void);
void TimerA_UART_tx(unsigned char byte);
void TimerA_UART_print(char *string);

unsigned char pulsadores = 0;


void flash( int ms_cycle, int n_times );
void wait_ms( int ms_cycle);

/**
 * blink.c
 */
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                   // Stop WDT
    if (CALBC1_1MHZ==0xFF)                      // If calibration constant erased
    {
        while(1);                               // do not load, trap CPU!!
    }
    DCOCTL = 0;                                 // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                      // Set DCO
    DCOCTL = CALDCO_1MHZ;
    P1SEL = UART_TXD + UART_RXD;                // Timer function for TXD/RXD pins
    P1DIR |= BIT1 + BIT2;                       // Set P1.1 and P1.2  to output direction
    P1IE |=  BIT3 + BIT4 + BIT5;                // P1.3 and P1.4 interrupt enabled
    P1IES |= BIT3 + BIT4 + BIT5;                // P1.3 and P1.4 Hi/lo edge
    P1REN |= BIT3 + BIT4 + BIT5;                // Enable Pull Up on P1.4 and P1.5
    P1IFG &=  ~( BIT3 + BIT4 + BIT5);           // P1.3 IFG cleared
                                                //BIT3 on Port 1 can be used as Switch2
    P1OUT = BIT3 + BIT4 + BIT5;

    flash( 25, 25 );

    __bis_SR_register(LPM0_bits + GIE);       // Enter LPM4 w/interrupt
}

// Port 1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)
#else
#error Compiler not supported!
#endif
{

    if( P1IFG & BIT4 )
    {
        P2OUT ^= BIT1;
        __delay_cycles(500000);
                                 // P1.4 IFG cleared
    }
    else if ( P1IFG & BIT5 )
    {
        P2OUT ^= BIT2;
        __delay_cycles(500000);

    }
    P1IFG = 0;

}

// Echo back RXed character, confirm TX buffer is ready first
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
  while (!(IFG2 & UCA0TXIFG));              // USCI_A0 TX buffer ready?
  UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character
}

void flash( int ms_cycle, int n_times )
{
    int l_var0 = 0;
    for( l_var0 = 0; l_var0 < n_times; l_var0++ )
    {
        P2OUT ^= BIT1 + BIT2;
        wait_ms(ms_cycle);
        P2OUT ^= BIT1 + BIT2;
        wait_ms(ms_cycle);
    }

    P2OUT &= ~( BIT1 + BIT2);
}

void wait_ms(int ms_cycle)
{
    int l_var0 = 0;
    for( l_var0 = 0; l_var0 < ms_cycle; l_var0++ )
    {
        __delay_cycles(1000);
    }
}
