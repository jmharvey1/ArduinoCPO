/* Rev: 2018-02-14 */
/*
fft_adc.pde
guest openmusiclabs.com 8.18.12
example sketch for testing the fft library.
it takes in data on ADC0 (Analog0) and processes them
with the fft. the data is sent out over the serial
port at 115.2kb.  there is a pure data patch for
visualizing the data.
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
 * In its current form, this sketch is setup to caputure Audio tones @ ~700Hz. 
 * If a another freq is perferred, change the value assigned to the "CntrFrqBin" variable.
 * Note: You can use the "fft_adc.pde" Processing sketch [included in this set of Github files]
 * to determine the fft bin (array entry) that represents the frequencies needed. 
 */
 
#define LOG_OUT 1 // use the log output function
#define FFT_N 128     //128//256 // set to 256 point fft
#define DataOutPin 11
#define GainPin 9
#define SlowData 6 // Manually Ground this Digital pin when when the incoming CW transmission is >30WPM  
                   // When this pin is left open, slower code transmissions should work, & noise immunity will be better

#include <FFT.h> // include the library

#include <Adafruit_NeoPixel.h>
#define PIN 14

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);
byte RED =0;
byte GREEN = 0;
byte BLUE =0;
int CntrFrqBin = 10;// ~700 Hz
int FrqHi = 0;
int FrqLo = 0;
int FrqOk = 0;
unsigned long pause =0;
int dblSmplCnt;
int halfSmplCnt;
unsigned long longAvgLvl = 0;
unsigned long NoiseAvgLvl = 40;
int hiTrgVal = 70;
int loTrgVal = -10;
int SqlchVal = 9;
int last[18];
int old[3];
//int lastHigh =0;
//int lastLow =0;
int lastSlopeVal=0;
//bool look4High = true;
byte delayLine = 0;
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

void setup() {
  //setup Digital signal output pin
  pinMode(DataOutPin, OUTPUT);
  digitalWrite(DataOutPin, HIGH);
  pinMode(SlowData, INPUT_PULLUP);

  //setup Gain Control Pin
  pinMode(GainPin, OUTPUT);
  digitalWrite(GainPin, HIGH); //Max Gain 40 db (adafruit MAX9814)
  //digitalWrite(GainPin, LOW); //Max Gain 50 db
  dblSmplCnt = FFT_N*2;
  halfSmplCnt = FFT_N/2;
  for(int i = 0 ; i<18; ++i){
    last[i] = 10;
    //last[i+10] = 10;
  }
  Serial.begin(115200); // use the serial port
  strip.begin();
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
    cli();  // UDRE interrupt slows this way down on arduino1.0
    double t = 0;
    
      for (int i = 0 ; i < dblSmplCnt ; i += 2) { // save 256 samples
        for (int l = 0 ; l < 4 ; l++) { // this for loop makes the fft use the average value of every four samples
        while(!(ADCSRA & 0x10)); // wait for adc to be ready
        ADCSRA = 0xf5; // restart adc
        // Must read low first
        byte m = ADCL; // fetch adc data
        byte j = ADCH;
        int k = (j << 8) | m; // form into an int
        k -= 0x0200; // form into a signed int
        k <<= 6; // form into a 16b signed int
        /*
        if (l==0) t = k;
        else t= t+k;
        if (l==3){
          k = t/4;
          //k = 1024*sin(2*3.14 * 400 *t );
          //t++;
          fft_input[i] = k; // put real data into even bins
          fft_input[i+1] = 0; // set odd bins to 0 
        }
        */
        if (l==0) fft_input[i] = k;
        if (l==1) fft_input[i+1] = k;
//        if (l==2) fft_input[i] = fft_input[i]-k;
//        if (l==3) fft_input[i+1] = fft_input[i+1]-k;
      }
    }
    
    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft
    sei();
    int sampleSize =1;
    int oldOffSet = 3*sampleSize;
    for(int i = 0 ; i<15; ++i){//shift old data one level down
      last[17-i] = last[14-i];
    }
    
//    last[0] = fft_log_out[7];
//    last[1] = fft_log_out[8];
//    last[2] = fft_log_out[9];
    last[0] = fft_log_out[CntrFrqBin-1];
    last[1] = fft_log_out[CntrFrqBin];
    last[2] = fft_log_out[CntrFrqBin+1];
    int curLfreqVal=0;
    int curMfreqVal=0;
    int curHfreqVal=0;
    int oldLfreqVal=0;
    int oldMfreqVal=0;
    int oldHfreqVal=0;
    int passBandNoise=0; 
    int noise = fft_log_out[CntrFrqBin-2];//fft_log_out[12];//int noise = fft_log_out[6];//fft_log_out[12];
    if(noise < fft_log_out[CntrFrqBin+2]) noise = fft_log_out[CntrFrqBin+2];//if(noise < fft_log_out[10]) noise = fft_log_out[10];
        
    for(int i = 0 ; i<sampleSize; ++i){
      curLfreqVal = curLfreqVal+last[i*3];
      curMfreqVal = curMfreqVal+last[(i*3)+1];
      curHfreqVal = curHfreqVal+last[(i*3)+2];
      oldLfreqVal = oldLfreqVal+last[(i*3)+oldOffSet];
      oldMfreqVal = oldMfreqVal+last[(i*3)+oldOffSet+1];
      oldHfreqVal = oldHfreqVal+last[(i*3)+oldOffSet+2];
    }
//    passBandNoise = ((last[0]- last[3])+(last[1]- last[4])+(last[2]- last[5]))/3;
//    passBandNoise = (((last[0]- old[0])+(last[1]- old[1])+(last[2]- old[2]))/3)+128;
//    old[0] = last[0];
//    old[1] = last[1];
//    old[2] = last[2];
    //    Serial.println("");
    curLfreqVal = curLfreqVal/sampleSize;
    curMfreqVal = curMfreqVal/sampleSize;
    curHfreqVal = curHfreqVal/sampleSize;
    oldLfreqVal = oldLfreqVal/sampleSize;
    oldMfreqVal = oldMfreqVal/sampleSize;
    oldHfreqVal = oldHfreqVal/sampleSize;

    int lSlope = curLfreqVal - oldLfreqVal;
    int mSlope = curMfreqVal - oldMfreqVal;
    int hSlope = curHfreqVal - oldHfreqVal;
  
     
    int totSlope = (lSlope+mSlope+hSlope +lastSlopeVal)/3;
    lastSlopeVal = totSlope;
    totSlope = -(abs(totSlope));
    
    int curSigLvl = curMfreqVal;//(curLfreqVal+curMfreqVal+curHfreqVal)/3;
    //if((noise+(SqlchVal+10)> longAvgLvl) && noise <100) longAvgLvl = noise+(SqlchVal+10);
    
//    if((noise+(SqlchVal+10)> longAvgLvl) && noise < NoiseAvgLvl+30) longAvgLvl = noise+(SqlchVal+10);
//    
//    if(curSigLvl>  noise+10 && (curSigLvl-(SqlchVal+8)>longAvgLvl)){  //if(curSigLvl>SqlchVal+longAvgLvl){
//      longAvgLvl = curSigLvl-(SqlchVal+8);
//    }
    
    //else if(longAvgLvl < noise+(SqlchVal+10))longAvgLvl = noise+(SqlchVal+10);;
//    longAvgLvl = (30*longAvgLvl-1)/30;//(30*longAvgLvl-(noise/30))/30;//else longAvgLvl = (40*longAvgLvl+noise)/41;
    if(curSigLvl>longAvgLvl) longAvgLvl = ((5*longAvgLvl)+curSigLvl)/6;
    else longAvgLvl = ((30*longAvgLvl)-3)/30;
    SqlchVal = 30-int((curSigLvl-noise)/2)+(int(3*NoiseAvgLvl/4)+int((longAvgLvl/2)));
    if(SqlchVal/2 > NoiseAvgLvl) NoiseAvgLvl = SqlchVal/2;
////////////////////////////////////////////////////////////

if(!armHi&&( curSigLvl>SqlchVal)){ //+50
//if(!armHi&&(( curSigLvl>longAvgLvl+SqlchVal)&&(totSlope<loTrgVal) )){
  armHi = true;
  armLo = false;
  toneDetect = true;
}
if(!armLo &&( curSigLvl<SqlchVal)){ //+50
//if(!armLo &&(( curSigLvl<longAvgLvl+SqlchVal)&&(totSlope<loTrgVal) )){
  armHi = false;
  armLo = true;
  toneDetect = false;
}
    NoiseAvgLvl = (30*NoiseAvgLvl +noise)/31;


    delayLine= delayLine<<1;
    delayLine &= B00001110;
    
    if(toneDetect) delayLine |= B00000001;
//    if(((delayLine ^ B00000001) == B00000100) || ((delayLine ^ B00000000) == B00000100)){
//    Serial.print(delayLine);
//    Serial.print("/");
//    Serial.print(delayLine ^ B00000001);
//    Serial.print("; ");
//    Serial.print(delayLine);
//    Serial.print("/");
//    Serial.print(delayLine ^ B00000000);
//    Serial.print(";  ");
//    Serial.println(delayLine &= B11111011);
//    }

//Use For Slow code [<27WPM] fill in the glitches
 if(digitalRead(SlowData)){
    if(((delayLine ^ B00001110) == 4 )|| ((delayLine ^ B00001111) == 4)) delayLine |= B00000100;
    if(((delayLine ^ B00000001) == B00000100) || ((delayLine ^ B00000000) == B00000100)) delayLine &= B11111011;
 }
      
  if(delayLine & B00001000){
     digitalWrite(DataOutPin, LOW);
     pause =25;
     if(abs(last[0] - last[2]) < 8 ){
      FrqOk++;
      if(FrqOk>4){
        FrqOk = 4;
        FrqHi = 0;
        FrqLo = 0;
        GREEN = longAvgLvl;//longAvgLvl+SqlchVal;//100;
        BLUE = 0;
        RED = 0;
       }
     }
     else{
        
       if((last[0] - last[2])> 8 ){
        FrqLo++;
        if(FrqLo>4){
          FrqLo = 4;
          FrqHi = 0;
          FrqOk = 0;
          RED = longAvgLvl;//longAvgLvl+SqlchVal;//100;
          BLUE = 0;
          GREEN = 0;
        }
       }
       if((last[2] - last[0])> 8 ){
        FrqHi++;
        if(FrqHi>4){
          FrqHi = 4;
          FrqLo = 0;
          FrqOk = 0;
          BLUE = longAvgLvl;//longAvgLvl+SqlchVal;//100;
          RED = 0;
          GREEN = 0;
        }
       }
     }
  }else{ //No Tone Detected
     digitalWrite(DataOutPin, HIGH);
     if(NoiseAvgLvl+30>80){ //Show Out of Band Freq after No valid signal interval [pause]
      if(pause>0){
         pause--;
      }
      else{
        BLUE = 0;
        RED = 0;
        GREEN = 0;   

       if(last[0] - last[2] > 8 ){
          RED = NoiseAvgLvl-30;//20;
          BLUE = 0;
          GREEN = 0;
         }
         if((last[2] - last[0])> 8 ){
          BLUE = NoiseAvgLvl-30;//;
          RED = 0;
          GREEN = 0;
         }
      } 
    }
    else{
      BLUE = 0;
      RED = 0;
      GREEN = 0;
    }
  }
  strip.setPixelColor(0,strip.Color(GREEN, RED, BLUE)); // set color of ASD 1293 LED to Green
  strip.show(); 
//  if( curSigLvl>longAvgLvl+SqlchVal) armHi = true;//if(totSlope>hiTrgVal || avgLvlVal>longAvgLvl+36)armHi = true;
//  else armHi = false;
//   
//  if(totSlope<loTrgVal)armLo = true; //& curSigLvl<longAvgLvl+35
//  else armLo = false;
//  fft_log_out[halfSmplCnt-8]= 0;
//  fft_log_out[halfSmplCnt-7]= passBandNoise;
  fft_log_out[halfSmplCnt-6]= 0; 
  fft_log_out[halfSmplCnt-5]= curSigLvl;
  fft_log_out[halfSmplCnt-4]= SqlchVal;//+50;//longAvgLvl+SqlchVal;
  fft_log_out[halfSmplCnt-3]= 0;//avgLowVal+128;
  fft_log_out[halfSmplCnt-2]= NoiseAvgLvl+30;//loTrgVal+128;
  fft_log_out[halfSmplCnt-1]= NoiseAvgLvl;//totSlope+128;
  if(toneDetect){
//    Serial.print("+++");
    if(decodeval ==0){
      decodeval =2;
      shift = false;
      dahcnt = 0;
     }
    if(shift){
      decodeval = decodeval<<1;
      shift = false; 
    }
    ++dahcnt;
    if(dahcnt >10){
      decodeval = decodeval+1;
      dahcnt = 0;
    }
    ltrCnt=0;
    wrdCnt=0;
    
  }
  else {
//    Serial.print("|");
    shift = true;
     dahcnt = 0;
    ++ltrCnt;
    if(gotLtr)++wrdCnt;    
  }
  if(ltrCnt>=10){
//    Serial.print("LLLLLLLLLLLLLLLLLLLLLLL  ");
    if(decodeval !=0){
//      Serial.print(DisplayChar(decodeval)); //un-comment this line to print 18WPM code to serial port
      gotLtr = true;
    }
//    Serial.println(decodeval);
    decodeval =0;
    ltrCnt=0;
  }
  if(wrdCnt>=15){
//    Serial.println("WWWWWWWWWWWWWWWWWWWWWW");
    if(gotLtr){
      gotLtr = false;
//      Serial.print(" "); //un-comment this line to print 18WPM code to serial port
      ++lineFeed;
    }
    
    if(lineFeed ==15){
      lineFeed =0;
//      Serial.print("\n"); //un-comment this line to print 18WPM code to serial port
    }
    
    wrdCnt=0;
    ltrCnt=0;
  }
//  Serial.print("\t");
//  Serial.print(loTrgVal);
//  Serial.print("\t");
//  Serial.print(hiTrgVal);
//  Serial.print("\t");
//  Serial.println(totSlope);
////  Serial.print("\t");
////  Serial.print(avgHighVal);//

   Serial.write(255); // send a start byte
   Serial.write(fft_log_out, halfSmplCnt); // send out the data
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
