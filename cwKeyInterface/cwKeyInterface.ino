/*
 * The Following Code was originally written to use a SS Micro as a simple code practice oscillator
 * (Other Arduino platforms should work as well, But as of this writing, IMO, the SS Micro is probably the most cost effective solution)
 * The input keying method can be either a straight Key, or a "Paddle" type Key
 * With the paddle key input the oscilator supports Iambic "A" mode.
 * Note the straight key uses a seperate input pin (D16) from the paddle inputs (D10 & D11).
 * For output the SS Micro can directly drive a 8 ohm speaker wired in serries with 200 ohms (pins D14 & 15)
 * Speaker volume wired this way is not loud, but ample for most code practice envirnoments 
 */
int wpm =12;// theorecctical sending speed ; measured in Words per Minute 
bool DitLast = false;
bool Running = false;
bool DitActive = false;
bool DahActive = false;
bool SpaceActive = false;
bool padDit = false;
bool padDah = false;
bool spdSwNO = false; // change to 'true' if your push button speed switches are are mormally open, or you're not using the speed change function
int DitPeriod; //measured in MilliSeonds; actual value calculated in SetUp section using WPM value
int TonePeriod;//measured in MicroSeconds; the final value is set based on the toneFerq value
int toneFreq = 600; //measured in hertz
int spdChngDelay=0;
unsigned long End;
unsigned long ok2Chk;

void setup() {
  //start serial connection
  //Serial.begin(9600); // for testing only
  //Configure the Speaker Pins, Digital Pins 14 & 15, as Outputs
  pinMode(14, OUTPUT);
  digitalWrite(14, LOW);
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
   //configure Key and Paddle Digital pins 16, 11 & 10 as inputs, and enable the internal pull-up resistor
  pinMode(16, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  //configure A2 as digital output [used in this case to drive a seperate CW decoder module]
  pinMode(A2, OUTPUT);
  digitalWrite(A2, HIGH);
  //Note switches I used are normally closed, so input goes HIGH when the user operates the switch
  pinMode(A0, INPUT_PULLUP);
  //digitalWrite(A0, HIGH);
  pinMode(A1, INPUT_PULLUP);
  //digitalWrite(A1, HIGH);
  DitPeriod = 1200/wpm;
  TonePeriod= (int)( 1000000.0/((float)toneFreq*2));
 
}

void loop() {
  //read input pins (Low means contact closure)  
  int Pin16Val = digitalRead(16);//straight Key input
  int Pin10Val = digitalRead(10); //Paddle "Dit" input
  int Pin11Val = digitalRead(11);//Paddle "Dah" input
  int PinA0Val = digitalRead(A0); //Speed Decrement input
  int PinA1Val = digitalRead(A1);//Speed Increment input
  if(spdSwNO){
    PinA0Val = !PinA0Val; 
    PinA1Val = !PinA1Val;
  }
  if (!Pin16Val){//if TRUE, Straight Key input is closed [to Ground])
   MakeTone();
   Running = false;
   DitActive = false;
   DahActive = false;
   SpaceActive = false;
  }
  if((SpaceActive & (millis()>ok2Chk))|| !Running ){ // 
    if(!Pin10Val) padDit = true;
    if(!Pin11Val) padDah = true;
  }
  if (!Running){// Check "paddle" status
    if (padDit & padDah){// if "TRUE" both input pins are closed, and we are in the "Iambic" mode
      if(DitLast) padDit = false;
      else padDah = false;
    }
    if (padDit & !DitActive ){
      DitActive = true;
      Running = true;
      padDit = false;
      End = (DitPeriod) + millis();
      //Serial.println(End);// testing only
    }
    else if (padDah & !DahActive ){
      DahActive = true;
      padDah = false;
      Running = true;
      End = (3*DitPeriod) +  millis();
    }
    //now check for speed change, But only we are actively sending stuff
    if (!Pin10Val || !Pin11Val){
      if(PinA0Val){ //user signaling to decrease speed
        spdChngDelay++;
        if(spdChngDelay==3){
          spdChngDelay=0;
          wpm--;
          if(wpm <8) wpm = 8;
          DitPeriod = 1200/wpm;
        }
      }
      if(PinA1Val){ //user signaling to increase speed
        spdChngDelay++;
        if(spdChngDelay==3){
          spdChngDelay=0;
          wpm++;
          if(wpm >30) wpm = 30;
          DitPeriod = 1200/wpm;
        }
      }
    }
  }
  else{// We are "running" (electronic keyer / Paddle mode is active)
    if (!SpaceActive){
      if (End > millis()){
          MakeTone();
        }
      else{
        End = (DitPeriod) +  millis();
        ok2Chk = millis()+ (2*DitPeriod/3);//start looking ahead to see which symbol (dit/dah) the user wants to send next; in this case, the micro will check paddle status 2/3 the way thru the space interval 
        SpaceActive = true;
        if (DitActive){
          DitActive = false;
          DitLast = true;
        }
        else{
          DahActive = false;
          DitLast = false;
        }
      }
    }
    else{ // SpaceActive is true;  So we want to remain silent for the lenght of a "dit"
       if (End > millis()){
        digitalWrite(A2, HIGH); //output to remote decoder
        digitalWrite(14, LOW);
        digitalWrite(15, LOW);
      }
      else{ //"dit" silence period met, so clear spaceActive flag 
        SpaceActive = false;
        Running = false;
      }
    }
    
 }// end of "Running" flag set 'true' (electronic keyer is active) code
 
  if(Pin16Val & !Running ){ //if the straight key input is "Open", and the Paddle mode isn't active, turn the speaker off
    digitalWrite(A2, HIGH); //output to remote decoder 
    digitalWrite(14, LOW);
    digitalWrite(15, LOW);
  }
 
  delayMicroseconds(TonePeriod);// pause for 1/2 a tone cycle before repeating the Loop.
}//End Main Loop

void MakeTone(){//Cycle the speaker pins
  digitalWrite(A2, LOW); //output to remote decoder
  digitalWrite(14, LOW);
  digitalWrite(15, HIGH);
  delayMicroseconds(TonePeriod);
  digitalWrite(14, HIGH);
  digitalWrite(15, LOW);
}
