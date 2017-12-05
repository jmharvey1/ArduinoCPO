/*  FHT_128_channel_analyser.pde
   (Modified from original sketch, referenced below, & adapted to better view
    the 64 channels used in the Arduino CWfftToneDecoderR1 sketch - KW4KD 20171205)

    an open-source display for spectrum analyser
    Copyright (C) 2013  Jürgen Rimmelspacher

    For use with "processing": http://processing.org/

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

final int X_OFFSET  =  40;                     // x-distance to left upper corner of window
final int Y_OFFSET  =  60;                     // y-distance to left upper corner of window
final int BOT_DIST  =  80;                     // distance to bottom line of window
final int COL_WIDTH =   8;                     // column widt
final int Y_DIST    =  64;                     // distance horizontal lines
final int X_DIST    =   10;                     // distance vertical lines
final int X_MAX     = (64+1)*X_DIST+1;        // x-axis lenght (854)
final int Y_MAX     = 256;                     // y-axis lenght
final int X_WINDOW  = X_MAX + 2*X_OFFSET;      // window width
final int Y_WINDOW  = Y_MAX+BOT_DIST+Y_OFFSET; // window height
final int X_ENUM    = 10;
PFont fontA;
color graphColor = color(25, 25, 250);
PFont fontGraph;
import processing.serial.*;
Serial port;
int[] inBuffer = new int[64];

void draw_grid()                               // draw grid an title
{ 
  int count=0;

  background(200);
  stroke(0);
  for (int x=0+X_DIST; x<=X_MAX; x+=X_DIST)    // vertical lines
  {
    if ( X_ENUM == count || 0 == count)
    { 
      line (x+X_OFFSET, Y_OFFSET, x+X_OFFSET, Y_MAX+Y_OFFSET+10);
      count=0;
    }
    else
    {
      line (x+X_OFFSET, Y_OFFSET, x+X_OFFSET, Y_MAX+Y_OFFSET);
    }    
    count++;
  }
  for (int y=0; y<=Y_MAX; y+=Y_DIST)           // horizontal lines 
  {
    line (X_OFFSET, y+Y_OFFSET, X_MAX+X_OFFSET, y+Y_OFFSET);
    textFont(fontA, 16);
    text( (Y_MAX-y), 7, y+Y_OFFSET+6);
  }
  textFont(fontA, 32);
  fill(graphColor); 
  text("64-Channel Spectrum Analyser", 215, 40);
  textFont(fontA, 16);
  text("magnitude", 7, 20);  
  text("(8bit-value)", 7, 40);  
  text("--> channel (number i)", X_OFFSET, Y_WINDOW-Y_OFFSET/2);
  text(" frequency   f ( i ) = i * SAMPLE_RATE / FHT_N ", 350, Y_WINDOW-Y_OFFSET/2 );
} 

void serialEvent(Serial p)                      // ISR triggerd by "port.buffer(129);"
{ 
  if ( 255 == port.read() )                     //  1 start-byte               
  {
    inBuffer = int( port.readBytes() );         // 128 data-byte
  }
}

void setup() 
{ 
  size( 854, 396);
  //size(X_WINDOW, Y_WINDOW);                      // size of window
  noStroke();
  fontGraph = loadFont("ArialUnicodeMS-48.vlw");
  textFont(fontGraph, 12);
  println(Serial.list());                        // show available COM-ports
  port = new Serial(this, "COM10", 115200);
  port.buffer(129);                              // 1 start-byte + 128 data-bytes 
  fontA = loadFont("ArialUnicodeMS-48.vlw");
  textFont(fontA, 16);
}

void draw() 
{ 
  int count=0;

  draw_grid();

  for (int i=0; i<64; i++)
  { 
    fill(graphColor);
    rect(i*X_DIST+X_OFFSET+X_DIST-COL_WIDTH/2, height-BOT_DIST, COL_WIDTH, -inBuffer[i]);
    if ( X_ENUM == count || 0 == count)
    { 
      text(i, (i+1)*X_DIST+X_OFFSET-COL_WIDTH/2, height-BOT_DIST+25);
      count=0;
    }
    count++;
  }
}