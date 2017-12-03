# ArduinoCPO
Arduino Based Code Practice system; A scale-able project made from smaller building blocks

For an overview, see YouTube video found at: https://youtu.be/U1LHWX86INQ

This project currently contains three separate Arduino sketches:

cwKeyInterface

Morse_Decoder_Leonardo

CWfftToneDecoder

A Fourth folder "FHT_128_channel_analyzer" is included here (for convenience)
It is copy of a "Processing" sketch that can be found elsewhere on the net.
(See http://wiki.openmusiclabs.com/wiki/Example)

The first & last sketch are targeted at the SSMirco. 
But other ATMEGA32U4 type boards should work; However, at this tme, only the SS Micro form factor
has been tested. 

The "Morse_Decoder_Leonardo" as the name implies is intended to work with Leonardo form-factor.
And will also work with the UNO, but with reduced performance.
The TFT Display shield used in the video can be sourced from this site:

https://www.ebay.com/itm/2-8-Inch-TFT-LCD-Display-Touch-Screen-Module-with-SD-Slot-For-Arduino-UNO-WRLU/292217750301?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649

At this time, there are a number vendors selling TFT Displays that share this same form factor, but use different
LCD drivers. The sites are often vague about what chip set they are using. Do your homework before using displays from other sources, as they may or may not work.
The TFT-LCD library used in the "Morse_Decoder_Leonardo" (Nov 30, 2017) can be found here:
https://github.com/prenticedavid/MCUFRIEND_kbv
Curiously at the time I installed it, I found I had to “uncomment” a line in its header file before it would recognize the the TFT shield spec'd above.

Also Its not Mentioned in the video. But might be of interest, the CWfftToneDecoder sketch contains a rudimentary CW Decoder (fixed at 18 WPM). So this sketch by itself (plus the Arduino's IDE/Serial Monitor) is all thats need to display ARRL code bulletins 

 

 
