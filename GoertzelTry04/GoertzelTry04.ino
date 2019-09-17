/* Rev: 2019-07-07 
   a Goertzel single Tone Detection process
   this time using a more balanced sampling interval
   along with a more simplified tone/squelch algorythem
   plus finding a optimized sampling rate.
   This version seeks to detect and indicate tone frequency hi or low
   output via the RGB LED
   
*/

/*
 * This Sketch was originally written to run on an SS Micro.
 * Its purpose is to use the Micro as audio tone detector for CW signals 
 * the digital output of this sketch (SS Micro Digital Pin 11) is intended to act as an input  
 * to drive a companion Arduino CW decoder
 * 
 */

 /*
  * See comment linked to the "SlowData" variable for fast CW transmissions
  */

/*
 * This version adds ASD1293 GRB LED Tuning indicator
 * Ideally when the LED is Green, the FFT pass band is "centered" on the CW tone
 * Conversely when the LED is Blue the CW tone is above the "Pass Band"
 * or if the LED is Red the tone is below the "Pass Band"
 * As wired in this version of the sketch. the ADS1293 input pin is connected to the Micro's Digital pin 14.
 * Note to use this LED you will need to include the Adafruit_NeoPixel library. Intrustions for installing it can be found at  
 * https://learn.adafruit.com/adafruit-neopixel-uberguide/arduino-library-installation
 */
/* 
 * In its current form, this sketch is setup to caputure Audio tones @ ~750Hz. 
 * If a another freq is perferred, change the value assigned to the "TARGET_FREQUENCYC" variable.
 */
 
#define DataOutPin 11
#define GainPin 9
#define SlowData 6 // Manually Ground this Digital pin when when the incoming CW transmission is >30WPM  
                   // When this pin is left open, slower code transmissions should work, & noise immunity will be better


/*
 * Goertzel stuff
 */
#define FLOATING float
#define SAMPLE  unsigned char

#define TARGET_FREQUENCYC  749.5 //Hz
#define TARGET_FREQUENCYL  734.0 //Hz
#define TARGET_FREQUENCYH  766.0 //Hz
#define CYCLE_CNT 6.0 // Number of Cycles to Sample per loop
float SAMPLING_RATE = 21900.0;//22000.0;//22800.0;//21750.0;//21990.0;
int SmplLpCnt = 200;
// #define N 184 //Block size sample 6 cylces of a 750Hz tone; ~5ms interval
int N = 0; //Block size sample 6 cylces of a 750Hz tone; ~5ms interval
int NL = 0;
int NC = 0;
int NH = 0;
int UsablTn = 0;
FLOATING coeff;
FLOATING Q1;
FLOATING Q2;
FLOATING cosine;

FLOATING coeffL;
FLOATING coeffC;
FLOATING coeffH;

int TnLpCnt = 0;
float mag = 0;
float magC= 0;
float magC1= 0;
float magC2 =0;
float magL = 0;
float magL1 = 0;
float magH = 0;
int kOld = 0;
int BlkData[300];
float SigPwrBin[64];
// End Goertzel stuff

float SclFctr =0;
float SclFctr2 =0;
float ToneLvl = 0;
float noise = 0;
float AvgToneSqlch = 0;
int ToneOnCnt = 0;
long KeyState = 0;
long SNR = 0;
//long Delta =0;
//long OldmagC =0; 

#include <Adafruit_NeoPixel.h>
#define PIN 14
int loopCnt = 0;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);
byte RED =0;
byte GREEN = 0;
byte BLUE =0;
byte WHITE =80; //Sets Brightness of  Down White Light
byte LightLvl =0;
int CntrFrqBin = 10;// ~700 Hz
int NoiseBin = 15;
int FrqHi = 0;
int FrqLo = 0;
int FrqOk = 0;
//unsigned long pause =0;
int dblSmplCnt;
int halfSmplCnt;
long longAvgLvl = 0;
long NoiseAvgLvl = 40;
float MidPt = 0;
int hiTrgVal = 70;
int loTrgVal = -10;
float SqlchVal = 0.0;
int loopcnt = 4;
int last[18];
int old[3];
//int lastHigh =0;
//int lastLow =0;
int lastSlopeVal=0;
//bool look4High = true;
byte delayLine = 0;
byte delayLine2 = 0;
bool shift = false;
bool armHi = false;
bool armLo = false;
bool gotLtr = false;
bool toneDetect = false;
int lineFeed =0;
int ltrCnt=0;
int wrdCnt=0;
int dahcnt = 0;
int decodeval=0;
char Msgbuf[6];
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

/* Call this routine before every "block" (size=N) of samples. */
void ResetGoertzel(void)
{
  Q2 = 0;
  Q1 = 0;
}

/* Call this once, to precompute the constants. */
void InitGoertzel(void)
{
  int  k;
  FLOATING  floatN;
  FLOATING  omega;
//  N = (int)((SAMPLING_RATE/TARGET_FREQUENCY)*(float)(CYCLE_CNT));
//  floatN = (FLOATING) N;
//  
//  k = (int) (0.5 + ((floatN * TARGET_FREQUENCY) / SAMPLING_RATE));
//  omega = (2.0 * PI * k) / floatN;
//  sine = sin(omega);
//  cosine = cos(omega);
//  coeff = 2.0 * cosine;

  NC = (int)((SAMPLING_RATE/TARGET_FREQUENCYC)*(float)(CYCLE_CNT));
  floatN = (FLOATING) NC;
  
  k = (int) (0.5 + ((floatN * TARGET_FREQUENCYC) / SAMPLING_RATE));
  omega = (2.0 * PI * k) / floatN;
//  sine = sin(omega);
  cosine = cos(omega);
  coeffC = 2.0 * cosine;

  NL = (int)((SAMPLING_RATE/TARGET_FREQUENCYL)*(float)(CYCLE_CNT));
  floatN = (FLOATING) NL;
  
  k = (int) (0.5 + ((floatN * TARGET_FREQUENCYL) / SAMPLING_RATE));
  omega = (2.0 * PI * k) / floatN;
//  sine = sin(omega);
  cosine = cos(omega);
  coeffL = 2.0 * cosine;

  NH = (int)((SAMPLING_RATE/TARGET_FREQUENCYH)*(float)(CYCLE_CNT));
  floatN = (FLOATING) NH;
  
  k = (int) (0.5 + ((floatN * TARGET_FREQUENCYH) / SAMPLING_RATE));
  omega = (2.0 * PI * k) / floatN;
//  sine = sin(omega);
  cosine = cos(omega);
  coeffH = 2.0 * cosine;

  ResetGoertzel();
}

/* Call this routine for every sample. */
void ProcessSample(int sample)
{
  FLOATING Q0;
  Q0 = coeff * Q1 - Q2 +(FLOATING)sample;
  Q2 = Q1;
  Q1 = Q0;
}


/* Optimized Goertzel */
/* Call this after every block to get the RELATIVE magnitude squared. */
FLOATING GetMagnitudeSquared(float Q1, float Q2, float coeff)
{
  FLOATING result;

  result = Q1 * Q1 + Q2 * Q2 - Q1 * Q2 * coeff;
  return result;
}

void setup() {
  //setup Digital signal output pin
  pinMode(DataOutPin, OUTPUT);
  digitalWrite(DataOutPin, HIGH);
  pinMode(SlowData, INPUT_PULLUP);

  //setup Gain Control Pin
  pinMode(GainPin, OUTPUT);
  digitalWrite(GainPin, HIGH); //Max Gain 40 db (adafruit MAX9814)
  //digitalWrite(GainPin, LOW); //Max Gain 50 db
  //dblSmplCnt = FFT_N*2;
  halfSmplCnt = 64;
  for(int i = 0 ; i<18; ++i){
    last[i] = 10;
    //last[i+10] = 10;
  }
  Serial.begin(115200); // use the serial port
  strip.begin();
   for(int i = 0 ; i<64; ++i){// initialize results array
    SigPwrBin[i]= 0;
   }
  
  InitGoertzel();
  coeff = coeffC;
  N = NC;
  
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = B00000000;// disable Adc in order to configure it
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // clear ADLAR in ADMUX (0x7C) to right-adjust the result
  // ADCL will contain lower 8 bits, ADCH upper 2 (in last two bits)
  ADMUX &= B11011111;
  
  // Set REFS1..0 in ADMUX (0x7C) to change reference voltage to the
  // proper source (01)
  ADMUX |= B01000000;
  
  // Clear MUX3..0 in ADMUX (0x7C) in preparation for setting the analog
  // input
  ADMUX = B01000111;
  DIDR0 = 0x01; // turn off the digital input for adc0
  
  ADCSRB = B00000000; // Set trigger source = Free Running
  ADCSRB |= B11000000; // Set High Speed Mode & Enable Multiplexer
  // Set the Prescaler to 128 (16000KHz/128 = 125KHz)
  // Above 200KHz 10-bit results are not reliable.
  ADCSRA |= B00000111; // Set prescale = 128)
  // Set ADATE (bit 5) in ADCSRA (0x7A) to enable auto-triggering.
  ADCSRA |= B00100000;
  // Set ADIE (bit 3) in ADCSRA (0x7A) to enable the ADC interrupt.
  // Without this, the internal interrupt will not trigger.
  ADCSRA |= B00001000;
  // Set ADEN (bit 7) in ADCSRA (0x7A) to enable the ADC.
  // Note, A normal conversion takes 13 ADC clocks to execute, but
  // the first conversion after the ADC is switched on (ADEN in
  // ADCSRA is set) takes 25 ADC clock cycles in order to initialize the analog circuitry.
  ADCSRA |= B10000000;
  sei(); // Enable global interrupts
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // Kick off the first ADC
  //readFlag = 0;
  // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
  ADCSRA |=B01000000;
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
}

void loop() {
  
  
   while(1) { // reduces jitter
    //sweep sample rates
//    --SmplLpCnt;
//    if(SmplLpCnt == 0){
//      SmplLpCnt = 200;
//      SAMPLING_RATE = SAMPLING_RATE - 20;
//      if(SAMPLING_RATE<20000){
//        SAMPLING_RATE = 24570;
//      }
//      InitGoertzel();
//      coeff = coeffC;
//      N = NC;
//    }
    
    cli();  // UDRE interrupt slows this way down on arduino1.0
    ResetGoertzel();
    //TnLpCnt = 0;
    switch (TnLpCnt) {
      case 0 :
        N = NC;
        coeff = coeffC;
        break;
      case 1 :
        N = NL;
        coeff = coeffL;
        break;
      case 2 :
        N = NH;
        coeff = coeffH;
        break;
    }
     
    //digitalWrite(DataOutPin, HIGH); //for timing/tuning tests only
    float AvgVal = 0.0;
    bool OvrLd = false;
    for (int i = 0 ; i < N ; ++i) { // save N samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      // Must read low first
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      k -=63; //Correct dc bias offest
//      if(k>4800 | k < -4800){
//        k=kOld; //bad data point zero it out//Serial.println(k); // enable to graph raw DAC values
//        OvrLd = true;
//      }
//      kOld = k;
      if(k>5000 | k < -5000){
        k=kOld; //bad data point zero it out//Serial.println(k); // enable to graph raw DAC values
        OvrLd = true;
      }
      else kOld = k;
      //BlkData[i] = k;
      ProcessSample(k);
      k= abs(k);
//      if(k >18000) OvrLd = true;//if(k >4500) OvrLd = true;
          
      AvgVal +=(float)k;
    }
    sei();
//    for (int i = 0 ; i < N ; ++i) {
//      Serial.println(BlkData[i]); // enable to graph raw DAC values
//    }
     //AvgVal=  AvgVal/2.5;
    //digitalWrite(DataOutPin, LOW); //for timing/tuning tests only
    mag =sqrt(GetMagnitudeSquared(Q1, Q2, coeff));
//    if( ToneLvl>SqlchVal){
//      switch (TnLpCnt) {
//        case 0 :
//          OldmagC = magC;
//          magC = (14*magC+mag)/15;
//          Delta = magC-OldmagC;
//          magL = magL +Delta;
//          magH = magH +Delta; 
//          break;
//        case 1 :
//          magL = (14*magL+mag)/15;
//          break;
//        case 2 :
//          magH = (14*magH+mag)/15;
//          break;
//      }
//    }
    bool GudTone = true;
    SigPwrBin[CntrFrqBin] = mag; //SigPwrBin[CntrFrqBin] = ((mag)/SclFctr); //Scale the tone magnitude to a max of 256
    if (mag <40000) GudTone = false;
    SigPwrBin[NoiseBin] = AvgVal-mag ;//SigPwrBin[NoiseBin] = ((AvgVal-(mag))/SclFctr);//((AvgVal-(1.26*mag))/SclFctr);
    float CurNoise = ((CurNoise)+SigPwrBin[NoiseBin])/2;
    ToneLvl =mag;//SigPwrBin[CntrFrqBin]; 
    noise = ((6*noise)+(CurNoise))/7;// calculate the running average of the unfiltered digitized Sound     
    SNR = 0;
    if(ToneLvl> noise & ToneLvl>  CurNoise){
      ++ToneOnCnt;
      if(ToneOnCnt >= 4){
        ToneOnCnt = 3;
        //MidPt =  (MidPt+(((ToneLvl -noise)/2)+ noise))/2;//NoiseAvgLvl
        MidPt =  ((2*MidPt)+(((ToneLvl -NoiseAvgLvl)/4)+ NoiseAvgLvl))/3;
        if(MidPt>  AvgToneSqlch) AvgToneSqlch= MidPt;
        SNR = ToneLvl -NoiseAvgLvl;
      }
    }
    SqlchVal = noise;
    if((ToneLvl< CurNoise) & (CurNoise>SqlchVal))  SqlchVal = CurNoise;//could be a static burst
    if(AvgToneSqlch> SqlchVal) SqlchVal = AvgToneSqlch;
    if( ToneLvl>SqlchVal){
      switch (TnLpCnt) {
        case 0 :
          //magC = (14*magC+mag)/15;
          magC1 = mag;
          break;
        case 1 :
          //magL = (14*magL+mag)/15;
          magL1 = mag;
          break;
        case 2 :
          magH = (14*magH+mag)/15;
          magC = (14*magC+magC1)/15;
          magL = (14*magC+magL1)/15;
          break;
      }
      ++TnLpCnt;
      if(TnLpCnt>2) TnLpCnt=0; 
    }
    else TnLpCnt = 0;
   

    
////////////////////////////////////////////////////////////
// Graph Geortzel magnitudes
    //Serial.print(TnLpCnt);
    //Serial.print("\t");
    Serial.print(magH);//Blue
    Serial.print("\t");
    Serial.print(magL);//Red
    Serial.print("\t");
    Serial.print(magC);//Green
    Serial.print("\t");  
//    Serial.print(LightLvl);
//    Serial.print("\t");

// Control Sigs     
    Serial.print(ToneLvl);//Serial.print(mag);//Serial.print(ToneLvl);//Orange
    Serial.print("\t");
//    Serial.print(SNR);//Purple
//    Serial.print("\t");
//    Serial.print(NoiseAvgLvl);//Purple
//    Serial.print("\t");
    Serial.print(noise);//Purple
    Serial.print("\t");
////    Serial.print(SigPwrBin[NoiseBin]);
////    Serial.print("\t");
    Serial.print(AvgToneSqlch);//Light Blue 
    Serial.print("\t");
//    Serial.print(SAMPLING_RATE);
//    Serial.print("\t");
    

////////////////////////////////////////////////////////////
    if(armHi&&( ToneLvl>SqlchVal)){
      toneDetect = true; //Had the same Tonestate 2X in a row. So go with it
    }
    if(armLo &&( ToneLvl<SqlchVal)){
      toneDetect = false; //Had the same Tonestate 2X in a row. So go with it
      //TnLpCnt = 0;
    }
    if(!armHi&&( ToneLvl>SqlchVal)){ //+50
      armHi = true;
      armLo = false;
      //toneDetect = true;
    }
    if(!armLo &&( ToneLvl<SqlchVal)){ //+50
      armHi = false;
      armLo = true;
      UsablTn=0;
      //toneDetect = false;
      ToneOnCnt = 0;
      //TnLpCnt = 0;
    }
    loopcnt -= 1;
    if(loopcnt<0){
      NoiseAvgLvl = (30*NoiseAvgLvl +(noise))/31; //every 3rd pass thru, recalculate running average of the composite noise signal
      //AvgToneSqlch= (30*AvgToneSqlch +noise)/31;
      //AvgToneSqlch= (17*AvgToneSqlch +noise)/18;//trying a little faster decay rate, to better track QSB
      AvgToneSqlch= (17*AvgToneSqlch +NoiseAvgLvl)/18;
      loopcnt = 2;  
    }
    


    delayLine= delayLine<<1;
    delayLine &= B00001110;
    delayLine2= delayLine2<<1; //2nd delay line used pass led info to RGB LED
    delayLine2 &= B00001110;
    if(GudTone) delayLine2 |= B00000001;
    if(toneDetect) delayLine |= B00000001;
//    if(((delayLine ^ B00000001) == B00000100) || ((delayLine ^ B00000000) == B00000100)){
//      Serial.print(delayLine);
//      Serial.print("/");
//      Serial.print(delayLine ^ B00000001);
//      Serial.print("; ");
//      Serial.print(delayLine);
//      Serial.print("/");
//      Serial.print(delayLine ^ B00000000);
//      Serial.print(";  ");
//      Serial.println(delayLine &= B11111011);
//    }

//Use For Slow code [<27WPM] fill in the glitches  pin is left open
   if(digitalRead(SlowData)){
      if(((delayLine ^ B00001110) == 4 )|| ((delayLine ^ B00001111) == 4)) delayLine |= B00000100;
      if(((delayLine ^ B00000001) == B00000100) || ((delayLine ^ B00000000) == B00000100)) delayLine &= B11111011;
   }
  KeyState= -100000;    
  if(delayLine & B00001000){// key Closed
     digitalWrite(DataOutPin, LOW);
     KeyState=0;
//     ++TnLpCnt;
//     if(TnLpCnt>2){
//      TnLpCnt =0;
//     }
  }
  else{ //No Tone Detected
     digitalWrite(DataOutPin, HIGH);
//     if(NoiseAvgLvl+30>80){ //Show Out of Band Freq after No valid signal interval [pause]
//    }
  }
// Graph data     
  Serial.println(KeyState);
  
//The following code is just to support the RGB LED.
 if(delayLine & B00001000){ //key Closed
  GREEN =0;
  RED=0;
  BLUE=0;
  if(magC >300000){//Excessive Tone Amplitude 
    GREEN =120;
    RED = 120;
    BLUE = 120;
  }//End Excessive Tone Amplitude
  else{// modulated light tests 
    LightLvl =(byte)(256*(magC/400000));
    if(magC <7000){// Just detectable Tone
      GREEN = LightLvl;
      RED = LightLvl; 
      BLUE = LightLvl;
    }//End Just detectable Tone
    else{//Freq Dependant Lighting
      magC2 = 1.02*magC;
      if(magC2>=magH & magC2>=magL){//Freq Just Right
        //BLUE = 0;
        GREEN= LightLvl;
        //RED = 0;
      }//End Freq Just Right
      else if(magH>magC & magH>=magL){//Freq High
        BLUE = LightLvl;
        //GREEN= 0;
        //RED = 0;
      }//End Freq High
      else if(magL>magC & magL>=magH){//Freq Low
        RED = LightLvl;
        //GREEN= 0;
        //BLUE = 0;
      }//End Freq Low
      if (GREEN==0 & RED ==0 & BLUE == 0){
        GREEN =10;
        RED = 10;
        BLUE = 10;
      }
    }//End Freq Dependant Lighting
    
  }//End modulated light tests
  strip.setPixelColor(0,strip.Color(GREEN, RED, BLUE)); 
  }//end Key Closed Tests
  else{//key open
    if(OvrLd){
      GREEN =10;
      RED=5;
      BLUE=10;
    } 
    else{
      GREEN =0;
      RED=0;
      BLUE=0;
    }
    
    
    strip.setPixelColor(0,strip.Color(GREEN, RED, BLUE)); // set color of ASD 1293 LED to Green
  }
  strip.show(); //activate the RGB LED
  //this set of code supports the external "Processing' Sketch; which is largley superceded by the Arduinio's IDE serial Plotter Tool
  SigPwrBin[halfSmplCnt-6]= 0; 
  SigPwrBin[halfSmplCnt-5]= ToneLvl;
  SigPwrBin[halfSmplCnt-4]= SqlchVal;//+50;//longAvgLvl+SqlchVal;
  SigPwrBin[halfSmplCnt-3]= 0;//avgLowVal+128;
  //SigPwrBin[halfSmplCnt-2]= NoiseAvgLvl+30;//loTrgVal+128;
  SigPwrBin[halfSmplCnt-1]= NoiseAvgLvl;//totSlope+128;
//  if(toneDetect){
////    Serial.print("+++");
//    if(decodeval ==0){
//      decodeval =2;
//      shift = false;
//      dahcnt = 0;
//     }
//    if(shift){
//      decodeval = decodeval<<1;
//      shift = false; 
//    }
//    ++dahcnt;
//    if(dahcnt >10){
//      decodeval = decodeval+1;
//      dahcnt = 0;
//    }
//    ltrCnt=0;
//    wrdCnt=0;
//    
//  }
//  else {
////    Serial.print("|");
//    shift = true;
//     dahcnt = 0;
//    ++ltrCnt;
//    if(gotLtr)++wrdCnt;    
//  }
//  if(ltrCnt>=10){
////    Serial.print("LLLLLLLLLLLLLLLLLLLLLLL  ");
//    if(decodeval !=0){
////      Serial.print(DisplayChar(decodeval)); //un-comment this line to print 18WPM code to serial port
//      gotLtr = true;
//    }
////    Serial.println(decodeval);
//    decodeval =0;
//    ltrCnt=0;
//  }
//  if(wrdCnt>=15){
////    Serial.println("WWWWWWWWWWWWWWWWWWWWWW");
//    if(gotLtr){
//      gotLtr = false;
////      Serial.print(" "); //un-comment this line to print 18WPM code to serial port
//      ++lineFeed;
//    }
//    
//    if(lineFeed ==15){
//      lineFeed =0;
////      Serial.print("\n"); //un-comment this line to print 18WPM code to serial port
//    }
//    
//    wrdCnt=0;
//    ltrCnt=0;
//  }

// Send data to processing sketch (fht-128-analyser)
//   Serial.write(255); // send a start byte
//   Serial.write(SigPwrBin, 64); // send out the data
 }//end while
}//end Loop


/////////////////////////////////////////////////////////////////////////
char* DisplayChar(int decodeval){
    char curChr = 0 ;
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
    sprintf ( Msgbuf,"%s%c", Msgbuf,curChr );
    return Msgbuf;
}
