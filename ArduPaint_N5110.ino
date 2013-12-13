/*
 * Arduino Paint with Nokia 5110 display and JoyStick Shield V1.A
 * 
 * Copyright 2013 Vitalii Sh. <vshmorgun@yandex.ru>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

//Defining connection points of the display Nokia5110
#define PIN_SCE   10
#define PIN_RESET 8
#define PIN_DC    13
#define PIN_SDIN  12
#define PIN_SCLK  11
//---------------------------------------------------
#define LCD_C     LOW
#define LCD_D     HIGH
#define LCD_CMD   0

//Screen resolution 
#define LCD_X     84     
#define LCD_Y     48
#define BL      9        //Display backlight pin



//Icons that indicate the status of the drawing
static const byte Symbols[][5] =
{
   {0x00, 0x0A, 0x04, 0x0A, 0x00} // 46 Fly mode icon
  ,{0x00, 0x00, 0x04, 0x00, 0x00} // 46 Drawing mode icon
  ,{0x00, 0x0E, 0x0E, 0x0E, 0x00} // 44 Black color icon
  ,{0x00, 0x0E, 0x0A, 0x0E, 0x00} // 43 White color icon

};

//Screnn buffer
byte screen_buffer[LCD_X][LCD_Y / 8];


int A=2, B=3, C=4, D=5, E=6, F=7, K=8; //Buttons pin
int buttonState = 0;
int xPos,yPos;       //Variable that determine the position of the drawing cursor
int jX = 0, jY = 0;  //Variables that determine the value of the slope joystick
byte pX = 0, pY = 0;

byte lostPixel;   
byte DrawMode = 0,ColorMode = 1; 

boolean BBL = true;  //Backlight state

//Clear display and data buffer
void LcdClear(void)
{
  for(byte k = 0;k <= LCD_Y/8;k++)
  {
    gotoXY(0,k);
    for(byte j = 0;j <= LCD_X;j++)
     {
       screen_buffer[j][k] = 0;
       LcdWrite(2,0);
       
     } 
  }
}

//Set position of cursor
void gotoXY(int x, int y)
{
  LcdWrite( 0, 0x80 | x);  // Column.
  LcdWrite( 0, 0X40 | y);  // Row.
}
 
//Function for drawing a pixel on stated position and color (white or black)
void DrawPixel(byte x, byte y, byte color){
  if ((x < 0) || (x >= LCD_X || (y < 0) || (y >= LCD_Y)))
    return;
    
  if(color)
    screen_buffer[x][(y/8)] |= _BV(y%8);
  else
    screen_buffer[x][(y/8)] &= ~_BV(y%8);
  Refresh_Box(x,y,x,y);
}

//Get pixel color
byte GetPixel(byte x, byte y) {
  if ((x < 0) || (x >= LCD_X) || (y < 0) || (y >= LCD_Y))
    return 0;

  return (screen_buffer[x][y/8] >> (y%8)) & 0x1;  
}

//This function redraw part of screen for the specified coordinates
void Refresh_Box(byte x_min,byte y_min, byte x_max, byte y_max)
{
  for(byte k = y_min/8;k <= y_max/8;k++)
  {
    gotoXY(x_min,k);
    for(byte j = x_min;j <= x_max;j++)
     {
       LcdWrite(1,screen_buffer[j][k]);
     } 
  }
}

//Draw icons on top left corner of screen, which indicate drawing mode
void DrawIcons()
{
  int n;
  gotoXY(0,0);
  if(DrawMode)  n = 1;
  else          n = 0;
  for (int j = 0; j < 5; j++)
  {
    screen_buffer[j][0] = Symbols[n][j];
  }

  if(ColorMode == 0)  n = 3;
  if(ColorMode == 1)  n = 2;
  
  for (int j = 0; j < 5; j++)
  {
    screen_buffer[j+5][0] = Symbols[n][j];
  }
  Refresh_Box(0,0,10,8);
}

//Write data or command to LCD
void LcdWrite(byte dc, byte data)
{
  digitalWrite(PIN_DC, dc);
  digitalWrite(PIN_SCE, LOW);
  shiftOut(PIN_SDIN, PIN_SCLK, MSBFIRST, data);
  digitalWrite(PIN_SCE, HIGH);
}

//Move drawing pixel on screen
void movepixel(int xMove, int yMove) {
  byte pX = xPos;
  byte pY = yPos;
  
  if(not(DrawMode)) DrawPixel(xPos,yPos,lostPixel);
  else
    if(not(ColorMode)) DrawPixel(xPos,yPos,0);
  yPos=yPos+yMove;
  xPos=xPos+xMove;
  if (xPos > (LCD_X-1))     xPos = LCD_X-1;
  if (xPos < 0)             xPos = 0; 
  if (yPos > (LCD_Y-1))     yPos = LCD_Y-1; 
  if (yPos < 0)             yPos = 0; 
  lostPixel = GetPixel(xPos,yPos);
  DrawPixel(xPos,yPos,1);
}

//Setting the initial configuration of the controller and the display initializes
void setup(void)
{
  Serial.begin(9600);      
  pinMode(A, INPUT_PULLUP); 
  pinMode(B, INPUT_PULLUP);      
  pinMode(C, INPUT_PULLUP);      
  pinMode(D, INPUT_PULLUP);      
  pinMode(E, INPUT_PULLUP);   //Enable the pull-up resistor on  button 
  pinMode(F, INPUT_PULLUP);
  pinMode(K, INPUT_PULLUP); 
  

  pinMode(PIN_SCE,   OUTPUT);
  pinMode(PIN_RESET, OUTPUT);
  pinMode(PIN_DC,    OUTPUT);
  pinMode(PIN_SDIN,  OUTPUT);
  pinMode(PIN_SCLK,  OUTPUT);
  pinMode(BL, OUTPUT);  
  
  digitalWrite(PIN_RESET, LOW);
  digitalWrite(PIN_RESET, HIGH);
  digitalWrite(BL, HIGH); //Turn on backlight

  //LCD initialize-----------------------------------------------  
  LcdWrite(LCD_CMD, 0x21);  // LCD Extended Commands.
  LcdWrite(LCD_CMD, 0xBb);  // Set LCD Vop (Contrast). //B1
  LcdWrite(LCD_CMD, 0x04);  // Set Temp coefficent. //0x04
  LcdWrite(LCD_CMD, 0x14);  // LCD bias mode 1:48. //0x13
  LcdWrite(LCD_CMD, 0x0C);  // LCD in normal mode. 0x0d for inverse
  LcdWrite(LCD_C, 0x20);
  LcdWrite(LCD_C, 0x0C);
  //---------------------------------------------------------------
  
  LcdClear();      //Clear display
  gotoXY(0,0);     //Set cursor position
  DrawIcons();     //Draw status icons
  yPos = 20;       //setting the initial position of drawing cursor
  xPos = 40;       //----
  DrawPixel(xPos,yPos,1);    //Draw pixel which is drawing cursor
  
}

void loop(void)
{
//----------------------------------------------------------------------------------------->>>>>
//Block listening state of buttons--------------------------------------------------------->>>>>
  buttonState = digitalRead(K);
  if (buttonState != 1) {
    if(DrawMode)
      DrawMode = 0;
    else
      DrawMode = 1;
    DrawIcons();
    while(buttonState != 1){  //Waiting the button is released
      buttonState = digitalRead(K);
    }
  }
  buttonState = digitalRead(A);
  if (buttonState != 1)
  {
    if(ColorMode)
      ColorMode = 0;
    else
      ColorMode = 1;
    while(buttonState != 1){  //Waiting the button is released
      buttonState = digitalRead(A);
    }  
    DrawIcons();  
  }
  
  
  //Disabling and enabling of backlight
  buttonState = digitalRead(F);
  if (buttonState != 1) {
    if (BBL){ 
      BBL = false;
      digitalWrite(BL, LOW);  
    }
    else { 
      BBL = true; 
      digitalWrite(BL, HIGH);
    }
    while(buttonState != 1){  //Waiting the button is released
      buttonState = digitalRead(F);
    }
  }  
  buttonState = digitalRead(E);
  if (buttonState != 1)
  {
    LcdClear();
    gotoXY(0,0);
    DrawIcons();
  }
//End of block listening state of buttons--------------------------------------------------<<<<<
//-----------------------------------------------------------------------------------------<<<<<  
  
  //Getting joystick position and write this data to variables
  jX = (analogRead(0)-512); 
  jY = (analogRead(1)-512);
  
  movepixel(jX/500,-jY/500); //Move the cursor position, if the joystick is inclined enough
 
  delay(100);

}
