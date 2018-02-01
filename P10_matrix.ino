#include <RTClib.h>
#include <Wire.h>
#include <SPI.h>
#define A_pin PA0
#define B_pin PA1
#define SCLK PA2
#define OE PA3
SPIClass SPI_2(2); //Create an SPI2 object.
RTC_DS3231 rtc;
bool update_ready=0;
DateTime now;
uint32_t last_millis;
int32_t buffer[16];
const byte nums[12][8] = {
  {
  B00000000,
  B00111100,
  B01100110,
  B01100110,
  B01100110,
  B01100110,
  B01100110,
  B00111100
},
{
  B00000000,
  B00011000,
  B00011000,
  B00111000,
  B00011000,
  B00011000,
  B00011000,
  B01111110
},{
  B00000000,
  B00111100,
  B01100110,
  B00000110,
  B00001100,
  B00110000,
  B01100000,
  B01111110
},{
  B00000000,
  B00111100,
  B01100110,
  B00000110,
  B00011100,
  B00000110,
  B01100110,
  B00111100
},{
  B00000000,
  B00001100,
  B00011100,
  B00101100,
  B01001100,
  B01111110,
  B00001100,
  B00001100
},{
  B00000000,
  B01111110,
  B01100000,
  B01111100,
  B00000110,
  B00000110,
  B01100110,
  B00111100
},{
  B00000000,
  B00111100,
  B01100110,
  B01100000,
  B01111100,
  B01100110,
  B01100110,
  B00111100
},{
  B00000000,
  B01111110,
  B01100110,
  B00001100,
  B00001100,
  B00011000,
  B00011000,
  B00011000
},{
  B00000000,
  B00111100,
  B01100110,
  B01100110,
  B00111100,
  B01100110,
  B01100110,
  B00111100
},{
  B00000000,
  B00111100,
  B01100110,
  B01100110,
  B00111110,
  B00000110,
  B01100110,
  B00111100
},{
  B00000000,
  B00000000,
  B00000000,
  B00011000,
  B00011000,
  B00000000,
  B00000000,
  B00000000
},
{
  B00000100,
  B00000100,
  B00001000,
  B00001000,
  B00010000,
  B00010000,
  B00100000,
  B00100000
}
};
void setup()
{
  for(int i=0;i<16;i++) buffer[i]=0x00000000;
  pinMode(A_pin,OUTPUT);
  pinMode(B_pin,OUTPUT);
  pinMode(SCLK,OUTPUT);
  pinMode(OE,OUTPUT);
  SPI_2.begin(); //Initiallize the SPI 2 port.
  //SPI_2.setBitOrder(MSBFIRST); // Set the SPI-2 bit order (*) 
  //SPI_2.setDataMode(SPI_MODE0); //Set the  SPI-2 data mode (**) 
  //Set the SPI speed
  SPI_2.setClockDivider(SPI_CLOCK_DIV2);    // Slow speed (72 / 64 = 1.125 MHz SPI speed) 
  rtc.begin();
}
void loop()
{ 
 //if(abs(millis()-last_millis)>200) 
 //{
  now=rtc.now();
  //last_millis=millis();
  disp_time(now.hour(),now.minute(),now.day(),now.month(),now.second());
 print_matrix(buffer);
  clear_buffer(buffer);
}
void setRow(int8_t row)
{
  if(row<0|| row>3) return;
  switch(row)
  {
    case 0:
    {
      digitalWrite(A_pin,0);
      digitalWrite(B_pin,0);
      break;
    }
    case 1:
    {
      digitalWrite(A_pin,1);
      digitalWrite(B_pin,0);
      break;
    }
    case 2:
    {
      digitalWrite(A_pin,0);
      digitalWrite(B_pin,1);
      break;
    }
    case 3:
    {
      digitalWrite(A_pin,1);
      digitalWrite(B_pin,1);
      break;
    }
  }
}
void print_matrix(int32_t * data)
{
  for(int i=0;i<4;i++)
  {
    clear_matrix();
    digitalWrite(OE,0);
    digitalWrite(SCLK,0);
    setRow(i);
    for(int j=24;j>=0;j-=8)
    {
    SPI_2.transfer(~data[i+12]>>j);
    SPI_2.transfer(~data[i+8]>>j);
    SPI_2.transfer(~data[i+4]>>j);
    SPI_2.transfer(~data[i]>>j);
    }
    digitalWrite(SCLK,1);
    digitalWrite(OE,1);
    if(now.hour()<5||now.hour()>22)delayMicroseconds(20);
    else delayMicroseconds(50);
    digitalWrite(OE,0);
    delay(1);
  }
}
void clear_matrix()
{
  digitalWrite(OE,0);
  digitalWrite(SCLK,0);
  for(int j=0;j<16;j++) SPI_2.transfer(0xff);
  digitalWrite(OE,1);
  digitalWrite(SCLK,1);
}
void clear_buffer(int32_t * data)
{
  for(int i=0;i<16;i++)data[i]=0x00000000;
}
void write_number(int32_t * data, int x, int y, int num)
{
  uint32_t temp;
  for(int i=0;i<=7;i++)
  {
    
    temp=nums[num][i];
    if(x>24)
    {
      data[i+y<17?y+i:16]|=temp>>x-24;
    }
    if(x<24 )data[i+y<17?y+i:16]|=temp<<24-x;
    
  }
}
void disp_time(int hour, int minute, int day, int month,int second)
{
  if(hour<10) write_number(buffer,6,-1, hour);
  else
  {
    write_number(buffer,-1,-1,hour/10);
    write_number(buffer,6,-1,hour%10);
  }
  //if(minute<10) write_number(buffer,16,-1, minute);
  //else
  //{
    write_number(buffer,18,-1,minute/10);
    write_number(buffer,25,-1,minute%10);
  //}
  if(day<10) write_number(buffer,6,8, day);
  else
  {
    write_number(buffer,-1,8,day/10);
    write_number(buffer,6,8,day%10);
  }
  if(month<10) write_number(buffer,18,8, month);
  else
  {
    write_number(buffer,18,8,month/10);
    write_number(buffer,25,8,month%10);
  }
  if(second%2)
  {
    write_number(buffer,12,-2,10);
    write_number(buffer,12,1,10);
  }
  write_number(buffer,12,8,11);
}



















