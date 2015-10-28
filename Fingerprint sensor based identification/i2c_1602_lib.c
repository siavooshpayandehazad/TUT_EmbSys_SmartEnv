#include <msp430.h>
#include <string.h>
#include "i2c_1602_lib.h"

#define LED_ON
#define LED_OFF

void lcddelay(){
  int i=0;
  int j=0;
  for(i=0; i<I2CLCD_DELAY; i++) for(j=0; j<I2CLCD_DELAY; j++)I2C_1602_SDA_READ;
}

void i2c_start(){
	I2C_1602_SDA_LOW;
	I2C_1602_DELAY_COMMAND;
	I2C_1602_SCL_LOW;
	I2C_1602_DELAY_COMMAND;
	return;
}

void i2c_flickclock(){
	I2C_1602_SCL_HIGH;
	I2C_1602_DELAY_COMMAND;
	I2C_1602_SCL_LOW;
	I2C_1602_DELAY_COMMAND;
	return;
}

void i2c_stop(){
    I2C_1602_SDA_LOW;
    I2C_1602_DELAY_COMMAND;
	I2C_1602_SCL_HIGH;
	I2C_1602_DELAY_COMMAND;
	I2C_1602_SDA_HIGH;
	I2C_1602_DELAY_COMMAND;
	return;
}
int i2c_sendbyte(int data){
	int i;
	int ack=0; //0 is good
	for(i=0; i<8; i++){
		if(data&1)I2C_1602_SDA_HIGH;
		else I2C_1602_SDA_LOW;
		I2C_1602_DELAY_COMMAND;
		i2c_flickclock();
		data=data>>1;
	}
	//check ack here if necessary
        I2C_1602_SDA_SETINPUT;
        I2C_1602_SDA_HIGH;
        I2C_1602_SCL_HIGH;
        if(I2C_1602_SDA_READ)ack=1;
        I2C_1602_SCL_LOW;
        I2C_1602_SDA_SETOUTPUT;
        //ack=1; //testing what if always bad
	return ack;
}

void lcdshift(int output){ //LED, D4, D5, D6, D7, RS, RW, BL //our address: 4Eh (write address) //this function is recent development and is fucked up
//note et kõik port registrid on inverted
        int flipped_address = 0;
	unsigned int output_translated=0, output_translated_enter=0; //(GP7)RS,E,DB4,DB5,DB6,DB7,LITE (GP1) (GP0 free)
	//if(output&0b10000000)output_translated|=0b; //this is that extra LED, we don't have that here
	
	
	//previous faulty crap
	/*if(output&0b01000000)output_translated|=0b00100000; //D4
	if(output&0b00100000)output_translated|=0b00010000; //D5
	if(output&0b00010000)output_translated|=0b00001000; //D6
	if(output&0b00001000)output_translated|=0b00000100; //D7
	if(output&0b00000100)output_translated|=0b10000000; //RS
	//if(output&0b00000010)output_translated|=0b; //RW //RW HARDWIRED!
	if(output&0b00000001)output_translated|=0b00000010; //BL*/
	
	if(output&0b01000000)output_translated|=0b00001000; //D4
	if(output&0b00100000)output_translated|=0b00000100; //D5
	if(output&0b00010000)output_translated|=0b00000010; //D6
	if(output&0b00001000)output_translated|=0b00000001; //D7
	if(output&0b00000100)output_translated|=0b10000000; //RS
	if(output&0b00000010)output_translated|=0b01000000; //RW
	if(output&0b00000001)output_translated|=0b00010000; //BL
	
	//output_translated=output;//FOR DEBUGGERY!!!!!!!
	
        output_translated_enter=output_translated;
        //output_translated_enter|=0b01000000; //previous faulty shit
		output_translated_enter|=0b00100000;
	//output_translated^=0b11111111; //bcuz output inverted!
    //    output_translated_enter^=0b11111111; //bcuz output inverted!
        
	
        if(I2C_1602_ADDRESS&0b10000000)flipped_address|=0b00000001;
        if(I2C_1602_ADDRESS&0b01000000)flipped_address|=0b00000010;
        if(I2C_1602_ADDRESS&0b00100000)flipped_address|=0b00000100;
        if(I2C_1602_ADDRESS&0b00010000)flipped_address|=0b00001000;
        if(I2C_1602_ADDRESS&0b00001000)flipped_address|=0b00010000;
        if(I2C_1602_ADDRESS&0b00000100)flipped_address|=0b00100000;
        if(I2C_1602_ADDRESS&0b00000010)flipped_address|=0b01000000;
        if(I2C_1602_ADDRESS&0b00000001)flipped_address|=0b10000000;
        
        i2c_start();
	if(i2c_sendbyte(flipped_address)){ //mothafuckin adre
         //if we get nack
           LED_ON;
           lcddelay();
           LED_OFF;
           lcddelay();
           LED_ON;
           lcddelay();
           LED_OFF;
           lcddelay();
           LED_ON;
           lcddelay();
           LED_OFF;
           lcddelay();
        } 
        //i2c_sendbyte(output_translated); //command
        i2c_sendbyte(output_translated_enter);
        i2c_sendbyte(output_translated); 
        
	i2c_stop();
	return;
}

void lcdsend(int RS, int RW, int eight, int BL, int LED){ //8bitdata D7 D6 D5 D4 D3 D2 D1 D0 //this works fo sho
  int output=0; int i=0;
  for(i=0; i<2; i++){
    if(RS>0) output|=0b00000100;
    if(RW>0) output|=0b00000010;
    if(BL>0) output|=0b00000001;
    if(LED>0) output|=0b10000000;
    if(i==0){
      if(eight&0b10000000) output|=0b00001000;
      if(eight&0b01000000) output|=0b00010000;
      if(eight&0b00100000) output|=0b00100000;
      if(eight&0b00010000) output|=0b01000000;
    }
    else{
      if(eight&0b00001000) output|=0b00001000;
      if(eight&0b00000100) output|=0b00010000;
      if(eight&0b00000010) output|=0b00100000;
      if(eight&0b00000001) output|=0b01000000;
    }
    lcdshift(output);
    output=0;
  }
  return;  
}

void lcdword(char *text){ //lcdword("Denry");
  int i, len;
  len=strlen(text);
  //lcdsend(1,0,len+48,1,0);
  for(i=0; i<len; i++){
    lcdsend(1,0,text[i], 1, 0);
  }
  return;
}

void lcdnr(int nr, int spaces){
  int x=0;
  while(nr>9999){
    nr-=10000;
    x++;
  }
  if(spaces>4)lcdsend(1,0,x+48,1,0);
  x=0;
  while(nr>999){
    nr-=1000;
    x++;
  }
  if(spaces>3)lcdsend(1,0,x+48,1,0);
  x=0;
  while(nr>99){
    nr-=100;
    x++;
  }
  if(spaces>2)lcdsend(1,0,x+48,1,0);
  x=0;
  while(nr>9){
    nr-=10;
    x++;
  }
  if(spaces>1)lcdsend(1,0,x+48,1,0);
  lcdsend(1,0,nr+48,1,0);
  return;
}

void lcdcursor(int x, int y){
  lcdsend(0,0,0b00000010, 1, 0); //returnhome
  x=x+40*y;
  while (x>0){
    lcdsend(0,0,0b00010100, 1, 0); //increment
    x--;
  }
  return;
}

void lcdclear(){
	lcdcursor (0,0);
	lcdword("                ");
	lcdcursor (0,1);
	lcdword("                ");
	return;
}

void initLCD(){
  I2C_1602_SDA_SETOUTPUT; 
  I2C_1602_SCL_SETOUTPUT;
  I2C_1602_SDA_HIGH;
  I2C_1602_SCL_HIGH;
  I2C_1602_SDA_PULLUP;
//  printf("Pins set inputs/outputs \n");
  lcdshift(0b00000000);
//lcdshift(0b); //LED, D4, D5, D6, D7, RS, RW, BL
  lcdshift(0b00100001);//set 4bit init
//  printf("LCDshift done \n");
  lcdsend(0, 0, 0b00101000, 1, 0);//two line disp
//  printf("LCDSend 1/4 done \n");
  lcdsend(0,0,0b00001110, 1, 0); //turn on disp & cursor
 // printf("LCDSend 2/4 done \n");
  lcdsend(0,0,0b00000110, 1, 0);//entry mode set
  //printf("LCDSend 3/4 done \n");
  lcdsend(0,0,0b00000010, 1, 0); //returnhome
  //printf("LCDSend 4/4 done \n");

  lcdclear(); //this is slow as fuck for some reason
 // printf("LCDclear done \n");
  lcdcursor(0, 0);
 // printf("Cursor set 0,0 \n");
}
