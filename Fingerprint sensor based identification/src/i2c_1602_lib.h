#ifndef I2C_1602_LIB_H
#define I2C_1602_LIB_H

//CHANGE FROM HERE
#define I2C_1602_SCL_PORT_OUT P2OUT
#define I2C_1602_SCL_PORT_DIR P2DIR
#define I2C_1602_SDA_PORT_OUT P2OUT
#define I2C_1602_SDA_PORT_DIR P2DIR
#define I2C_1602_SDA_PORT_REN P2REN
#define I2C_1602_SDA_PORT_IN P2IN
#define I2C_1602_SCL_PIN 4
#define I2C_1602_SDA_PIN 5

#define I2CLCD_DELAY 0 //you can make it smaller, but at one point LCD may start failing because you are going too fast. try.
//DON'T CHANGE BELOW, STOP HERE


//#define I2C_1602_DELAY_COMMAND delayMicroseconds(100)
#define I2C_1602_DELAY_COMMAND lcddelay()

#define I2C_1602_SDA_HIGH I2C_1602_SDA_PORT_OUT |= (1<<I2C_1602_SDA_PIN)
#define I2C_1602_SDA_LOW I2C_1602_SDA_PORT_OUT &= ~(1<<I2C_1602_SDA_PIN)
#define I2C_1602_SCL_HIGH I2C_1602_SCL_PORT_OUT |= (1<<I2C_1602_SCL_PIN)
#define I2C_1602_SCL_LOW I2C_1602_SCL_PORT_OUT &= ~(1<<I2C_1602_SCL_PIN)

//#define I2C_1602_SDA_PULLUP	I2C_1602_SDA_PORT_REN |= (1<<I2C_1602_SDA_PIN)
#define I2C_1602_SDA_PULLUP
#define I2C_1602_SDA_SETOUTPUT I2C_1602_SDA_PORT_DIR |= (1<<I2C_1602_SDA_PIN)
#define I2C_1602_SDA_SETINPUT I2C_1602_SDA_PORT_DIR &= ~(1<<I2C_1602_SDA_PIN)
#define I2C_1602_SCL_SETOUTPUT I2C_1602_SCL_PORT_DIR |= (1<<I2C_1602_SCL_PIN)

#define I2C_1602_SDA_READ I2C_1602_SDA_PORT_IN & (1<<I2C_1602_SDA_PIN)
#define I2C_1602_ADDRESS 0x4E

extern void lcddelay();
extern void i2c_start();
extern void i2c_flickclock();
extern void i2c_stop();
extern int i2c_sendbyte(int data);
extern void lcdshift(int output); //LED,BL,RW,RS,D7,D6,D5,D4 <- see on outdated jama! // v6i siiski!
extern void lcdsend(int RS, int RW, int eight, int BL, int LED);
extern void lcdcursor(int x, int y);
extern void lcdword(char *text);
extern void lcdnr(int nr, int spaces);
extern void lcdclear();
extern void initLCD();

#endif
