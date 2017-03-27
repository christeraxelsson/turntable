//
// 01 modified 14 Dec 14 by Stuart Allen to equalise step times (was 2 then 'pause') > 02
// 02 modified 14 Dec 14 to allow direct writing to io port D
// 04 modified to use array to hold motor phase.
//    functions moveMotor(), stepMotor and releaseMotor() added
// 05 modified 21 Jun 15 to us timer 2 interrupts NOT
// Modified 7 Mar 16 to rotated ANTICLOCK until hall effect sensor detects magnet
//  then CLOCKWISEwise until it is not
//  then CLOCKWISEwise until a zero position
// Added 4 innput switches and 3 output LEDs
// 14 Mar 16 Tidied up code
// constants won't change. They're used here to set pin numbers:
const int hallPin = 12;     // the number of the hall effect sensor pin
const int ledPin =  13;     // the number of the LED pin

int Blue = 3; //motor cable colours
int Green = 4; //motor cable colours
int Yellow = 5; //motor cable colours
int Orange = 6; //motor cable colours
int SW1 = 9;  // consol switch no.1
int SW2 = 10;  // consol switch no.
int SW3 = 7;  // consol switch no.
int SW4 = 8;  // consol switch no.
int LED1 = 2; // consol LED red.
int LED2 = 11; // consol LED amber.
int LED3 = 13; // consol LED green.
int t1 = 8;     // no of step at speed 1 while accellerating.
int t2 = 16;    // no of steps at speeds 1 and 2 while accellerating
int s1 = 10;    // pause (delay) for speed 3
int s2 =  8;    // pause (delay) for speed 2
int s3 =  5;    // pause (delay) for speed 1

int ANTICLOCK = 1;
int CLOCKWISE = -1;
int BACKLASH = 100; // try 3 degrees
int DEG15 = 500;  // steps for 15 degrees
int DEG180=6000;  // steps for 180 deg
unsigned char motorStep[4] = {0x50,0x48,0x28,0x30};

// variables will change:
int hallState = 0;          // variable for reading the hall sensor status
int stepPos = 0;
int TTpos =0;

//#define DEBUG 1

void setup()
{
  // initialise input pins.
  pinMode(hallPin, INPUT);     
  pinMode(SW1, INPUT); 
  pinMode(SW1,INPUT_PULLUP);  
  pinMode(SW2, INPUT);     
  pinMode(SW2,INPUT_PULLUP);  
  pinMode(SW3, INPUT);     
  pinMode(SW3,INPUT_PULLUP);  
  pinMode(SW4, INPUT);     
  pinMode(SW4,INPUT_PULLUP);  
 // initialise output pins.
 pinMode(Blue, OUTPUT);
 pinMode(Green, OUTPUT);
 pinMode(Yellow, OUTPUT);
 pinMode(Orange, OUTPUT);
 pinMode(LED1, OUTPUT);
 pinMode(LED2, OUTPUT);
 pinMode(LED3, OUTPUT);
 #ifdef DEBUG
   Serial.begin(57600);
   digitalWrite(LED2, HIGH);
 #endif
 PORTD=(PORTD & 0x87) | motorStep[stepPos];
 delay(8);
 digitalWrite(LED1, HIGH);        // Red lLED indicates we are moving
 moveWhileHall(ANTICLOCK,8,HIGH); // Move ANTICLOCK until hall sensor detects turntable  bridge magnet
// move to edge of magnet 
  moveWhileHall(CLOCKWISE,8,LOW); // move CLOCKWISEwise until magnet no longer detected.
// now move to starting position
   moveMotor(128,CLOCKWISE,8);    // move 128 steps to zero position
   TTpos=0;                       // set position count to 0
   #ifdef DEBUG
     Serial.write("At position zero\n");
     digitalWrite(LED1, LOW );      // Red LED off
   #endif 
} // end of setup

void loop()
{ // main code here, to run repeatedly:
  int s;
  digitalWrite(LED3,HIGH);
// read switches
  s=digitalRead(SW1);
  if (s==LOW){    // 15 degrees clockwise
      delay(10);  // debounce period
      if(digitalRead(SW1)==LOW){      // if still low
        digitalWrite(LED3, LOW);
        moveMotor(DEG15,CLOCKWISE,s3);
      }
      while(digitalRead(SW1)==LOW); // do nothing until switch released
   }
  s=digitalRead(SW2);
  if (s==LOW){    // 15 degrees anticlockwise
      delay(10);  // debounce period
      if(digitalRead(SW2)==LOW){      // if still low
        digitalWrite(LED3, LOW);
        moveMotor(DEG15 + BACKLASH ,ANTICLOCK,s3);
        moveMotor(BACKLASH,CLOCKWISE,s3);
      }
      while(digitalRead(SW2)==LOW); // do nothing until switch released
   }
  s=digitalRead(SW3);
  if (s==LOW){    // 180 degrees clockwise
      delay(10);  // debounce period
      if(digitalRead(SW3)==LOW){      // if still low
        digitalWrite(LED3, LOW);
        moveMotor(DEG180,CLOCKWISE,s3);
      }
      while(digitalRead(SW3)==LOW); // do nothing until switch released
   }
  s=digitalRead(SW4);
  if (s==LOW){    // Back to position 0
      delay(10);  // debounce period
      if(digitalRead(SW4)==LOW){      // if still low
        digitalWrite(LED3, LOW);
        if(TTpos>0){
          moveMotor(TTpos,CLOCKWISE,s3);          
        }
        else
        {
          moveMotor(-TTpos + BACKLASH ,ANTICLOCK,s3);
          moveMotor(BACKLASH,CLOCKWISE,s3);
        }
      }
      while(digitalRead(SW4)==LOW); // do nothing until switch released
   }
}
// DIR ANTICLOCK = 1 , CLOCKWISE = -1)
// delaying PAUSE millisecs between steps
void moveMotor (int nsteps, int dir, int pause){
  int i,s;
  int t3 = nsteps-t2;
  int t4 = nsteps-t1;
  #ifdef DEBUG
    Serial.write("Turntable position was ");
    Serial.print(TTpos);
  #endif
  digitalWrite(LED1,HIGH);  // Red LED on for moving
  PORTD=(PORTD & 0x87) | motorStep[stepPos];  // turn stepper motor drive back on.
  delay(pause);            // wait a while
  for(i=0;i<nsteps;i++){
    s = pause;
    if((i<t1)||(i>t4))
      s = s1;           // slowest speed
    if((i<t2)||(i>t3))
      s = s2;           // middle speed
    stepMotor(dir);     // step
    delay(s);           // and wait
  }
  PORTD=(PORTD & 0x87); // turn stepper motor drive off

  TTpos = TTpos + dir * nsteps; // update position
  if(TTpos<-6000)       // adjust TTpos if less than -6000
    TTpos=TTpos+12000;
  if(TTpos>=6000)       // adjust TTpos if greater than, or equal to 6000
    TTpos=TTpos-12000;
  // TTpos must now be in the range -6000 to +5999
  #ifdef DEBUG
    Serial.write(" Turntable position is ");
    Serial.print(TTpos);
    Serial.write("\n");
  #endif
  digitalWrite(LED1, LOW);  // Red LED off for stopped
}// end of moveMotor()

// move while hall effect sensor is in a particular state
void moveWhileHall (int dir, int pause, int state){
  int s;
  s=digitalRead(hallPin);
  while(s==state){
    stepMotor(dir);
    delay(pause);
    s=digitalRead(hallPin);
  }
}// end of moveMotor()

// steps motor 1 step. (ANTICLOCK = 1 , CLOCKWISE = -1)
void stepMotor(int step){
  stepPos=stepPos+step;
  if(stepPos>=4)
    stepPos=0;
  else 
  if(stepPos<0)
    stepPos=3;
    // hopefully leave other bits of PORTD as they were.
  PORTD=(PORTD & 0x87) | motorStep[stepPos];
}// end of stepMotor()
