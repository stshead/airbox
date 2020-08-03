#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include "sh1106.h"

#define CHANNEL 0
#define VCCSTATE SH1106_SWITCHCAPVCC
#define WIDTH  128
#define HEIGHT  64
#define NUM_PAGE 8

#define RST 25
#define DC  24
unsigned char buffer[1024];

void command(unsigned char cmd){
    digitalWrite(DC, LOW);
    wiringPiSPIDataRW(CHANNEL, &cmd, 1);
}

void SH1106_begin()
{
	pinMode(RST,OUTPUT);
	pinMode(DC,OUTPUT);
	wiringPiSPISetup(CHANNEL, 2000000);    //2M
	
	digitalWrite(RST,HIGH);
	delay(10);
	digitalWrite(RST,LOW);
	delay(10);
	digitalWrite(RST,HIGH);

    command(0xAE);//--turn off oled panel
    command(0x02);//---set low column address
    command(0x10);//---set high column address
    command(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    command(0x81);//--set contrast control register
    command(0xA0);//--Set SEG/Column Mapping     
    command(0xC0);//Set COM/Row Scan Direction   
    command(0xA6);//--set normal display
    command(0xA8);//--set multiplex ratio(1 to 64)
    command(0x3F);//--1/64 duty
    command(0xD3);//-set display offset    Shift Mapping RAM Counter (0x00~0x3F)
    command(0x00);//-not offset
    command(0xd5);//--set display clock divide ratio/oscillator frequency
    command(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
    command(0xD9);//--set pre-charge period
    command(0xF1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    command(0xDA);//--set com pins hardware configuration
    command(0x12);
    command(0xDB);//--set vcomh
    command(0x40);//Set VCOM Deselect Level
    command(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
    command(0x02);//
    command(0xA4);// Disable Entire Display On (0xa4/0xa5)
    command(0xA6);// Disable Inverse Display On (0xa6/a7) 
    command(0xAF);//--turn on oled panel
}
void SH1106_clear()
{
	int i;
	for(i = 0;i<sizeof(buffer);i++)
	{
		buffer[i] = 0;
	}
}
void SH1106_pixel(int x,int y,char color)
{
    if(x > WIDTH || y > HEIGHT)return ;
    if(color)
        buffer[x+(y/8)*WIDTH] |= 1<<(y%8);
    else
        buffer[x+(y/8)*WIDTH] &= ~(1<<(y%8));
}
void SH1106_char1616(unsigned char x,unsigned char y,unsigned char chChar)
{
	unsigned char i, j;
	unsigned char chTemp = 0, y0 = y, chMode = 0;

	for (i = 0; i < 32; i ++) {
		chTemp = Font1612[chChar - 0x30][i];
		for (j = 0; j < 8; j ++) {
			chMode = chTemp & 0x80? 1 : 0; 
			SH1106_pixel(x, y, chMode);
			chTemp <<= 1;
			y ++;
			if ((y - y0) == 16) {
				y = y0;
				x ++;
				break;
			}
		}
	}
}

void SH1106_char3216(unsigned char x,unsigned char y,unsigned char chChar)
{
	unsigned char i, j;
	unsigned char chTemp = 0, y0 = y, chMode = 0;

	for (i = 0; i < 64; i ++) {
		chTemp = Font3216[chChar - 0x30][i];
		for (j = 0; j < 8; j ++) {
			chMode = chTemp & 0x80? 1 : 0; 
			SH1106_pixel(x, y, chMode);
			chTemp <<= 1;
			y ++;
			if ((y - y0) == 32) {
				y = y0;
				x ++;
				break;
			}
		}
	}
}
void SH1106_char(unsigned char x,unsigned char y,char acsii,char size,char mode)
{
	unsigned char i,j,y0=y;
	char temp;
	unsigned char ch = acsii - ' ';
	for(i = 0;i<size;i++)
	{
		if(size == 12)
		{
			if(mode)temp=Font1206[ch][i];
			else temp = ~Font1206[ch][i];
		}
		else 
		{			
			if(mode)temp=Font1608[ch][i];
			else temp = ~Font1608[ch][i];
		}
		for(j =0;j<8;j++)
		{
			if(temp & 0x80) SH1106_pixel(x,y,1);
			else SH1106_pixel(x,y,0);
			temp <<=1;
			y++;
			if((y-y0)==size)
			{
				y = y0;
				x ++;
				break;
			}
		}
	}
}
void SH1106_string(unsigned char x,unsigned char y, const char *pString,
					unsigned char Size,unsigned char Mode)
{
    while (*pString != '\0') {       
        if (x > (WIDTH - Size / 2)) {
			x = 0;
			y += Size;
			if (y > (HEIGHT - Size)) {
				y = x = 0;
			}
		}
		
        SH1106_char(x, y, *pString, Size, Mode);
        x += Size / 2;
        pString ++;
    }
}

void SH1106_bitmap(unsigned char x,unsigned char y,const unsigned char *pBmp,
					unsigned char chWidth,unsigned char chHeight)
{
	unsigned char i, j, byteWidth = (chWidth + 7)/8;
	for(j = 0; j < chHeight; j++){
		for(i = 0;i <chWidth;i ++){
			if(*(pBmp + j*byteWidth+i/8) & (128 >> (i & 7))){
				SH1106_pixel(x + i, y + j, 1);
			}
		}
	}		
}

void SH1106_display()
{
    unsigned char page;
    unsigned char *pBuf = buffer;
    
    for (page = 0; page < 8; page++) {  
        /* set page address */
        command(0xB0 + page);
        /* set low column address */
        command(0x02); 
        /* set high column address */
        command(0x10); 
        /* write data */
        digitalWrite(DC, HIGH);
        wiringPiSPIDataRW(CHANNEL, pBuf, WIDTH); 
        pBuf += WIDTH;
    }
}
