# ArduinoCPO
Arduino Based Code Practice system; A scale-able project made from smaller building blocks

For an overview, see YouTube video found at: https://youtu.be/U1LHWX86INQ

This project currently contains four separate Arduino sketches:

cwKeyInterface

Morse_Decoder_LeonardoR1

CWfftToneDecoder

GoertzelTry04

A Fifth folder "FHT_128_channel_analyzer" is included here (for convenience)
It is a copy of the "Processing" sketch that can be found elsewhere on the net.
(See http://wiki.openmusiclabs.com/wiki/Example)

The first & last 2 sketches are targeted at the SSMirco. 
But other ATMEGA32U4 type boards could work; However, at this time, only the SS Micro form factor
has been tested. 

Today, the GoertzelTry04 is the preferred tone detector sketch. It is not compatible with the processing
sketch mentioned above. Instead to analyze/visualize this sketch's performance, use the Arduino's serial
plot tool.

The CWfftToneDecoder sketch is still here, because of its relavance to the original YouTube video
 

The "Morse_Decoder_LeonardoR1" as the name implies is intended to work with Leonardo form-factor.
The sketch can also work with the UNO, but with reduced performance. Please, do not use the UNO board if replicating 
the "stand-alone" KW4KD(2019) decoder. There is no support for that configuration.
The TFT Display shield used in the video can be sourced from this site:

https://www.ebay.com/itm/2-8-Inch-TFT-LCD-Display-Touch-Screen-Module-with-SD-Slot-For-Arduino-UNO-WRLU/292217750301?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649

At this time, there are a number vendors selling TFT Displays that share this same form factor, but use different
LCD drivers. The sites are often vague about what chip set they are using. Do your homework before using displays from other sources, as they may or may not work.
The TFT-LCD library used in the "Morse_Decoder_Leonardo" (Nov 30, 2017) can be found here:
https://github.com/prenticedavid/MCUFRIEND_kbv
Curiously at the time I installed it, I found I had to “uncomment” a line in its header file before it would recognize the the TFT shield spec'd above.

Also while its not Mentioned in the video, but might be of interest, the CWfftToneDecoder sketch contains a rudimentary CW Decoder (fixed at 18 WPM). So this sketch by itself (plus the Arduino's IDE/Serial Monitor) is all that's need to display ARRL code bulletins 
 

 
