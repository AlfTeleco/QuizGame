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
#include <stdlib.h>

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


enum Button {
    Start = 0,
    Red,
    Green,
    White,
    Blue
    };

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

unsigned char button_semaphore = 0;
char *button_name = "N_B";
unsigned int button = 0;
unsigned long  timer_counts = 0;

void flash( int ms_cycle, int n_times );
void wait_ms( int ms_cycle);
void integer_to_string( char *str, unsigned int number );
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
    P1DIR = BIT1;                               // Set all P1.x to input direction except TX
    P1IE =  ~BIT1;                              // All P1.x interrupts enabled except TX
    P1IES = ~BIT1;                              // P1.3 and P1.4 Hi/lo edge
    P1REN = ~( BIT1 + BIT2 );                   // Enable Pull Up on P1.4 and P1.5
    P1IFG =  0;                                 // P1.x IFG cleared
                                                // BIT3 on Port 1 can be used as Switch2
    P2DIR = 0xFF;                               // Set all P1.x to ouput direction

    // Configure TA1 to count
    TA1CTL = TASSEL_1 + ID_3 + MC_2 + TACLR;        // ACLK, upmode, clear TAR


    __enable_interrupt();

    TimerA_UART_init();                     // Start Timer_A UART
    TimerA_UART_print("\r\n");
    TimerA_UART_print("Bienvenido a la miniconsola QuizGame\r\n");
    TimerA_UART_print("Este proyecto es una idea de David Sandin Martin\r\n");
    TimerA_UART_print("para el colegio Sagrado Corazon de Jesus que vio su luz\r\n");
    TimerA_UART_print("en marzo del 2020.\r\n");
    TimerA_UART_print("Ideado y realizado por antiguos alumnos del Amor de dios.\r\n");

    P2OUT = BIT0 + BIT1 + BIT2;

    flash( 25, 25 );

    char time[] = "0";
    for(;;)
    {
        if( button_semaphore )
        {
            switch (button) {
                case Start:
                    button_name = "Go...!_\r";
                    TimerA_UART_print(button_name);
                    int t_time = 0;
                    srand( timer_counts % 10 );
                    while( t_time > 2000 || t_time < 500 )
                    {
                        t_time = rand();
                    }
                    //integer_to_string( time, t_time );
                   // TimerA_UART_print(time);
                    wait_ms( t_time );
                    P2OUT = 0;
                    TA1CTL = TASSEL_1 + ID_3 + MC_2 + TACLR;        // Clear and starts the timer
                    break;
                case Red:
                    timer_counts *= 244;
                    timer_counts /=1000;
                    integer_to_string( time, timer_counts );
                    button_name = "_R*";
                    TimerA_UART_print(time);
                    TimerA_UART_print("_ms");
                    TimerA_UART_print(button_name);
                    break;
                case White:
                    timer_counts *= 244;
                    timer_counts /=1000;
                    integer_to_string( time, timer_counts );
                    button_name = "_W*";
                    TimerA_UART_print(time);
                    TimerA_UART_print("_ms");
                    TimerA_UART_print(button_name);
                    break;
                case Green:
                    timer_counts *= 244;
                    timer_counts /=1000;
                    integer_to_string( time, timer_counts );
                    button_name = "_G*";
                    TimerA_UART_print(time);
                    TimerA_UART_print("_ms");
                    TimerA_UART_print(button_name);

                    break;
                case Blue:
                    timer_counts *= 244;
                    timer_counts /=1000;
                    integer_to_string( time, timer_counts );
                    button_name = "_B*";
                    TimerA_UART_print(time);
                    TimerA_UART_print("_ms");
                    TimerA_UART_print(button_name);
                    break;
                default:
                    break;
            }
            wait_ms(50);
            timer_counts = 0;
            button_semaphore = 0;
            P1IFG =  0;                                 // P1.x IFG cleared
        }
    }

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

    if( !button_semaphore )
    {
        TA1CTL &= MC_0;        // Stops the timer
        timer_counts = TA1R;
        if( P1IFG & BIT0 )
        {
            button = Start;
            P1IE =  ~BIT1;                              // All P1.x interrupts enabled except TX
            P2OUT  = BIT2;
        }
        else if ( P1IFG & BIT4 )
        {
            P2OUT = BIT1;
            button = Blue;
            P1IE = BIT0 + BIT2;
        }
        else if ( P1IFG & BIT5 )
        {
            P2OUT = BIT0;
            button = Red;
            P1IE = BIT0 + BIT2;
        }
    }

    button_semaphore = 1;
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

//------------------------------------------------------------------------------
// Function configures Timer_A for full-duplex UART operation
//------------------------------------------------------------------------------
void TimerA_UART_init(void)
{
    TACCTL0 = OUT;                          // Set TXD Idle as Mark = '1'
    TACCTL1 = SCS + CM1 + CAP + CCIE;       // Sync, Neg Edge, Capture, Int
    TACTL = TASSEL_2 + MC_2;                // SMCLK, start in continuous mode
}
//------------------------------------------------------------------------------
// Outputs one byte using the Timer_A UART
//------------------------------------------------------------------------------
void TimerA_UART_tx(unsigned char byte)
{
    while (TACCTL0 & CCIE);                 // Ensure last char got TX'd
    TACCR0 = TAR;                           // Current state of TA counter
    TACCR0 += UART_TBIT;                    // One bit time till first bit
    TACCTL0 = OUTMOD0 + CCIE;               // Set TXD on EQU0, Int
    txData = byte;                          // Load global variable
    txData |= 0x100;                        // Add mark stop bit to TXData
    txData <<= 1;                           // Add space start bit
}

//------------------------------------------------------------------------------
// Prints a string over using the Timer_A UART
//------------------------------------------------------------------------------
void TimerA_UART_print(char *string)
{
    while (*string) {
        TimerA_UART_tx(*string++);
    }
}
//------------------------------------------------------------------------------
// Timer_A UART - Transmit Interrupt Handler
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    static unsigned char txBitCnt = 10;

    TACCR0 += UART_TBIT;                    // Add Offset to CCRx
    if (txBitCnt == 0) {                    // All bits TXed?
        TACCTL0 &= ~CCIE;                   // All bits TXed, disable interrupt
        txBitCnt = 10;                      // Re-load bit counter
    }
    else {
        if (txData & 0x01) {
          TACCTL0 &= ~OUTMOD2;              // TX Mark '1'
        }
        else {
          TACCTL0 |= OUTMOD2;               // TX Space '0'
        }
        txData >>= 1;
        txBitCnt--;
    }
}
//------------------------------------------------------------------------------
// Timer_A UART - Receive Interrupt Handler
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) Timer_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    static unsigned char rxBitCnt = 8;
    static unsigned char rxData = 0;

    switch (__even_in_range(TA0IV, TA0IV_TAIFG)) { // Use calculated branching
        case TA0IV_TACCR1:                        // TACCR1 CCIFG - UART RX
            TACCR1 += UART_TBIT;                 // Add Offset to CCRx
            if (TACCTL1 & CAP) {                 // Capture mode = start bit edge
                TACCTL1 &= ~CAP;                 // Switch capture to compare mode
                TACCR1 += UART_TBIT_DIV_2;       // Point CCRx to middle of D0
            }
            else {
                rxData >>= 1;
                if (TACCTL1 & SCCI) {            // Get bit waiting in receive latch
                    rxData |= 0x80;
                }
                rxBitCnt--;
                if (rxBitCnt == 0) {             // All bits RXed?
                    rxBuffer = rxData;           // Store in global variable
                    rxBitCnt = 8;                // Re-load bit counter
                    TACCTL1 |= CAP;              // Switch compare to capture mode
                    //__bic_SR_register_on_exit(LPM0_bits);  // Clear LPM0 bits from 0(SR)
                }
            }
            break;
    }
}
//------------------------------------------------------------------------------

void flash( int ms_cycle, int n_times )
{
    int l_var0 = 0;
    for( l_var0 = 0; l_var0 < n_times; l_var0++ )
    {
        P2OUT ^= BIT0 + BIT1 + BIT2;
        wait_ms(ms_cycle);
        P2OUT ^= BIT0 + BIT1 + BIT2;
        wait_ms(ms_cycle);
    }

    P2OUT &= ~( BIT0 + BIT1 + BIT2);
}

void wait_ms(int ms_cycle)
{
    int l_var0 = 0;
    for( l_var0 = 0; l_var0 < ms_cycle; l_var0++ )
    {
        __delay_cycles(1000);
    }
}

void integer_to_string( char *str, unsigned int number )
{
    // number of figures?
    int number_of_figures = 1;
    unsigned int t_number = number;

    while( (t_number /= 10) > 0 )
    {
        number_of_figures++;
    }

    t_number = number;

    int l_var0 = 0;
    char *p = str;
    p = p + number_of_figures - 1;

    for( l_var0 = number_of_figures; l_var0 > 0; l_var0-- )
    {
        *p = t_number%10 + 48;
        p--;
        t_number /= 10;
    }
    p = str + number_of_figures;
    *p = '\0';

}
