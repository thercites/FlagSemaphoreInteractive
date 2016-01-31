//Servo Connections
// Red     -- vcc
// Brown   -- gnd
// Orange  -- signal

// illegal letters H,I,O,W,X,Z

#include <avr/wdt.h>
#include "LedControl.h"
#include "fonts.h"
#include "ElapsedTime.h"
#include <Servo.h> 

const int touch = 2;
const int touchPower = 3;
const int touchGround = 4;

const int led = 13;
const int servoLt = 9;
const int servoRt = 6;
const int armSpeed = 15;

const int matrixLoad = 10;
const int matrixClock = 11;
const int matrixData = 12;

//const int promptInterval = 60000;
const int displayPause = 2000;

const char *startChars = ".xX.xX.xX";
const char *finishChars = "Vv.Vv.Vv.";

// words to say
const char *wordList[] = { "APPLE", 
                           "PEAR",  
                           "BANANA",
                           "PAWPAW",
                           "PAPYA",
                           "RASPBERRY",
                           "GRAPE",
                           "MULBERRY",
                           "GUAVA",
                           "DATE",
                           "CURRENT",
                           "JAMBUL",
                           "JEJUBE",
                           "ELDERBERRY",
                           "RAMBUTAN",
                           "BLUEBERRY",
                           "CRANBERRY",
                           "KUMQUAT"
                           };
                           
const int wordListLength = 18;

// semaphore signal arm positions 
typedef struct {int left; int right;} signal;
                
const int uu = 180; // up
const int ua = 135; // up angle
const int hh = 90;  // horizontal
const int da = 45;  // down angle
const int dd = 0;   // down

const signal signals[] = { 
                      {da,dd},  //A
                      {hh,dd},  //B
                      {ua,dd},  //C 
                      {uu,dd},  //D
                      {dd,ua},  //E
                      {dd,hh},  //F
                      {dd,da},  //G
                      {uu,hh},  //H (J)
                      {ua,hh},  //I (Y)
                      {uu,hh},  //J
                      {da,uu},  //K
                      {da,ua},  //L
                      {da,hh},  //M
                      {da,da},  //N
                      {hh,ua},  //O (Q)
                      {hh,uu},  //P
                      {hh,ua},  //Q
                      {hh,hh},  //R
                      {hh,da},  //S
                      {ua,uu},  //T
                      {ua,ua},  //U
                      {uu,da},  //V
                      {uu,uu},  //W (?)
                      {uu,uu},  //X (?)
                      {ua,hh},  //Y
                      {uu,uu},  //Z (?)
                      {dd,dd}   // space
                   };

LedControl matrix = LedControl(matrixData,matrixClock,matrixLoad,1);

Servo ltArmServo;
Servo rtArmServo;

ElapsedTime timer;

void setup() {
  pinMode(touchPower, OUTPUT);
  pinMode(touchGround, OUTPUT);
  pinMode(touch, INPUT);
  
  pinMode(led, OUTPUT);
  
  digitalWrite(led, LOW);    

  wakeUpMatrix();
  
  ltArmServo.attach(servoLt); 
  rtArmServo.attach(servoRt);
 
  resetTouchSwitch();
  randomSeed(analogRead(0));
  
  // set the watchdog timer
  wdt_enable(WDTO_8S);
}

void loop() {
  wdt_reset();
  
  if(digitalRead(touch) == HIGH) {
     const char * word;
     // give feedback
     showString(startChars, 250);
     // choose a word
     word = wordList[random(0,wordListLength)];
     // quiz the word
     quizString(word, displayPause);
     // show the word
     showString(word, displayPause);
     restArms();
     matrix.clearDisplay(0);
     delay(displayPause/2);
     showString(finishChars, 250);
     // reset the timer
     timer.startTimer(10000); 
  }
  else {
     // test if time to flash
     if(timer.timeHasElapsed()){
       showString(finishChars, 250);
       timer.startTimer(60000);
     }
  }
}

void wakeUpMatrix(void) {
  matrix.shutdown(0,false);
  delay(50);
  matrix.setIntensity(0,7);
  matrix.clearDisplay(0);
}

void showLetter (const byte c){
  for (byte col = 0; col < 8; col++){
    matrix.setRow(0,col,pgm_read_byte(&cp437_font[c][col]));
  } 
  wdt_reset();
}

void showString (const char * s, const int pause)
{
  char c;
  while(c = *s++){
    showLetter(c); 
    delay(pause);
    showLetter(' ');  
    delay (10);      
  }
} 

void signLetter(const char c){
  moveLtArm(signals[c-'A'].left);
  moveRtArm(signals[c-'A'].right);
}

void quizString (const char *s, const int pause){
  char c;
  while(c = *s++){
    showLetter('?');
    signLetter(c);
    delay(pause);
    showLetter(c);
    delay(pause);
  }
}

void moveLtArm(int pos) {
  digitalWrite(led,HIGH); 
  setRelativeServoPosition(ltArmServo, armSpeed, pos);
  delay(20); 
  digitalWrite(led,LOW);  
}

void moveRtArm(int pos) {
  digitalWrite(led,HIGH);
  setRelativeServoPosition(rtArmServo, armSpeed, (uu-pos));
  delay(20); 
  digitalWrite(led,LOW);  
}

void restArms(void){
  moveLtArm(0);
  moveRtArm(0);
}

void setRelativeServoPosition(Servo & servo, int rate, int pos) {
  int currentPos = servo.read();
  int index = 0;
  
  if(pos < 0) pos = 0;
  if(pos > 180) pos = 180;
  
  if(pos < currentPos) {
    for(index = currentPos; index >= pos; index--) {  
      wdt_reset();
      servo.write(index);
      delay(rate); 
    }  
  }
  else if(pos > currentPos) {
    for(index = currentPos; index <= pos; index++) {
    	  wdt_reset();
      servo.write(index);              
      delay(rate);                        
    }  
  }
}

void resetTouchSwitch(void) {
  digitalWrite(touchPower, LOW);
  delay(50);
  digitalWrite(touchGround, LOW);
  digitalWrite(touchPower, HIGH);
  delay(50); 
}

ISR(WDT_vect){

}
