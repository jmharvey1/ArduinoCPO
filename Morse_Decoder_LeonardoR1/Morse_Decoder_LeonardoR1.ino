/* Rev: 2019-06-20 */
/* Rev: 2018-02-13 */
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
// This is the INT0 (Interrupt 0) Pin of the ATMega8
//const byte interruptPin = 0;
int cnt=0;
int btnPrsdCnt =0;
int TEcnt=0;
int textSrtX = 0;
int textSrtY = 0;
int scrnHeight =0;
int scrnWidth;
int LineLen = 27; //max number of characters to be displayed on one line
int row = 7;//7; //max number of lines to be displayed
int xbhi =250; //button touch reading 2.5" screen
int xblo = 217; //button touch reading 2.5" screen
int ybc1  = 208; //button touch reading 2.5" screen
int ybc2  = ybc1; //button touch reading 2.5" screen
int yblo = 123; //button touch reading 2.5" screen
int ybhi = 300; //button touch reading 2.5" screen
int Stxh = 248; //Status touch reading 2.5" screen
int Stxl = 210; //Status touch reading 2.5" screen
int Styh = 114; //Status touch reading 2.5" screen
int Styl = 2; //Status touch reading 2.5" screen
int curRow = 0; //Status touch reading 2.5" screen
int offset = 0; //Status touch reading 2.5" screen 
bool newLineFlag = false;
char Pgbuf[448];
char Msgbuf[32];
char TxtMsg[] = {'T','H','A','N','K',' ','Y','O','U',' ','F','O','R',' ','S','U','B','S','C','R','I','B','I','N','G','!','\n'};
int statsMode =0;
bool chkStatsMode = true;
bool SwMode = false;
bool BugMode = false;//true;//
bool Bug2 = false;//false;//
int ModeCnt = 0; // used to cycle thru the 3 decode modes (norm, bug, bug2)
char TxtMsg1[] = {"1 2 3 4 5 6 7 8 9 10 11\n"};
char TxtMsg2[] = {"This is my 3rd Message\n"};
int msgcntr =0;
int badCodeCnt = 0;
char newLine = '\n'; 

//volatile bool state = LOW;
volatile bool valid = LOW;
volatile bool mark = LOW;
bool dataRdy = LOW;
bool Test = true;//false;
volatile long period =0;
volatile long start=0;
volatile unsigned long thisWordBrk =0;
//volatile long Oldstart=0;//JMH 20190620
volatile long noSigStrt;
//volatile long OldnoSigStrt;//JMH 20190620
//bool ignoreKyUp = false;//JMH 20190620
//bool ignoreKyDwn = false;//JMH 20190620
//volatile unsigned long deadSpace =0;
volatile int TimeDat[8];
int Bitpos = 0;
volatile int TimeDatBuf[24];
int TDBptr =0;
volatile int DeCodeVal;
volatile int OldDeCodeVal;
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
//int displayH = 320;
int fontH = 16;
int fontW = 12;
int cursorY = 0;
int cursorX = 0;
int wpm =0;
int lastWPM =0;
int state =0;
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
  chkChrCmplt();  
  if(digitalRead(interruptPin)== LOW) { //key-down
    start = millis();
    unsigned long deadSpace = start-noSigStrt;
    letterBrk = 0;
    if(Bug2){
      //Serial.println("Path 1");
      if(deadSpace <avgDit & deadSpace> avgDit/4){
        avgDeadSpace = (15*avgDeadSpace+deadSpace)/16;
      }      
    }
    else{
      if(deadSpace <avgDah & deadSpace> avgDit/2){ //this dead space interval appears to be an inter-character event
        //Serial.println("Path 2");
        avgDeadSpace = (15*avgDeadSpace+deadSpace)/16;
        if(ModeCnt ==0 & (avgDeadSpace < avgDit)){ // running Normall mode; use Dit timing to establish minmum "space" interval 
          avgDeadSpace = avgDit;
          //Serial.println("Path 3");
        }
      }
    }
    
    if(wordBrkFlg){
      wordBrkFlg = false;
      thisWordBrk = start-wordStrt;
      //Serial.println(thisWordBrk);
      if(thisWordBrk < 11*avgDeadSpace){
        wordBrk = (5*wordBrk+ thisWordBrk)/6;
        MaxDeadTime = 0;
        charCnt = 0;
        //Serial.println(wordBrk);
      }
    } else if(charCnt>12){
      if(MaxDeadTime < wordBrk){
        wordBrk = MaxDeadTime;
        //Serial.println(wordBrk);
      }
      MaxDeadTime = 0;
      charCnt = 0;
    }
    noSigStrt =  millis();//jmh20190717added to prevent an absurd value
    if(DeCodeVal== 0) {
      DeCodeVal = 1;
      valid = LOW;
    }
    //Test = false;//false;//true;
    return;
  }
  else { // "Key Up" evaluations; Time to decide whether this was a "Dit" or a "Dah"
    if(DeCodeVal!= 0){
      noSigStrt =  millis();
      period = noSigStrt-start;
      TimeDat[Bitpos] = period;
      Bitpos +=1;
      if(Bug2){
        space = ((3*space)+avgDeadSpace)/4;
      }
      else{
        if(avgDeadSpace> avgDit) space = ((3*space)+avgDeadSpace)/4; //20190717 jmh - Changed to averaging space value to reduce chance of glitches causing mid character letter breaks 
        else space=  ((3*space)+avgDit)/4; //20190717 jmh - Changed to averaging space value to reduce chance of glitches causing mid character letter breaks
       }
      long ltrBrk = 0; 
      if (BugMode){
       ltrBrk = (6*avgDeadSpace)/3;
//       Serial.print(space);
//       Serial.print(";  ");
//       Serial.println(ltrBrk);   
      }
      else ltrBrk = long(5.0*(float(space)/3.0)); 
      //Serial.print(ltrBrk);
      //Serial.print(";");
      letterBrk = ltrBrk + noSigStrt;//letterBrk = (5*avgDit/3) + noSigStrt;//letterBrk = (5*avgDeadSpace/3) + noSigStrt;//2*avgDit + noSigStrt;;//letterBrk = 
      
      start =0;  
    }
    
  }// end of key interrupt processing; 
  // Now, if we are here; the interrupt was a "Key-Up" event. So its time to decide whether this "Key-up" event represents a "dit", a "dah", or just garbage.
  // 1st check. & throw out key-up events that have durations that represent speeds of less than 5WPM.
  if(period >720){ //Reset, and wait for the next key closure
    period = 0;
    //noSigStrt =0;
    noSigStrt =  millis();//jmh20190717added to prevent an absurd value
    
    DeCodeVal = 0;
    return; // overly long key closure; Go back look for a new set of events 
  }
   //test to determine that this is a significant signal (key closure duration) event, & warrants evaluation as a dit or dah
   if((float)period >0.3*(float)avgDit){// if "true", we do have a usable event
    badKeyEvnt = 20;
    DeCodeVal = DeCodeVal<<1; //shift the current decode value left one place to make room for the next bit.
    if(period >=1.5*avgDit){    // it smells like a "Dah".
       if((period >= lastDah/2) || BugMode ){
         DeCodeVal =DeCodeVal+1;// it appears to be a "dah' so set the least significant bit to "one"
         if(BugMode) letterBrk = letterBrk + 0.8*avgDit ;
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
      //Serial.print("*"); 
      //we might have missinturpeted the previous key closure, & probably need to back up, and re-evaluate the previous bit before adding a "0" bit to
      //the decodeval.
      DeCodeVal = DeCodeVal>>1;// back  up one bit
      DeCodeVal = DeCodeVal|1; //set the previous key closure to be a "dah"; this may take more thought than this simple logic 
      DeCodeVal = DeCodeVal<<1;// now set this bit to be a "dit"
      lastDit = period;
      lastDah = 3*avgDit; // recalibrate last dah interval, in prepration to evaluate next key closure
      period = CalcAvgPrd(period);
      mark = true;
      period =0;
      return;
    }
    else{ // if(period >= 0.5*avgDit)
      lastDit = period;
      if(DeCodeVal != 2 ){ // don't recalculate speed based on a single dit (it could have been noise)or a single dah ||(curRatio > 5.0 & period> avgDit )
        period = CalcAvgPrd(period);
      }
    }
    period = 0;  
    return; // ok, we are done with evaluating this usable key event
   }
   else{
    // if here, this was an atypical event, & may indicate a need to recalculate the average period.
    //Serial.print("*");
    //Serial.print(period);
    //Serial.print("*");
    if(period >0){//this too is correct for when the wpm speed as been slow & now needs to speed by a significant amount
      ++badKeyEvnt;
      //Serial.print("*");
      if(badKeyEvnt >=20){
        //Serial.print("*");
        period = CalcAvgPrd(period);
        period =0;
        badKeyEvnt=0;
        //noSigStrt =0;
        noSigStrt =  millis();//jmh20190717added to prevent an absurd value
        letterBrk = 0;
       }  
    }
//    if(Test) Serial.println(DeCodeVal); //we didn't experience a full keyclosure event on this pass through the loop [this is normal]
   }
   
  
}// End Interrupt routine





/////////////////////////////////////////////////////////////

void setup() {

  
  Serial.begin(9600);
  //if (!Serial) delay(2500);           //allow some time for Leonardo
  
  
  tft.reset();
  uint16_t ID = tft.readID();
//  tft.begin(0x6814);//MCUfriends.com 3.5" Display id 
  //tft.begin(0x9090);//MCUfriends.com 3.5" Display id (Black Letters on White Screen)
  tft.begin(0x4747);//MCUfriends.com 2.8" Display id
  //tft.begin(0x9341);//Hamfest display//The value here is screen specific & depends on the chipset used to drive the screen,
  // in this case 0x9341 = ILI9341 LCD driver
  tft.setRotation(1);
  tft.fillScreen(BLACK);
  scrnHeight = tft.height();
  scrnWidth = tft.width();
  displayW = scrnWidth; 
  if (scrnHeight == 320){ //we are using a 3.5 inch screen
    row = 10; //max number of decoded text rows that can be displayed
    xbhi = 325; //button touch reading 3.5" screen
    xblo = 300; //button touch reading 3.5" screen
    ybc1  = 356; //button touch reading 3.5" screen
    ybc2  = 180;
    yblo = 270; //button touch reading 3.5" screen
    ybhi = 270; //button touch reading 3.5" screen
    Stxh = 336; //Status touch reading 3.5" screen
    Stxl = 300; //Status touch reading 3.5" screen
    Styh = 521; //Status touch reading 3.5" screen
    Styl = 369; //Status touch reading 3.5" screen
  }

  DrawButton();
  Button2();
  tft.setCursor(textSrtX,textSrtY);
  tft.setTextColor(WHITE);//tft.setTextColor(WHITE, BLACK);
  tft.setTextSize(2);
  tft.setTextWrap(false);
  enableINT();
  start=0;
  Serial.print("Starting...");
//  Serial.print(F("tft.readID() finds: ID = 0x"));
//  Serial.println(ID, HEX);
//  
//  Serial.print("tft.width = "); Serial.print(scrnWidth);
//  Serial.print("\tft.height = "); Serial.print(scrnHeight);
//  Serial.print("\n");
  if (scrnHeight == 320){ //we are using a 3.5 inch screen
    dispMsg("             KW4KD (20190909)           ");
  }
  else{
    dispMsg("      KW4KD (20190909)     ");
  }
  wordBrk = ((5*wordBrk)+4*avgDeadSpace)/6; 
}//end of SetUp

void loop() 
{
  TSPoint p = ts.getPoint();  //Get touch point
  if (p.z > ts.pressureThreshhold) { // if (p.z > MINPRESSURE && p.z < MAXPRESSURE) { //
     p.y = map(p.y, TS_LEFT, TS_RIGHT, 0, scrnWidth);
     p.x = map(p.x, TS_TOP, TS_BOT, 0, scrnHeight);
//     Serial.print("p = "); Serial.print(p.z);
//     Serial.print(";\t X = "); Serial.print(p.x);
//     Serial.print("\tY = "); Serial.print(p.y);
//     Serial.print("\n");
  }else{
    buttonEnabled =true;
    if(!SwMode) ++btnPrsdCnt;
    if(btnPrsdCnt >100){
      SwMode = true;
      //buttonEnabled = false;
      btnPrsdCnt = 0;
    }
   
  }
  if(!buttonEnabled)Serial.println("buttonEnabled = False");
//  int xbhi =250
//  int xblo = 217
//  int ybc  = 208
//  int yblo = 123
//  int ybhi = 300
  if(p.x>xblo && p.x<xbhi && p.y>yblo && p.y<ybc1 && buttonEnabled)// The user has touched a point inside the Blue rectangle, & wants to clear the Display
   {//reset (clear) the SScreen 
     buttonEnabled = false; //Disable button
//   Serial.print("Clear Screen");
     if( interruptPin == 2) KillINT();
     else enableDisplay();
     tft.fillScreen(BLACK);
     DrawButton();
     Button2();
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
       
   }else if(p.x>xblo && p.x<xbhi && p.y>ybc2 && p.y<ybhi && buttonEnabled){
     btnPrsdCnt = 0;
     if (SwMode){
      SwMode = false;
      ModeCnt +=1;
      if (ModeCnt==3) ModeCnt = 0;
      switch (ModeCnt){
        case 0:
          BugMode = false;
          Bug2 = false;
          break;
        case 1:
          BugMode = true;
          break;
        case 2:
          BugMode = false;
          Bug2 = true;
          break;     
      }
//      Serial.println("Norm/Bug Button");
      if( interruptPin == 2) KillINT();
      else enableDisplay();
      Button2();
      if( interruptPin == 2) enableINT();
    }
    
      //Serial.println("Norm/Bug Button");
      //BugMode = !BugMode;
      //Button2();
      //tft.setCursor(cursorX, cursorY);   
   }
    
  
   //if(p.x>210 && p.x<248 && p.y>2 && p.y<114 &chkStatsMode ){
   //if(p.x>300 && p.x<336 && p.y>369 && p.y<521 &chkStatsMode ){
   if(p.x>Stxl && p.x<Stxh && p.y>Styl && p.y<Styh &chkStatsMode ){ 
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
    
   }

  
  }//end of Main Loop

  //////////////////////////////////////////////////////////////////////

  
void chkChrCmplt(){
  state =0;
  unsigned long now = millis();
  //check to see if enough time has passed since the last key closure to signify that the character is complete
  if((now >= letterBrk) & letterBrk !=0 & DeCodeVal>1){
    state =1; //have a complete letter
    //Serial.println(now-letterBrk);
    //Serial.println(letterBrk);
  }
  float noKeySig = (float)(now - noSigStrt);
  if((noKeySig >= 0.75*((float)wordBrk) ) & noSigStrt !=0 & !wordBrkFlg & (DeCodeVal == 0)){
      //Serial.print(wordBrk);
      //Serial.print("\t");
      state = 2;//have word
      
      wordStrt = noSigStrt;
      if( DeCodeVal == 0){
        noSigStrt =0;// this should always be "true"; something weird happened if not true
        noSigStrt =  millis();//jmh20190717added to prevent an absurd value
        MaxDeadTime =0;
        charCnt = 0;
      }
      wordBrkFlg = true;
      //Test = false;
  }
  
  // for testing only
//  if(OldDeCodeVal!=DeCodeVal){
//    OldDeCodeVal=DeCodeVal;
//    Serial.print(DeCodeVal);
//    Serial.print("; ");
//  }
  
  if(state==0){
    if ((unsigned long)noKeySig > MaxDeadTime & noKeySig < 7*avgDeadSpace ) MaxDeadTime = (unsigned long)noKeySig;
    return;
  }
  else{
    //Serial.println(state);
    if(state>=1){//if(state==1){
      
      if(DeCodeVal>=2){
        int i =0;
        while(CodeValBuf[i]>0) ++i;// move buffer pointer to 1st available empty array position
        CodeValBuf[i] = DeCodeVal;
        for(int p =0;  p<Bitpos; p++ ){ // map timing info into time buffer (used only for debugging
           TimeDatBuf[p]= TimeDat[p];
           TimeDat[p] =0;// clear out old time data
        }
      }
//      Serial.print("DeCodeVal = "); Serial.println(DeCodeVal);
      if(state==2){
        int i =0;
        while(CodeValBuf[i]>0) ++i;// move buffer pointer to 1st available empty array position
        CodeValBuf[i] = 255;
      }  
      TDBptr = Bitpos;
      Bitpos = 0;
      
      letterBrk =0;
      ++charCnt;
      DeCodeVal = 0; // make program ready to process next series of key events 
      period = 0;//     before attemping to display the current code value
    }
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
  //if(DeCodeVal == 3) return;//{ //don't recalculated speed based on a single "dah"
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
    if(Test) Serial.print(decodeval);
    if(Test) Serial.print("\t");
    //clear buffer
    for( int i = 0; i < sizeof(Msgbuf);  ++i )
       Msgbuf[i] = 0;
    // check for Dictionary for special case [Text]
    if (decodeval== 42) sprintf ( Msgbuf,"<AR> " );
    else if (decodeval== 31) sprintf ( Msgbuf,"TO" );// could also be "OT" needs further evaluation
    else if (decodeval== 34) sprintf ( Msgbuf,"VE" );
    else if (decodeval== 38) sprintf ( Msgbuf,"UN" );
    else if (decodeval== 899) sprintf ( Msgbuf,"<73> " );
    else if (decodeval== 40) sprintf ( Msgbuf,"<AS> " );
    else if (decodeval== 44) sprintf ( Msgbuf,"AD" );
    else if (decodeval== 45) sprintf ( Msgbuf,"WA" );
    else if (decodeval== 52) sprintf ( Msgbuf,"TL" );
    else if (decodeval== 54) sprintf ( Msgbuf,"<KN>" );
    else if (decodeval== 69) sprintf ( Msgbuf,"<SK> " );
    else if (decodeval== 256+173) sprintf ( Msgbuf,"CQ " );
    
     
    //Single character Text values  
    else if (decodeval<64){
      curChr =morseTbl[decodeval];
      // for debugging: 
//      if (curChr =='_'){ // for debugging 
//        curChr =0;
//        sprintf( Msgbuf,"[%d]",decodeval );
//        for(int i = 0; i<TDBptr; i++){
//          sprintf( Msgbuf, "%s %d, ",Msgbuf, TimeDatBuf[i]);
//        }
//      }// end if
    }
    else{
      switch (decodeval) {
         case 64 :
          sprintf ( Msgbuf,"HE" );
          break;
        case 70 :
          sprintf ( Msgbuf,"SG" );
          break;
        case 72 :
          sprintf ( Msgbuf,"US" );
          break;  
        case 74 :
          sprintf ( Msgbuf,"UR" );
          break;  
        case 76 :
          curChr ='?';
          break;
        case 78 :
          curChr ='UG';
          break;  
        case 80 :
          sprintf ( Msgbuf,"RS" );
          break;  
        case 81 :
          sprintf ( Msgbuf,"AV" );
          break;  
        case 82 :
          sprintf ( Msgbuf,"AF" );
          break;
        case 84 :
          sprintf ( Msgbuf,"AL" );
          break;  
        case 85 :
          curChr ='.';
          break;
         case 86 :
          sprintf ( Msgbuf,"AP" );//sprintf ( Msgbuf,"RW" );
          break;
        case 89 :
          sprintf ( Msgbuf,"PA" );
          break;
        case 91 :
          curChr = ( Msgbuf,"AY" );
          break;      
        case 92 :
          curChr = ( Msgbuf,"AMI" );
          break;  
        case 94 :
          curChr = 39;//"'"
          break;
        case 96 :
          sprintf ( Msgbuf,"THE" );
          break;
        case 105 :
         sprintf ( Msgbuf,"CA" );
          break;  
        case 110 :
          sprintf ( Msgbuf,"TAG" );
          break;
        case 113 :
          sprintf ( Msgbuf,"GU" );
          break;
        case 114 :
          sprintf ( Msgbuf,"GR" );
          break;          
        case 115 :
          curChr =',';
          break;
        case 116 :
          sprintf ( Msgbuf,"MAI" );
          break;
        case 118 :
          sprintf ( Msgbuf,"MP" );
          break;    
        case 120 :
          sprintf ( Msgbuf,"OS" );
          break;  
        case 121 :
          sprintf ( Msgbuf,"OU" );
          break;
        case 122 :
          sprintf ( Msgbuf,"OR" );
          break;
       case 123 :
          sprintf ( Msgbuf,"OW" );
          break;   
       case 125 :
          sprintf ( Msgbuf,"OK" );
          break;   
       case 126 :
          sprintf ( Msgbuf,"OME" );
          break;
       case 127 :
          sprintf ( Msgbuf,"TOO" );
          break;
       case 145 :
          sprintf ( Msgbuf,"FU" );
          break;   
       case 146 :
          sprintf ( Msgbuf,"UF" );
          break;   
       case 148 :
          sprintf ( Msgbuf,"UL" );
          break;         
       case 150 :
          sprintf ( Msgbuf,"UP" );
          break;
       case 162 :
          sprintf ( Msgbuf,"AVE" );
          break;
       case 176 :
          sprintf ( Msgbuf,"WH" );
          break;
       case 178 :
          sprintf ( Msgbuf,"PR" );
          break;
       case 192 :
          sprintf ( Msgbuf,"THE" );
          break;
       case 209 :
          sprintf ( Msgbuf,"CU" );
          break;   
       case 211 :
          sprintf ( Msgbuf,"CW" );
          break;                  
       case 212 :
          sprintf ( Msgbuf,"AL" );
          break;
       case 216 :
          sprintf ( Msgbuf,"YS" );
          break;
       case 232 :
          sprintf ( Msgbuf,"QS" );
          break;      
       case 234 :
          sprintf ( Msgbuf,"QR" );
          break;          
       case 242 :
          sprintf ( Msgbuf,"OF" );
          break;
       case 243 :
          sprintf ( Msgbuf,"OUT" );
          break;     
       case 244 :
          sprintf ( Msgbuf,"OL" );
          break;  
       case 246 :
          sprintf ( Msgbuf,"OP" );
          break;
       case 248 :
          sprintf ( Msgbuf,"OB" );
          break;
       case 255 :
          curChr =' ';
          break;
       case 283 :
          sprintf ( Msgbuf,"VY" );   
       case 296 :
          sprintf ( Msgbuf,"FB" );
          break;
       case 324 :
          sprintf ( Msgbuf,"LL" );
          break;
       case 328 :
          sprintf ( Msgbuf,"RRI" );
          break;
       case 360 :
          sprintf ( Msgbuf,"WAS" );
          break;         
       case 364 :
          sprintf ( Msgbuf,"AYI" );
          break;   
       case 416 :
          sprintf ( Msgbuf,"CH" );
          break;      
       case 442 :
          sprintf ( Msgbuf,"NOR" );
          break;
       case 468 :
          sprintf ( Msgbuf,"MAL" );
          break;        
        case 482 :
          sprintf ( Msgbuf,"OVE" );
          break;
        case 492 :
          sprintf ( Msgbuf,"OAD" );
          break;    
        case 494 :
          sprintf ( Msgbuf,"OWN" );
          break;
       case 500 :
          sprintf ( Msgbuf,"OKI" );
          break;   
       case 510 :
          sprintf ( Msgbuf,"OON" );
          break;
       case 596 :
          sprintf ( Msgbuf,"UAL" );
          break;
       case 708 :
          sprintf ( Msgbuf,"WIL" );
          break;      
       case 716 :
          sprintf ( Msgbuf,"WUD" );
          break;
       case 832 :
          sprintf ( Msgbuf,"CHE" );
          break;
       case 842 :
          sprintf ( Msgbuf,"CAR" );
          break;            
       case 862 :
          sprintf ( Msgbuf,"CON" );
          break;
       case 922 :
          sprintf ( Msgbuf,"MUC" );
          break;
       case 968 :
          sprintf ( Msgbuf,"OUS" );
          break;   
       case 974 :
          sprintf ( Msgbuf,"OUG" );
          break;   
       case 1348 :
          sprintf ( Msgbuf,"ALL" );
          break;
       case 1480 :
          sprintf ( Msgbuf,"JUS" );
          break;
        case 1940 :
          sprintf ( Msgbuf,"OUL" );
          break;   
       case 1942 :
          sprintf ( Msgbuf,"OUP" );
          break;
       case 14752 :
          sprintf ( Msgbuf,"MUCH" );
          break;               
       default: 
          curChr ='*';// comment this line out when using the following debug code
          // for debugging: 
//          sprintf( Msgbuf,"[%d]",decodeval );
//          for(int i = 0; i<TDBptr; i++){
//            sprintf( Msgbuf, "%s %d, ",Msgbuf, TimeDatBuf[i]);
//          }
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
      if (curRow+1 > row)scrollpg(); // its time to Scroll the Text up one line
   }
   sprintf ( Msgbuf,"%s%c", Msgbuf,curChr );
   dispMsg(Msgbuf); // print current character(s) to OLED display
//   if(Test) Serial.println(Msgbuf);
//   int msgpntr = 0;
//   if( interruptPin == 2) KillINT();
//   else enableDisplay();
//   tft.setCursor(cursorX, cursorY);
//   while( Msgbuf[msgpntr] !=0){
//    char curChar = Msgbuf[msgpntr];
//    if(curRow>0) sprintf ( Pgbuf,"%s%c", Pgbuf, curChar);
//    tft.print(curChar);
//    msgpntr++;
//    cnt++;
//    if(((cnt)-offset)*fontW >= displayW){
//      curRow++;
//      cursorX = 0;
//      cursorY = curRow*(fontH+10);
//      offset = cnt;
//      tft.setCursor(cursorX, cursorY);
//      if (curRow+1 > row){
//        scrollpg();
//      }
//    }
//    else cursorX = (cnt-offset)*fontW; 
//   }
   
   wpm = CalcWPM(avgDah/3);
   if(wpm != 1){//if(wpm != lastWPM & decodeval == 0 & wpm <36){//wpm != lastWPM
    //lastWPM = wpm;
    if(curChr !=' ') showSpeed();
    if(TEcnt> 7 &  curRatio > 4.5){ // if true, we probably not waiting long enough to start the decode process, so reset the dot dash ratio based o current dash times
     avgDit = avgDah/3;   
    }
   }
   
   if( interruptPin == 2) enableINT();
  }

//////////////////////////////////////////////////////////////////////
void dispMsg(char Msgbuf[32]){
  if(Test) Serial.println(Msgbuf);
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
  if(((cnt)-offset)*fontW >= displayW){
    curRow++;
    cursorX = 0;
    cursorY = curRow*(fontH+10);
    offset = cnt;
    tft.setCursor(cursorX, cursorY);
    if (curRow+1 > row){
      scrollpg();
    }
  }
  else cursorX = (cnt-offset)*fontW;
 } 
}
//////////////////////////////////////////////////////////////////////
void scrollpg(){
  //buttonEnabled =false;
  //if(Test)Serial.print("ReDraw Frame\n");
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
    if(displayW == 480) Pgbuf[cnt] = Pgbuf[cnt+40];//shift existing text character forward by one line
    else Pgbuf[cnt] = Pgbuf[cnt+27];//shift existing text character forward by one line
     
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
/////////////////////////////////////////////////////////////////////

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
    //Create Clear Button
    int Bposx = 130;
    int Bwidth = 80;
    int Bposy = 195;
    int Bheight = 40;
  if (scrnHeight == 320) Bposy = scrnHeight -(Bheight+5);
  if (scrnWidth == 480)  Bposx = Bposx +32;//Bposx = Bposx +20;
  tft.fillRect(Bposx,Bposy, Bwidth, Bheight, BLUE);
  tft.drawRect(Bposx,Bposy, Bwidth, Bheight,WHITE);
  tft.setCursor(Bposx+11,Bposy+12);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Clear");

}
///////////////////////////////////////////////////////////

void Button2(){
    //Create Norm/Bug Button
    int Bposx = 210;
    int Bwidth = 80;
    int Bposy = 195;
    int Bheight = 40;
  if (scrnHeight == 320) Bposy = scrnHeight -(Bheight+5);
  if (scrnWidth == 480) Bposx = Bposx +32;//Bposx = Bposx +20;  
  tft.fillRect(Bposx,Bposy, Bwidth, Bheight, GREEN);
  tft.drawRect(Bposx,Bposy, Bwidth, Bheight,WHITE);
  tft.setCursor(Bposx+11,Bposy+12);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  switch (ModeCnt){
  case 0:
    tft.print("Norm");
    break;
  case 1:
    tft.print("Bug1");
    break;
  case 2:
    tft.print("Bug2");
    break;     
  }
//  if(!BugMode) tft.print("Norm");
//  else  tft.print("Bug");
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

  tft.setCursor(0, (row+1)*(fontH+10));
  tft.fillRect(0 , (row+1)*(fontH+10), 11*fontW, fontH+10, BLACK);//tft.fillRect(0 , 8*(fontH+10), 10*fontW, fontH+10, BLACK);
  tft.print(buf);
//  if(!mark) tft.print(buf);
//  else{
//     mark = false;
//     tft.print("XXXXXX");
//  }
}
