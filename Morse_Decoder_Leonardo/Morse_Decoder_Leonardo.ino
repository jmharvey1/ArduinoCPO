
/*  
 *   The TFTLCD library comes from here:  https://github.com/prenticedavid/MCUFRIEND_kbv    
 *    note: after inporting the above library using the ide add zip library menu option, remane the folder to remover the word "master"
 ^   The Touch Screen & GFX libraries are standard (unmodified) Adafruit libraries & were found here:4
 *       https://github.com/adafruit/Adafruit-GFX-Library
 *       https://github.com/adafruit/Touch-Screen-Library
*/
 
#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <MCUFRIEND_kbv.h>


#define LCD_CS A3 
#define LCD_CD A2 
#define LCD_WR A1 
#define LCD_RD A0 
#define LCD_RESET A4 

//MCUfriends.com touch screen corner values
#define TS_LEFT 137
#define TS_TOP 156
#define TS_RIGHT 874
#define TS_BOT 855

//MCUfriends.com touch screen pin assignments
#define YP A1  
#define YM 7    
#define XM A2
#define XP 6    

//hamfest touchscreen assignments
//#define YP A2  // must be an analog pin, use "An" notation!
//#define XM A3  // must be an analog pin, use "An" notation!
//#define YM 8   // can be a digital pin
//#define XP 9   // can be a digital pin


//#define YM 7   // can be a digital pin
//#define XP 6   // can be a digital pin

#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

//Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
MCUFRIEND_kbv tft;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#if defined(__AVR_ATmega32U4__)
  //Code in here will only be compiled if an Arduino Leonardo is used.
  
  #define interruptPin 0
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
  //Code in here will only be compiled if an Arduino Uno is used.
  
  #define interruptPin 2
#endif

boolean buttonEnabled = true;
//////////////////////////////////////////////////////////
// This is the INT0 (Interupt 0) Pin of the ATMega8
//const byte interruptPin = 0;
int cnt=0;
int TEcnt=0;
int textSrtX = 0;
int textSrtY = 0;
int scrnHeight;
int scrnWidth;
int LineLen = 27; //max number of characters to be displayed on one line
int row = 7;//7; //max number of lines to be displayed
int curRow = 0;
int offset = 0; 
bool newLineFlag = false;
char Pgbuf[448];
char Msgbuf[32];
char TxtMsg[] = {'T','H','A','N','K',' ','Y','O','U',' ','F','O','R',' ','S','U','B','S','C','R','I','B','I','N','G','!','\n'};
int statsMode =0;
bool chkStatsMode = true;
char TxtMsg1[] = {"1 2 3 4 5 6 7 8 9 10 11\n"};
char TxtMsg2[] = {"This is my 3rd Message\n"};
int msgcntr =0;
int badCodeCnt = 0;
char newLine = '\n'; 
//char TxtMsg[] = {"Thank you for subscribing!\n"};   
// We need to declare the data exchange
// variable to be volatile - the value is
// read from memory.
volatile bool state = LOW;
volatile bool valid = LOW;
volatile bool mark = LOW;
bool dataRdy = LOW;
bool Test = false;
volatile long period =0;
volatile long start=0;
volatile long noSigStrt;
//volatile unsigned long deadSpace =0;

volatile int DeCodeVal;
volatile int CodeValBuf[3];
long avgDit = 80; //average 'Dit' duration
unsigned long avgDah =240; //average 'Dah' duration
long lastDit = avgDit;
int badKeyEvnt =0;
volatile unsigned long avgDeadSpace = avgDit;
volatile int space = 0;
unsigned long lastDah = avgDah;
volatile unsigned long letterBrk =0;//letterBrk =avgDit;
volatile unsigned long wordBrk = avgDah ;
volatile unsigned long wordStrt;
volatile unsigned long deadTime;
volatile unsigned long MaxDeadTime;
volatile bool wordBrkFlg = false;
int charCnt = 0;
float curRatio =3;
int displayW = 320;
int displayH = 320;
int fontH = 16;
int fontW = 12;
int cursorY = 0;
int cursorX = 0;
int wpm =0;
int lastWPM =0;
char morseTbl[]={
  '~', '\0',
  'E', 'T',
  'I', 'A', 'N', 'M',
  'S', 'U', 'R', 'W', 'D', 'K', 'G', 'O',
  'H', 'V', 'F', '_', 'L', '_', 'P', 'J', 'B', 'X', 'C', 'Y', 'Z', 'Q', '_', '_',
  '5', '4', '_', '3', '_', '_', '_', '2',
  '_', '_', '_', '_', '_', '_', '_', '1', '6', '-', '/', '_', '_', '_', '_', '_', '7',
  '_', '_', '_', '8', '_' , '9' ,'0'  
};

// Install the interrupt routine.
void KeyEvntSR() {
  
  if(digitalRead(interruptPin)== LOW) { //key-down
    start = millis();
    letterBrk = 0;
    unsigned long deadSpace = start-noSigStrt;
    if(deadSpace <avgDah & deadSpace> avgDit/2){
      avgDeadSpace = (15*avgDeadSpace+deadSpace)/16;
      }
    if(wordBrkFlg){
      wordBrkFlg = false;
      int thisWordBrk = start-wordStrt;
      if(thisWordBrk < 11*avgDeadSpace){
        wordBrk = (5*wordBrk+ thisWordBrk)/6;
        MaxDeadTime = 0;
        charCnt = 0;
      }
      //else wordBrk = (2*wordBrk+4*avgDeadSpace)/3;
    } else if(charCnt>12){
      if(MaxDeadTime < wordBrk){
        wordBrk = MaxDeadTime;
      }
      MaxDeadTime = 0;
      charCnt = 0;
    }
      //wordBrk = 4*avgDeadSpace ;//wordBrk = 5*avgDit; 

    noSigStrt = 0;
    if(DeCodeVal== 0) {
      DeCodeVal = 1;
      valid = LOW;
    }
    Test = false;//false;//true;
    return;
  }
  else { // "Key Up" evaluations
    if(DeCodeVal!= 0){
      noSigStrt =  millis();
      period = noSigStrt-start;
      
      if(avgDeadSpace> avgDit) space = avgDeadSpace;
      else space= avgDit;
      
      letterBrk = (5*space/3) + noSigStrt;//letterBrk = (5*avgDit/3) + noSigStrt;//letterBrk = (5*avgDeadSpace/3) + noSigStrt;//2*avgDit + noSigStrt;;//letterBrk = 
      start =0;  
    }
    
  }// end of key interupt processing; 
  // Now, if we are here; the interrupt was a "Key-Up" event. So its time to decide whether this "Key-up" event represents a "dit", a "dah", or just garbage.
  // 1st check. & throw out key-up events that have durations that represent speeds of less than 5WPM.
  if(period >720){ //Reset, and wait for the next key closure
    period = 0;
    noSigStrt =0;
    DeCodeVal = 0;
    return; // overly long key closure; Go back look for a new set of events 
  }
   //test to determine that this is a significant signal (key closure duration) event, & warrants evaluation as a dit or dah
   if((float)period >0.3*(float)avgDit){// if "true", we do have a usable event
    badKeyEvnt = 20;
    DeCodeVal = DeCodeVal<<1; //shift the current decode value left one place to make room for the next bit.
    if(period >=1.5*avgDit){    // it smells like a "Dah".
       if(period >= lastDah/2){
         DeCodeVal =DeCodeVal+1;// it appears to be a "dah' so set the least significant bit to "one"
         lastDah = period;
         CalcAvgDah(lastDah);  // Normal pathway to detect a Dah/Dash
       }
       else{ //apparently it was meant to be a 'dit'; leave the least significant bit as 'zero'
        lastDit = period; //save this period for comparison purposes against next key closure
        //period = period/3;
        if( DeCodeVal != 2){ // don't recalculated speed based on a single dit (it could have been noise)
          period = CalcAvgPrd(period);
        }
        //noSigStrt =0;
       }
    }
    else if(period <lastDit/2 & DeCodeVal>=4){//if true, this is an unusual event. 
      //we might have missinturpeted the previous key closure, & probably need to back up, and re-evaluate the previous bit before adding the current bit to
      //the decodeval.
      DeCodeVal = DeCodeVal>>1;
      DeCodeVal =DeCodeVal+1;
      DeCodeVal = DeCodeVal<<1;
      lastDit = period;
      lastDah = 3*avgDit; // recalibrate last dah interval, in prepration to evaluate next key closure
      period = CalcAvgPrd(period);
      mark = true;
      period =0;
      return;
      }
      else{ // if(period >= 0.5*avgDit)
        lastDit = period;
        
        if(DeCodeVal != 2){ // don't recalculate speed based on a single dit (it could have been noise) ||(curRatio > 5.0 & period> avgDit )
          period = CalcAvgPrd(period);
        }
      }
    period = 0;  
    return; // ok, we are done with evaluating this usable key event
   }
   else{
    // if here, this was an atypical event, & may indicate a need to recalculate the average period.
    if(period >0){//this is to correct for when the wpm speed as been slow & now needs to speed by a significant amount
      ++badKeyEvnt;
      if(badKeyEvnt >=20){
        period = CalcAvgPrd(period);
        period =0;
        badKeyEvnt=0;
        noSigStrt =0;
        letterBrk = 0;
       }  
    }
//    if(Test) Serial.println(DeCodeVal); //we didn't experience a full keyclosure event on this pass through the loop [this is normal]
   }
   
  
}// End Interupt routine





/////////////////////////////////////////////////////////////

void setup() {

  
  Serial.begin(9600);
  Serial.print("Starting...");
  
  tft.reset();
  //uint16_t ID = tft.readID();
  //tft.begin(ID);
  tft.begin(0x4747);//MCUfriends.com Display id
  //tft.begin(0x9341);//Hamfest display//The value here is screen specific & depends on the chipset used to drive the screen,
  // in this case 0x9341 = ILI9341 LCD driver
  tft.setRotation(1);
  
  tft.fillScreen(BLACK);
  DrawButton();

  //Draw white frame
  //tft.drawRect(0,0,319,240,WHITE);
 tft.setCursor(textSrtX,textSrtY);
 tft.setTextColor(WHITE);//tft.setTextColor(WHITE, BLACK);
 tft.setTextSize(2);
 tft.setTextWrap(false);
 enableINT();
 start=0;
 scrnHeight = tft.height();
 scrnWidth = tft.width();
 Serial.print("tft.width = "); Serial.print(scrnWidth);
 Serial.print("\tft.height = "); Serial.print(scrnHeight);
 Serial.print("\n");
 wordBrk = 4*avgDeadSpace; 
}//end of SetUp

void loop() 
{
  TSPoint p = ts.getPoint();  //Get touch point
  if (p.z > ts.pressureThreshhold) { // if (p.z > MINPRESSURE && p.z < MAXPRESSURE) { //
     p.y = map(p.y, TS_LEFT, TS_RIGHT, 0, scrnWidth);
     p.x = map(p.x, TS_TOP, TS_BOT, 0, scrnHeight);
     Serial.print("p = "); Serial.print(p.z);
     Serial.print(";\t X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\n");
  }
  buttonEnabled =true;
  if(p.x>217 && p.x<250 && p.y>123 && p.y<207 && buttonEnabled)// The user has touched a point inside the red rectangle, & wants to clear the Display
   {//reset (clear) the SScreen 
     buttonEnabled = false; //Disable button
//   Serial.print("Clear Screen");
     if( interruptPin == 2) KillINT();
     else enableDisplay();
     tft.fillScreen(BLACK);
     DrawButton();
     avgDit =80; //average 'Dit' duration
     avgDah =240;
     wpm = CalcWPM(avgDit);
     showSpeed();
     tft.setCursor(textSrtX,textSrtY);
    if( interruptPin == 2) enableINT();
     for( int i = 0; i <  sizeof(Pgbuf);  ++i )
         Pgbuf[i] = 0;
    cnt=0;
    curRow = 0;
    offset = 0;
    cursorX = 0;
    cursorY =0;     
       
   }
   if(p.x>210 && p.x<248 && p.y>2 && p.y<114 &chkStatsMode ){
    chkStatsMode = false;
    // The user has touched a point inside stats area & wants to change the Stats mode
    if(statsMode ==0) statsMode =1;
    else statsMode =0;
   }

//   if(Test){
//     Serial.print(avgDit);
//     Serial.print(";  ");
//     Serial.print(lastDit);
//     Serial.print(";  ");
//     Serial.print(lastDah);
//     Serial.print(";  ");
//     if(start>0)Serial.print(millis()-start);
//     else Serial.print(period);
//     Serial.print(";  ");
//     if(noSigStrt>0)Serial.print(millis()-noSigStrt);
//     else Serial.print("0");
//     Serial.print(";  ");
//   }
   chkChrCmplt();  
   while(CodeValBuf[0]>0){
    DisplayChar(CodeValBuf[0]);
    //Now check & see if enough time has lapsed since the last key closure to signify that the word is complete
    
   }

  
  }//end of Main Loop

  //////////////////////////////////////////////////////////////////////

  
void chkChrCmplt(){
  int state =0;
  unsigned long now = millis();
  //check to see if enough time has passed since the last key closure to signify that the character is complete
   if((now >= letterBrk) & letterBrk !=0 & DeCodeVal>1) state =1;
   float noKeySig = (float)(now - noSigStrt);
   if((noKeySig >= 0.75*((float)wordBrk) ) & noSigStrt !=0 & !wordBrkFlg){
      state = 2;
      wordStrt = noSigStrt;
      if( DeCodeVal == 0){
        noSigStrt =0;// this should always be "true"; something weird happened if not true
        MaxDeadTime =0;
        charCnt = 0;
      }
      wordBrkFlg = true;
      Test = false;
   }

   
 if(!state){
  if ((unsigned long)noKeySig > MaxDeadTime & noKeySig < 7*avgDeadSpace ) MaxDeadTime = (unsigned long)noKeySig;
  return;
 }
 else{
     int i =0;
     while(CodeValBuf[i]>0) ++i;// move buffer pointer to 1st available empty aray position
     if(state==1){
      CodeValBuf[i] = DeCodeVal;
      letterBrk =0;
      ++charCnt;
      DeCodeVal = 0; // make program ready to process next series of key events 
      period = 0;//     before attemping to display the current code value
     }
     else CodeValBuf[i] = 255;
  }
}
  //////////////////////////////////////////////////////////////////////

  
int CalcAvgPrd(int thisdur){
      
      if(thisdur> 3.4*avgDit) thisdur = 3.4*avgDit; //limit the effect that a single sustained "keydown" rvrnt cant have
      if(thisdur>= 2.5*avgDit){
        CalcAvgDah(thisdur);
        curRatio = (float)avgDah/(float)avgDit;//avgDit = avgDah/curRatio;
        
      }
      else{ // this appears to be a "dit"
        avgDit = (9*avgDit+ thisdur)/10;
        curRatio = (float)avgDah/(float)avgDit;
      }
        
      if(avgDit > 1000) avgDit = 1000;
      if(avgDit <30) avgDit = 30;
      if(DeCodeVal ==1) DeCodeVal =0;
//      if(Test){
//        Serial.print(DeCodeVal);
//        Serial.print(";  "); 
//        Serial.println("Valid");
//      }
      thisdur = 0;
      return thisdur; 
    }
  
  /////////////////////////////////////////////////////////////////////

int CalcWPM(int dotPeriod){
  int codeSpeed =1200/dotPeriod;
  return codeSpeed;  
}

///////////////////////////////////////////////////////////////////////

void CalcAvgDah(int thisPeriod){
   avgDah = (((3*avgDah)+ thisPeriod)/4);
}
//////////////////////////////////////////////////////////////////////

void DisplayChar(int decodeval){
    char curChr = 0 ;
    //CodeValBuf =0;
    //if(decodeval !=0){
      for(int i=1; i< 3; i++) { 
        CodeValBuf[i-1] = CodeValBuf[i]; 
      }
      CodeValBuf[2] = 0;
    //}
    if(decodeval ==2 || decodeval ==3) ++TEcnt;
    else TEcnt = 0;
    if(Test) Serial.println(decodeval);
    //clear buffer
    for( int i = 0; i < sizeof(Msgbuf);  ++i )
       Msgbuf[i] = 0;
    // check for special case [Text]
    if (decodeval== 42) sprintf ( Msgbuf,"<AR> " );
    else if (decodeval== 899) sprintf ( Msgbuf,"<73> " );
    else if (decodeval== 40) sprintf ( Msgbuf,"<AS> " );
    else if (decodeval== 69) sprintf ( Msgbuf,"<SK> " );
    else if (decodeval== 256+173) sprintf ( Msgbuf,"CQ " );
    else if (decodeval== 54) sprintf ( Msgbuf,"<KN>" );   
    else if (decodeval<64) curChr =morseTbl[decodeval];
    else{
       switch (decodeval) {
      case 85 :
         curChr ='.';
         break;
      case 115 :
         curChr =',';
         break;
      case 76 :
         curChr ='?';
         break;
      case 255 :
         curChr =' ';
         break;   
     default: 
      curChr ='*';
      break;
    break;              
     }
      
    }
    if (curChr == 'E' || curChr == 'T') ++badCodeCnt; 
    else badCodeCnt = 0;  
    if(badCodeCnt > 5 & wpm >25) avgDit = 1200/15;
    if (((cnt)-offset)*fontW >= displayW){;//if ((cnt -(curRow*LineLen) == LineLen)){
      curRow++;
      offset = cnt;
      cursorX = 0;
      cursorY = curRow*(fontH+10);
      tft.setCursor(cursorX, cursorY);
      //offset = (curRow*LineLen);
      //while(cnt <offset) cnt++;
    if (curRow+1 > row){// its time to Scroll the Text up one line
      //buttonEnabled =false;
      if(Test)Serial.print("ReDraw Frame\n");
      curRow =0;
      cursorX = 0;
      cursorY = 0;
      cnt=0;
      offset =0;
     if( interruptPin == 2) KillINT();
     else enableDisplay();
      tft.fillRect(cursorX, cursorY, displayW, row*(fontH+10), BLACK);//erase current page of text
      tft.setCursor(cursorX, cursorY);
      while(Pgbuf[cnt] !=0 & curRow+1<row){//print current page buffer and move current text up one line 
        tft.print(Pgbuf[cnt]);
        Pgbuf[cnt] = Pgbuf[cnt+27];//shift existing text character forward by one line
        cnt++;
        //delay(300);
        if(((cnt)-offset)*fontW >= displayW){
          curRow++;
          offset = cnt;
          cursorX = 0;
          cursorY = curRow*(fontH+10);
          tft.setCursor(cursorX, cursorY);
        }
        else cursorX = (cnt-offset)*fontW;
        chkChrCmplt();
      }//end While Loop
      while(Pgbuf[cnt] !=0){//finish cleaning up last line
        //tft.print(Pgbuf[cnt]); 
        Pgbuf[cnt] = Pgbuf[cnt+26];
        cnt++;
      }
    }
   }
   sprintf ( Msgbuf,"%s%c", Msgbuf,curChr );
   int msgpntr = 0;
   if( interruptPin == 2) KillINT();
   else enableDisplay();
   tft.setCursor(cursorX, cursorY);
   while( Msgbuf[msgpntr] !=0){
    char curChar = Msgbuf[msgpntr];
    if(curRow>0) sprintf ( Pgbuf,"%s%c", Pgbuf, curChar);
    tft.print(curChar);
    msgpntr++;
    cnt++;
    cursorX = (cnt-offset)*fontW; 
   }
   wpm = CalcWPM(avgDah/3);
   if(wpm != 1){//if(wpm != lastWPM & decodeval == 0 & wpm <36){//wpm != lastWPM
    lastWPM = wpm;
    showSpeed();
    if(TEcnt> 7 &  curRatio > 4.5){ // if true, we probably not waiting long enough to start the decode process, so reset the dot dash ratio based o current dash times
     avgDit = avgDah/3;   
    }
   }
   
   if( interruptPin == 2) enableINT();
  }
//////////////////////////////////////////////////////////////////////

void KillINT(){
  detachInterrupt(digitalPinToInterrupt(interruptPin));
  pinMode(interruptPin, OUTPUT);
  //This is important, because the libraries are sharing pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
}

void enableINT(){
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), KeyEvntSR, CHANGE);
  // pinMode(XM, INPUT);
  //pinMode(YP, INPUT);
}

void enableDisplay(){
  //This is important, because the libraries are sharing pins
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
}



void DrawButton(){
    //Create Red Button
    int posx = 130;
    int width = 80;
    int posy = 195;
    int height = 40;
  tft.fillRect(posx,posy, width, height, BLUE);
  tft.drawRect(posx,posy, width, height,WHITE);
  tft.setCursor(posx+11,posy+12);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Clear");

}
///////////////////////////////////////////////////////////

void showSpeed(){
  char buf[20];
  int ratioInt = (int)curRatio;
  int ratioDecml = (int)((curRatio-ratioInt)*10);
  chkStatsMode = true; 
if(statsMode ==0){
  //  sprintf ( buf,"%d/%d.%d/%d", wpm, ratioInt, ratioDecml, avgDeadSpace );
  sprintf ( buf,"%d/%d.%d WPM", wpm, ratioInt, ratioDecml);
  //  sprintf ( buf,"%d",wordBrk);
}
else{
    sprintf ( buf,"%d",avgDit);
    sprintf ( buf,"%s/%d",buf,avgDah);
    sprintf ( buf,"%s/%d",buf,avgDeadSpace);
}

  tft.setCursor(0, 8*(fontH+10));
  tft.fillRect(0 , 8*(fontH+10), 11*fontW, fontH+10, BLACK);//tft.fillRect(0 , 8*(fontH+10), 10*fontW, fontH+10, BLACK);
  tft.print(buf);
//  if(!mark) tft.print(buf);
//  else{
//     mark = false;
//     tft.print("XXXXXX");
//  }
}

