/*============================================================================================================
FILENAME        :countersystem.c
DESCRIPTION     :A program for an automated counter system through an integration of an ultrasonic sensor to a PIC16F877A and an displays the output to the LCD  
AUTHOR          :Zach Riane I. Machacon, Eldridge Aaron S. Miole, Dave Nelson F. Ogue, John Jason C. Zamora
CREATED ON      :5 May 2024
============================================================================================================*/
#include<xc.h> // include file for the XC8 compiler

#pragma config FOSC = XT
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

#define _XTAL_FREQ 4000000  // Define the system operating frequency as 4MHz
bit myINTF = 0;
bit myTMR0IF = 0;

void interrupt ISR() {
  GIE = 0;

  if (INTF) {
    INTF = 0;
    myINTF = 1;
  } else if (TMR0IF) {
    TMR0IF = 0;
    myTMR0IF = 1;
  } else if (TMR1IF) {
    TMR1IF = 0;
  }

  GIE = 1;
}

void delay(int count) {
  int of_count = 0;

  while (count > of_count) {
    if (myTMR0IF) {
      of_count++;
      myTMR0IF = 0;
    }
  }
}


void instCtrl(unsigned char INST) {
  PORTD = INST;
  RB5 = 0;
  RB7 = 1;
  delay(1);
  RB7 = 0;
}

void dataCtrl(unsigned char DATA) {
  PORTD = DATA;
  RB5 = 1;
  RB7 = 1;
  delay(1);
  RB7 = 0;
}

void initLCD() {
  delay(1);
  instCtrl(0x38);
  instCtrl(0x08);
  instCtrl(0x01);
  instCtrl(0x06);
  instCtrl(0x0C);
  
  
}

void distanceDisplay(unsigned int distance)
{
	unsigned int onesDigit = (distance%10);
	unsigned int tensDigit = ((distance/10)%10);
	unsigned int hundredsDigit = ((distance/100)%10);

	instCtrl(0xC2);
  	dataCtrl('D');
  	dataCtrl('i');
  	dataCtrl('s');
  	dataCtrl('t');
  	dataCtrl('a');
  	dataCtrl('n');
  	dataCtrl('c');
  	dataCtrl('e');
  	dataCtrl('(');
  	dataCtrl('c');
  	dataCtrl('m');
  	dataCtrl(')');
  	dataCtrl(':');
  	dataCtrl(' ');

	if (hundredsDigit != 0)
	{	
		dataCtrl(hundredsDigit+0x30);
		dataCtrl(tensDigit+0x30);
		dataCtrl(onesDigit+0x30);
	}
	else if (tensDigit != 0)
	{
		dataCtrl(' ');
		dataCtrl(tensDigit+0x30);
		dataCtrl(onesDigit+0x30);
	} else {
		dataCtrl(' ');
		dataCtrl(' ');
		dataCtrl(onesDigit+0x30);
	}
}

void distanceTrig(unsigned int distance)
{
	unsigned int onesDigit = (distance%10);
	unsigned int tensDigit = ((distance/10)%10);
	unsigned int hundredsDigit = ((distance/100)%10);

    instCtrl(0x96);
  	dataCtrl('T');
  	dataCtrl('r');
  	dataCtrl('i');
  	dataCtrl('g');
  	dataCtrl(' ');
  	dataCtrl('D');
  	dataCtrl('i');
  	dataCtrl('s');
  	dataCtrl('t');
  	dataCtrl(':');
  	dataCtrl(' ');

	if (hundredsDigit != 0)
	{	
		dataCtrl(hundredsDigit+0x30);
		dataCtrl(tensDigit+0x30);
		dataCtrl(onesDigit+0x30);
	}
	else if (tensDigit != 0)
	{
		dataCtrl(' ');
		dataCtrl(tensDigit+0x30);
		dataCtrl(onesDigit+0x30);
	} else {
		dataCtrl(' ');
		dataCtrl(' ');
		dataCtrl(onesDigit+0x30);
	}
}

void counterDisplay(int counter) {
	int tenThousandPlace = (counter/10000)%10;
	int thousandPlace = (counter/1000)%10;
	int hundredPlace = (counter/100)%10;
	int tenPlace = (counter/10)%10;
	int onePlace = counter % 10;

	instCtrl(0x8B);	

	if (tenThousandPlace)
	{
		dataCtrl(tenThousandPlace+0x30);
		dataCtrl(thousandPlace+0x30);
		dataCtrl(',');
		dataCtrl(hundredPlace+0x30);
		dataCtrl(tenPlace+0x30);
		dataCtrl(onePlace+0x30);
	}
	else if (thousandPlace && !tenThousandPlace)
	{
		dataCtrl(thousandPlace+0x30);
		dataCtrl(',');
		dataCtrl(hundredPlace+0x30);
		dataCtrl(tenPlace+0x30);
		dataCtrl(onePlace+0x30);
		dataCtrl(' ');
	}
	else if (hundredPlace && !tenThousandPlace && !thousandPlace)
	{
		dataCtrl(hundredPlace+0x30);
		dataCtrl(tenPlace+0x30);
		dataCtrl(onePlace+0x30);
		dataCtrl(' ');
		dataCtrl(' ');
	}
	else if (tenPlace && !hundredPlace && !tenThousandPlace && !thousandPlace)
	{
		dataCtrl(tenPlace+0x30);
		dataCtrl(onePlace+0x30);
		dataCtrl(' ');
		dataCtrl(' ');
		dataCtrl(' ');
	}
	else if (onePlace && !tenPlace && !hundredPlace && !tenThousandPlace && !thousandPlace)
	{
		dataCtrl(onePlace+0x30);
		dataCtrl(' ');
		dataCtrl(' ');
		dataCtrl(' ');
		dataCtrl(' ');
	}
	else
	{
		dataCtrl('0');
		dataCtrl(' ');
		dataCtrl(' ');
		dataCtrl(' ');
		dataCtrl(' ');
	}
}

void sendPulse()
{		
		RC0 = 0;
		__delay_us(2);
		RC0 = 1;         // Send an ultrasonic burst 
		__delay_us(10);
		RC0 = 0;
		
}

unsigned int readSensor()
{
    TMR1 = 0x0000;
	sendPulse();
	TMR1ON = 1;
	while(!RC1 && TMR1 <23000); // stuck here
	TMR1 = 0x0000;
	while(RC1);
	TMR1ON = 0;
	float a = TMR1;   		//Reads Timer Value
    a = a/9.7;               //Converts Time to Distance 
	a = a + 1;

	return (int)a;	// get distance in cm
}

void main(){
	
	unsigned int counter = 1;
	unsigned int distance = 0;
	unsigned int triggerDistance = 0;

	ADCON1= 0x07;
	
	TRISB = 0x01;
	TRISD = 0x00;
	TRISA = 0xFF;
	TRISC = 0x02;
	
	RB6 = 0;
 	if (eeprom_read(0) == 0)
	{  
    	counter = 0;
 	} 
	else 
	{
    	counter = (int)eeprom_read(0);
  	}
	GIE = 0;
	OPTION_REG = 0x04;
  	TMR0IF = 0;
  	TMR0IE = 1;
  	INTEDG = 1; // TIMER 0 SET-UP
  	INTE = 1;
  	INTF = 0;

	T1CON = 0x30; // 1:8 prescaler, internal clock, Timer1 off
	TMR1IE = 1; // enable Timer1 overflow interrupt (PIE1 reg)
	TMR1IF = 0;

  	PEIE = 1;
	GIE = 1;
	
	initLCD();
	instCtrl(0x84);
  	dataCtrl('C');
  	dataCtrl('o');
  	dataCtrl('u');
  	dataCtrl('n');
  	dataCtrl('t');
  	dataCtrl(':');
  	dataCtrl(' ');
	int lessThanTen = 0;
	
	triggerDistance = readSensor();
	delay(20);
	triggerDistance = readSensor();

	triggerDistance = triggerDistance - 1;

	distanceTrig(triggerDistance);
	while(1)
	{	
		distance = readSensor();
		
		if (distance < triggerDistance && lessThanTen == 0)
		{
			lessThanTen = 1;
			counter++;
			RB6 = 1; // trigger buzzer
			counterDisplay(counter);
			distanceDisplay(distance);
			delay(30);
			RB6 = 0;
			eeprom_write(0,counter);
			//delay(122); // 1 second delay to avoid multiple reads 
		}
		else if(distance >= triggerDistance){
			lessThanTen = 0;
		}
		distanceDisplay(distance);
		if(myINTF)
		{
			if (RA0)
			{
				counter++;
			}
			else if(RA1)
			{
				if(counter == 0)
				{
					counter = 0;
				}
				else counter--;
			}
			else if(RA2)
			{
				counter = 0;
			}
			eeprom_write(0,counter);
			myINTF = 0;
			while(RB0);
		counterDisplay(counter);
		}
		
	}
}