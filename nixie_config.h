//I have one clock, with Burroughs tubes, that I bought as a ready-to-run
//complete unit. It rapid-cycles the digits for 5 seconds every hour.
//Rapid-cycle meaning each digit stays on for about 0.1 seconds for a total
//of 0.5 seconds on time for each digit in that 5 second routine. The cycling
//order is numeric, i.e. 0 - 1 - 2 - ... - 9 - 0. Tubes still look as good today
//as they did when I got it 6 years ago, It runs 16 - 18 hours a day.

//I also have several clocks I have designed and built myself that use various
//kinds of Russian tubes, including one that uses IN-18s with "bad" (old style
//with little or no mercury in them) date codes. These I rapid-cycle (0.1 sec / digit)
//for 30 seconds every hour but I cycle one tube at a time, working my way across 
//the display, 5 times. Thus the time is not totally unreadable during the cycling
//and it looks cool. Net result is still 0.5 seconds per digit per tube. Also, on 
//these clocks I don't cycle in mumerical order, I cycle in stacking order, back 
//to front, mainly just because I like how it looks. The tubes show no signs 
//of poisoning so far.


#define hvData 2
#define hvClock 3
#define hvStrobe 4

#define lvLatch 6
#define lvData 7
#define lvClock 8

#define leds A1
#define NUMPIXELS 6

#define espTX A2
#define espRX A3

#define touchRST 5
#define touchCHG 0


//int oddDigitMap[] = {0,9,8,7,6,1,2,3,4,5};  //0987612345 actual digits
//int evenDigitMap[] = {6,7,8,9,0,5,4,3,2,1}; //6789054321 actual digits

//int oddDigitMap[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; //0987612345 actual digits
//int evenDigitMap[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}; //6789054321 actual digits

int anodes[] = {A0, 13, 12, 11, 10, 9};

int oddDigitMap[10][2] = //true output number, true output HVSR section (1-10 or 11-20)
{
  {0, 1}, //00
  {5, 0}, //01
  {6, 0}, //02
  {7, 0}, //03
  {8, 0}, //04
  {9, 0}, //05
  {4, 1}, //06
  {3, 1}, //07
  {2, 1}, //08
  {1, 1}  //09
};

int evenDigitMap[10][2] = //true output number, true output HVSR section (1-10 or 11-20)
{
  {4, 0}, //10
  {9, 1}, //11
  {8, 1}, //12
  {7, 1}, //13
  {6, 1}, //14
  {5, 1}, //15
  {0, 0}, //16
  {1, 0}, //17
  {2, 0}, //18
  {3, 0}, //19
};


int numbers[] = {
  0b0111111111,
  0b1011111111,
  0b1101111111,
  0b1110111111,
  0b1111011111,
  0b1111101111,
  0b1111110111,
  0b1111111011,
  0b1111111101,
  0b1111111110
};

//Touch IC registers

//Touch IC Botton values in HEX:
//8  S
//20 +
//10 -

#define myAddress 0x1F
#define touchICReg 0x1B

#define touchICCal 56
#define touchICMaxOn 55
#define touchICNTHR0 32
#define touchICNTHR1 33
#define touchICNTHR2 34
#define touchICAKS0 39
#define touchICKeyStatus 3

//softPWM
SOFTPWM_DEFINE_CHANNEL(0, DDRC, PORTC, PORTC0);  //Arduino pin A0
SOFTPWM_DEFINE_CHANNEL(1, DDRB, PORTB, PORTB5);  //Arduino pin 13
SOFTPWM_DEFINE_CHANNEL(2, DDRB, PORTB, PORTB4);  //Arduino pin 12
SOFTPWM_DEFINE_CHANNEL(3, DDRB, PORTB, PORTB3);  //Arduino pin 11
SOFTPWM_DEFINE_CHANNEL(4, DDRB, PORTB, PORTB2);  //Arduino pin 10
SOFTPWM_DEFINE_CHANNEL(5, DDRB, PORTB, PORTB1);  //Arduino pin 9


SOFTPWM_DEFINE_OBJECT_WITH_PWM_LEVELS(6, 80);
