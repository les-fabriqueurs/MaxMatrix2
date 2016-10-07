/*
 * MaxMatrix2
 * 
 * Version 0.1
 * Update of MaxMatrix Version 1.0 Feb 2013 Copyright 2013 Oscar Kin-Chung Au
 * 
 */


#include "Arduino.h"
#include "MaxMatrix2.h"

MaxMatrix::MaxMatrix(byte _data, byte _load, byte _clock, byte _num) 
{
	data = _data;
	load = _load;
	clock = _clock;
	num = _num;
	for (int i=0; i<80; i++)
		buffer[i] = 0;
}

void MaxMatrix::init()
{
	pinMode(data,  OUTPUT);
	pinMode(clock, OUTPUT);
	pinMode(load,  OUTPUT);
	digitalWrite(clock, HIGH); 

	setCommand(max7219_reg_scanLimit, 0x07);      
	setCommand(max7219_reg_decodeMode, 0x00);  // using an led matrix (not digits)
	setCommand(max7219_reg_shutdown, 0x01);    // not in shutdown mode
	setCommand(max7219_reg_displayTest, 0x00); // no display test
	
	// empty registers, turn all LEDs off
	clear();
	
	setIntensity(0x0f);    // the first 0x0f is the value you can set
}

void MaxMatrix::setIntensity(byte intensity)
{
	setCommand(max7219_reg_intensity, intensity);
}

void MaxMatrix::clear()
{
	for (int i=0; i<8; i++) 
		setRowAll(i,0);
		
	for (int i=0; i<80; i++)
		buffer[i] = 0;
}



void MaxMatrix::setCommand(byte command, byte value)
{
	digitalWrite(load, LOW);    
	for (int i=0; i<num; i++) 
	{
		shiftOut(data, clock, MSBFIRST, command);
		shiftOut(data, clock, MSBFIRST, value);
	}
	digitalWrite(load, LOW);
	digitalWrite(load, HIGH);
}


void MaxMatrix::setRowAll(byte row, byte value)
{
	digitalWrite(load, LOW);    
	for (int i=0; i<num; i++) 
	{
		shiftOut(data, clock, MSBFIRST, row + 1);
		shiftOut(data, clock, MSBFIRST, value);
		buffer[row * i] = value;
	}
	digitalWrite(load, LOW);
	digitalWrite(load, HIGH);
}


// rewritten for rotated matrix
void MaxMatrix::setDot(byte col, byte row, byte value)
{
  // target matrix number (from 0 to num-1 for "in display" dot) 
  int n = col/8;
  // virtual row number - (from 0 to 8*num-1) - 8 rows for each matrix
  int vr = n * 8 + row;
  // conversion from display to matrix col number (from 0 to 7)  
  int vc = 7 - (col % 8);

  bitWrite(buffer[vr], vc, value);
    
	digitalWrite(load, LOW);    
	for (int i=0; i<num; i++) 
	{
		if (i == n)
		{
			// Send the number of the line to modify 
			shiftOut(data, clock, MSBFIRST, row+1);
			// Send the byte to display on that line
			shiftOut(data, clock, MSBFIRST, buffer[vr]);
		}
		else
		{
			shiftOut(data, clock, MSBFIRST, 0);
			shiftOut(data, clock, MSBFIRST, 0);
		}
	}
	digitalWrite(load, LOW);
	digitalWrite(load, HIGH);
}


 // rewritten for rotated matrix
 // to update for optimization (setRow)
void MaxMatrix::writeSprite(int x, int y, const byte* sprite)
{
	int w = sprite[0];
	int h = sprite[1];
	
//	if (h == 8 && y == 0)
//		for (int i=0; i<w; i++)
//		{
//			int c = x + i;
//			if (c>=0 && c<80)
//				setColumn(c, sprite[i+2]);
//		}
//	else
		for (int i=0; i<w; i++)
			for (int j=0; j<h; j++)
			{
				int c = x + i;
				int r = y + j;
				if (c>=0 && c<80 && r>=0 && r<8)
					setDot(c, r, bitRead(sprite[i+2], j));
			}
}

 // rewritten for rotated matrix
void MaxMatrix::reload()
{
	for (int i=0; i<8; i++)
	{
		int row = i;
		digitalWrite(load, LOW);    
		for (int j=0; j<num; j++) 
		{
			shiftOut(data, clock, MSBFIRST, i + 1);
			shiftOut(data, clock, MSBFIRST, buffer[row]);
			row += 8;
		}
		digitalWrite(load, LOW);
		digitalWrite(load, HIGH);
	}
}


// rewritten for rotated matrix
void MaxMatrix::shiftLeft(bool rotate, bool fill_zero)
{
	//byte old = buffer[0];
	//int i;
	//for (i=0; i<80; i++)
		//buffer[i] = buffer[i+1];
	//if (rotate) buffer[num*8-1] = old;
	//else if (fill_zero) buffer[num*8-1] = 0;
  
  int i;
  byte old;
	for (i=0; i<80; i++)
  {
     if (i < 8)
       bitWrite(old, 7, buffer[i]);      
    buffer[i] = buffer[i] << 1; 
    if (i<72)   
      bitWrite(buffer[i],0 ,bitRead(buffer[i+8],7)); 
    else
      bitWrite(buffer[i],0 ,0);
  }
   
  if (rotate)
  {
    for (i=0; i<8; i++)
      bitWrite(buffer[(num*8)-8 + i], 0, bitRead(old,i));
  }  
  
	reload();
}




