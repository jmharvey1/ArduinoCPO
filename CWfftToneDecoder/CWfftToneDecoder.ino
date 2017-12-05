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
 * In its present form, it optumized for tones around 600Hz.
 */


#define LOG_OUT 1 // use the log output function
#define FFT_N 128     //128//256 // set to 256 point fft
#define DataOutPin 11
#define GainPin 9

#include <FFT.h> // include the library
int dblSmplCnt;
int halfSmplCnt;
//int avgLowVal =70;
//int avgHighVal =-70;
//int avgLvlVal = 0;
int longAvgLvl = 0;
int hiTrgVal = 70;
int loTrgVal = -70;
int SqlchVal = 9;
int last[18];
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
    last[0] = fft_log_out[7];
    last[1] = fft_log_out[8];
    last[2] = fft_log_out[9];
    int curLfreqVal=0;
    int curMfreqVal=0;
    int curHfreqVal=0;
    int oldLfreqVal=0;
    int oldMfreqVal=0;
    int oldHfreqVal=0;

    int noise = fft_log_out[6];//fft_log_out[12];
    //noise = (noise+ fft_log_out[10])/2;//(noise+ fft_log_out[13])/2;
    if(noise < fft_log_out[10]) noise = fft_log_out[10];
        
    for(int i = 0 ; i<sampleSize; ++i){
      curLfreqVal = curLfreqVal+last[i*3];
      //Serial.print("; Cur:");
      //Serial.print(last[i*3]);
      //Serial.print("; Old:");
      curMfreqVal = curMfreqVal+last[(i*3)+1];
      curHfreqVal = curHfreqVal+last[(i*3)+2];
      oldLfreqVal = oldLfreqVal+last[(i*3)+oldOffSet];
      //Serial.print(last[(i*3)+oldOffSet]);
      oldMfreqVal = oldMfreqVal+last[(i*3)+oldOffSet+1];
      oldHfreqVal = oldHfreqVal+last[(i*3)+oldOffSet+2];
    }
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
//    if(totSlope>0) avgHighVal = (19*avgHighVal+totSlope)/20;
//    else avgLowVal = (19*avgLowVal+totSlope)/20;
//    if(totSlope>0) avgHighVal = (5*avgHighVal+totSlope)/6;
//    else avgLowVal = (5*avgLowVal+totSlope)/6;
    
    int curSigLvl = (curLfreqVal+curMfreqVal+curHfreqVal)/3;
    if(noise> longAvgLvl) longAvgLvl = noise;
    else if(curSigLvl>SqlchVal+longAvgLvl){
      longAvgLvl = curSigLvl-(SqlchVal+5);
    }
    else  longAvgLvl = (20*longAvgLvl-(noise/20))/20;//else longAvgLvl = (40*longAvgLvl+noise)/41;
//    if(curSigLvl> avgLvlVal) avgLvlVal = curSigLvl; 
//    else avgLvlVal = (3*avgLvlVal+curSigLvl )/4;
    //avgLvlVal = (7*avgLvlVal+curSigLvl+curSigLvl )/8;
  
//    if(avgHighVal > 50) hiTrgVal = avgHighVal-15;
//    else hiTrgVal = 32;
//    if(avgLowVal < -25) loTrgVal = avgLowVal-20;
//    else loTrgVal = -5;
//  hiTrgVal = 27;//32;
  loTrgVal = -8;//-12;//-10
////////////////////////////////////////////////////////////

//   if( curSigLvl>longAvgLvl+SqlchVal) armHi = true;//if(totSlope>hiTrgVal || avgLvlVal>longAvgLvl+36)armHi = true;
//   else armHi = false;
//   if(totSlope<loTrgVal)armLo = true; //& curSigLvl<longAvgLvl+35
//   else armLo = false;

if( curSigLvl>longAvgLvl+SqlchVal){
    armHi = true;//if(totSlope>hiTrgVal || avgLvlVal>longAvgLvl+36)armHi = true;
    armLo = false;
   }
   else{
    armHi = false;
    armLo = true;
   }



   if(armLo) toneDetect = false;
//   else armLo = false;

   if(armHi) toneDetect = true;// by placing this line here, a High sig Lvl will overide a negative going slope
//   else armHi = false;

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
    if(((delayLine ^ B00001110) == 4 )|| ((delayLine ^ B00001111) == 4)) delayLine |= B00000100;
    if(((delayLine ^ B00000001) == B00000100) || ((delayLine ^ B00000000) == B00000100)) delayLine &= B11111011;
      
  if(delayLine & B00001000){
     digitalWrite(DataOutPin, LOW);
  }else{
     digitalWrite(DataOutPin, HIGH);
   }
//  if( curSigLvl>longAvgLvl+SqlchVal) armHi = true;//if(totSlope>hiTrgVal || avgLvlVal>longAvgLvl+36)armHi = true;
//  else armHi = false;
//   
//  if(totSlope<loTrgVal)armLo = true; //& curSigLvl<longAvgLvl+35
//  else armLo = false;


  fft_log_out[halfSmplCnt-6]= 0; 
  fft_log_out[halfSmplCnt-5]= curSigLvl;
  fft_log_out[halfSmplCnt-4]= longAvgLvl+SqlchVal;
  fft_log_out[halfSmplCnt-3]= 0;//avgLowVal+128;
  fft_log_out[halfSmplCnt-2]= loTrgVal+128;
  fft_log_out[halfSmplCnt-1]= totSlope+128;
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
