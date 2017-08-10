#include "msp430.h";

#define BUTTON BIT3
#define RXD BIT1
#define TXD BIT2

static volatile char tenths = 0;
static volatile char seconds = 0;
static volatile char data;
char numberStr[5];

/*Default brightness settings */
unsigned int Bright[3]  = {1000,500,100};

//NOTE: These integers below can be consolidated into a single array
unsigned int NextState= 1; //next lamp state, default 1
unsigned int LastState = 1;
unsigned int CurrentState = 0;//Used for UART Input to assign input values to Bright[CurrentState-1] as long as CurrentState != 0

unsigned char CycleComplete = 0; //used to tell whether we should return to state3 or state1 from state0 
//defaults at 0 to prevent needed 2 presses at initial state
 
/* Function Prototype */
void ConfigureWDT(void);
void InitializeButton(void);
void ConfigureTimerA(void);
void ConfigureUART(void);
void UARTSendArray(unsigned char *TxArray, unsigned char ArrayLength);
char Int2DecStr(char *str, unsigned int value);
 
/* Main function */
void main(void)
{
 WDTCTL = WDTPW + WDTHOLD;          // Stop WDT 
 //ConfigureWDT();
 BCSCTL1 = CALBC1_1MHZ; // Set DCO Clock to 1MHz
 DCOCTL = CALDCO_1MHZ;
 
 P1DIR |= BIT6;              // P1.6 to output
 P1SEL |= BIT6;             //P1.6 to TA0.1 (Timer output bit)
 
 //TA0CCR0 = 1000-1;           //PWM period
 TA0CCTL1 = OUTMOD_7;    //CCR1 Reset/Set
 //TACTL = TASSEL_2 +MC_1; //SMCLK, up mode
 //CCR1 = 0;             // Set the LED P1.6 to off
 
 InitializeButton();
 ConfigureUART();
 ConfigureTimerA();
 UARTSendArray(" Enter + or - to adjust brightness of current mode:",51);
 UARTSendArray("\n\r", 2);
 
 _BIS_SR(LPM0_bits+GIE);        // Enter LPM0 with interrupts enabled
}
 
/* This function configures the button so it will trigger interrupts
 * when pressed.  Those interrupts will be handled by PORT1_ISR() */
void InitializeButton(void)
{
    P1DIR &= ~BUTTON;   // Set button pin as an input pin
    P1OUT |= BUTTON;    // Set pull up resistor on for button
    P1REN |= BUTTON;    // Enable pull up resistor for button to keep pin high until pressed
    P1IES |= BUTTON;    // Enable Interrupt to trigger on the falling edge (high (unpressed) to low (pressed) transition)
    P1IFG &= ~BUTTON;   // Clear the interrupt flag for the button
    P1IE |= BUTTON;     // Enable interrupts on port 1 for the button
}

void ConfigureTimerA(void){
/* Configure timer A as a clock divider to generate timed interrupt */
TACCTL0 = CCIE;
TACTL = TASSEL_2 +ID_3+MC_1; // Use the SMCLK to clock the counter, SMCLK/8, count up mode
TACCR0 = 1250-1; // Set maximum count (Interrupt frequency 1MHz/8/12500 = 10Hz)
}

/* Port 1 interrupt to service the button press */
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  P1IFG &= ~BUTTON;                 // Clear the interrupt flag for the button
  P1IE &= ~BUTTON;                  // Disable Button interrupt
  WDTCTL = WDT_MDLY_32;             // Start and set watchdog timer (WDT) to trigger every 32ms
  IFG1 &= ~WDTIFG;                  // Clear the interrupt flag for the WDT
  IE1 |= WDTIE;                 // enable WDT interrupt
 
 switch (NextState){
   
   case 0:
   CCR1 = 0;
   CurrentState=0;
   if (CycleComplete == 1){
     NextState=1;
     CycleComplete =0;
   } else{
     NextState = LastState;
   }
   
   break;
   
   case 1:
   CCR1 = Bright[0];
   CurrentState=1;
   LastState=1;
   NextState=2;
   seconds = 0;
   tenths = 0;
   break;
   
   case 2:
   CCR1 = Bright[1];
   CurrentState=2;
   LastState=2;
   NextState=3;
   seconds = 0;
   tenths = 0;
   break;
   
   case 3:
   CCR1=Bright[2];
   CurrentState=3;
   LastState=3;
   NextState=0;
   seconds = 0;
   tenths=0;
   CycleComplete =1;
   
   break;
 }
}

//Used to change brightness via Hardware UART
// Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void){
  
data = UCA0RXBUF;
switch(data){
  case '+':
  if(CurrentState>0){
    if(Bright[CurrentState-1]<=1100){
      Bright[CurrentState-1]+=100;
      CCR1 = Bright[CurrentState-1];
      UARTSendArray("The current brightness level is: ",33);
      UARTSendArray(numberStr,Int2DecStr(numberStr, Bright[CurrentState-1]));
      UARTSendArray("\n\r", 2);
    }else{
      UARTSendArray("This mode is already at max brightness!",39);
      UARTSendArray("\n\r", 2);
    }
  }else{
    UARTSendArray("Select a mode first.",20);
    UARTSendArray("\n\r", 2);
  }
  break;
  
  case '-':
  if(CurrentState>0){
    if(Bright[CurrentState-1]>=200){
      Bright[CurrentState-1]-=100;
      CCR1 = Bright[CurrentState-1];
      UARTSendArray("The current brightness level is: ",33);
      UARTSendArray(numberStr,Int2DecStr(numberStr, Bright[CurrentState-1]));
      UARTSendArray("\n\r", 2);
    }else{
      UARTSendArray("This mode is already at minimum brightness!",43);
      UARTSendArray("\n\r", 2);
    }
  }else{
    UARTSendArray("Select a mode first.",20);
    UARTSendArray("\n\r", 2);
  }
  break;
  
  default:
  UARTSendArray(" Enter + or - to adjust brightness of current mode:",66);
  UARTSendArray("\n\r", 2);
  break;
}
}

void UARTSendArray(unsigned char *TxArray, unsigned char ArrayLength){
 // Send number of bytes Specified in ArrayLength in the array at using the hardware UART 0
 // Example usage: UARTSendArray("Hello", 5);
 // int data[2]={1023, 235};
 // UARTSendArray(data, 4); // Note because the UART transmits bytes it is necessary to send two bytes for each integer hence the data length is twice the array length
 
while(ArrayLength--){ // Loop until StringLength == 0 and post decrement
 while(!(IFG2 & UCA0TXIFG)); // Wait for TX buffer to be ready for new data
 UCA0TXBUF = *TxArray; //Write the character at the location specified py the pointer
 TxArray++; //Increment the TxString pointer to point to the next character
 }
}

void ConfigureUART(void){
 /* Configure hardware UART */
 P1SEL |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
 P1SEL2 |= RXD + TXD ; // P1.1 = RXD, P1.2=TXD
 UCA0CTL1 |= UCSSEL_2; // Use SMCLK
 UCA0BR0 = 104; // Set baud rate to 9600 with 1MHz clock (Data Sheet 15.3.13)
 UCA0BR1 = 0; // Set baud rate to 9600 with 1MHz clock
 UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1
 UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
 IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
}

static const unsigned int dec[] = {
 10000, // +5
 1000, // +6
 100, // +7
 10, // +8
 1, // +9
 0
};
 
char Int2DecStr(char *str, unsigned int value){
 char c;
 char n=0;
 int *dp = dec;
 
while (value < *dp) dp++; // Move to correct decade
 do {
 n++;
 c = 0; // count binary
 while((value >= *dp) && (*dp!=0)) ++c, value -= *dp;
 *str++ = c+48; //convert to ASCII
 }
 while(*dp++ >1);
 return n;
}
 
// WDT Interrupt Service Routine used to de-bounce button press
#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void)
{
    IE1 &= ~WDTIE;                   // disable Watchdog timer (WDT) interrupt
    IFG1 &= ~WDTIFG;                 // clear WDT interrupt flag
    WDTCTL = WDTPW + WDTHOLD;        // put WDT back in hold state
    P1IE |= BUTTON;                  // Reenable interrupts for the button
}

// TimerA interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
if (++tenths == 100){
  tenths = 0;
  ++seconds;
  }
if ((seconds>5)&&(CCR1!=0)){
  NextState=0;
  seconds=0;
  CycleComplete = 0;
  }
//UARTSendArray(numberStr,Int2DecStr(numberStr, seconds));//Uncomment for debugging timer
}

