# ArduinoCPO
Arduino Based Code Practice system; A scaleable project made from smaller building blocks
This project currently contains three seperate Arduino sketches
CWfftToneDecoder
cwKeyInterface

Morse_Decoder_Leonardo

A Fourth folder "FHT_128_channel_analyser" is included here (for convience)
It is copy of a "Processing" sketch that can be found elsewhere on the net.
(See http://wiki.openmusiclabs.com/wiki/Example)

The first two Arduino sketches are targeted at the SSMirco 
But other ATMEGA32U4 type boards should work; However, only the SS Micro form factor
has been tested. 

The "Morse_Decoder_Leonardo" as the name implies should work with Leonardo form-factor,
and will work with the UNO but with reduced performance.
The the TFT shield used in the video can be sourced from this site: 
https://www.ebay.com/itm/2-8-Inch-TFT-LCD-Display-Touch-Screen-Module-with-SD-Slot-For-Arduino-UNO-WRLU/292217750301?ssPageName=STRK%3AMEBIDX%3AIT&_trksid=p2057872.m2749.l2649
At this time, there are a number vendors selling simila TFT Displays sharing this same form factor, but using different
LCD drivers. The sites are often vague about what chip set their unit uses. Displays from other sources may or may not work.
The TFT-LCD library used in the "Morse_Decoder_Leonardo" (Nov 30, 2017) can be found here:
https://github.com/prenticedavid/MCUFRIEND_kbv
Curiously at the time I installed it, I had to uncomment a line in its header file before it would recognize the the TFT shield spec'd above.


 

 