/*
 * Neesha Bhardwaj
 861300379
 Custom project
 */ 

//tutorials and some code from:
//https://www.newbiehack.com/MicrocontrollerControlAHobbyServo.aspx
//http://diotlabs.daraghbyrne.me/3-working-with-sensors/fsrs/
//http://www.gadgetronicx.com/pir-motion-sensor-interface-with-avr/
//adc init in main func()
//then need to read adc
//input = Read_ADC(0x00); for A0 in port a
//can only read adc once per tick
#include <avr/io.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include "timer.h"

//BEGIN ADC
void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

unsigned short Read_ADC(uint8_t ch) {
	ADMUX = (1<<REFS0); //AVCC as ADV voltage reference
	ADCSRA = (1<<ADEN) | (1 << ADPS2); //enable ADC, prescale 16
	ch = ch&0b00000111;
	ADMUX |= ch;
	
	ADCSRA |= (1<<ADSC);
	while (!(ADCSRA & (1<<ADIF)));
	ADCSRA |= (1<<ADIF);
	return ADC;
}
//END OF ADC

//the 3 state machines 
enum checkweights {weightinit, enough, notenough} checkweight ;
enum checkmotions {motioninit, motion, nomotion} checkmotion; 
enum servomotors {servoinit, servomove} servomotor;

unsigned short input;
unsigned short motioninput; //variable used to determine if motion has been sensed or not
unsigned char condition = 0x00;
unsigned char tempB = 0x00; //temp value to store port b value for the LED
unsigned char motiontempB = 0x00; //temp value for the motion

	
//condition is set to 0x01 when there is not enough food	
void TickFct_checkweight(){
	switch(checkweight) {
		case weightinit:
		checkweight = enough;
		break;
		
		case enough:
		if (input < 512) { //using 512 since value from 0 -1023 so 512 is halfway
			checkweight = notenough;
			condition = 0x01; //turn on the condition var to indicate not enough
			tempB = 0x00;
		}
		else if (input >= 512) {
			checkweight = enough;
			condition = 0x00;
			tempB  = 0x01; //test for the LED
		}
		break;
		
		case notenough:
		if (input < 512) {
			checkweight = notenough;
			condition = 0x01;
			tempB = 0x00;
		}
		else if (input >= 512) {
			checkweight = enough;
			condition = 0x00;
			tempB = 0x01;
		}
		break;
		
		default:
		break;
	} //end of the check weight state machine trnsition

	switch (checkweight) {
		case weightinit:
			break;
		case enough:
			break;
		case notenough:
			break;
		default:
			break;
		
	} //end of the actions
} //end of the tck function for the check weight


void TickFct_checkmotions() {
		switch (checkmotion) { //begin of motion sensor switch statement
			case motioninit: //automatically go to the no motion state
                checkmotion = nomotion;
                break;
			
			case nomotion: //when no motion is sensed
				if(!motioninput && ((condition & 0x01) == 0x01))            // check for sensor pin PC.0 using bit
				//pinc0 is defined as 0
				{
				condition = 0x01; //condition is 1 when there is no motion
				motiontempB = 0x00; //turning the led2 off since no motion but keep value
				checkmotion = nomotion;
                }
                else if (motioninput && ((condition & 0x01) != 0x01))
                {
                    condition = 0x03; //condition is 0x03 when motion is sensed
                    motiontempB = 0x02; //turning on 2nd led to show motion
                    checkmotion = motion;
                }
			
                break;
			
			case motion:

                if (motioninput && ((condition & 0x01) != 0x01))
                // check for sensor pin PC.0 using bit
                //pinc0 is defined as 0
                {
                    condition = 0x03;
                    motiontempB = 0x02;
                    checkmotion = motion;        // LED on
                            
                }
                
                else if (!motioninput && ((condition & 0x01) == 0x01))
                {
                    motiontempB = 0x00; //led is off
                    condition = 0x01;
                    checkmotion = nomotion;
                }
			
                break;
			
			default:
                checkmotion = motioninit;
                break;
                
		} //end of the motion sensor switch statement
		switch(checkmotion) {
			case motioninit:
				break;
			case motion:
				break;
			case nomotion:
				break;
			default:
				break;
			
		} //end of the actions for the motion state machine
} //end of the tick function for the mnotion fucntion

void TickFct_servomotors()
{
	
	//start of the servo motor swtich statement
	switch(servomotor) {
		case servoinit:
            servomotor = servomove;
            break;
		
		case servomove:
		//only go into the if statement if the condition has been set to 0x03
            if ((condition & 0x03) == 0x03)
            {
                tempB = 0x00;
                OCR1A = ICR1 - 1600; //PD5
                _delay_ms(50);
                OCR1A = ICR1 - 2500; //PD5
                _delay_ms(50);
                servomotor = servomove;
            }
            break;
		
		default:
			break;
		
	} //END OF THE SERVO
	switch(servomotor) {
		case servoinit:
			break;
		case servomove:
			break;
		default:
			break;
	} //end of the actions for the servo motor
}

int main(void)
{
	
	//the set up for the servo
	DDRD |= 0xFF;
	TCCR1A |= 1<<WGM11 | 1<<COM1A1 | 1<<COM1A0;
	TCCR1B |= 1<<WGM13 | 1<<WGM12 | 1<<CS10;
	ICR1 = 19999; //the 16 bit timer
	// the end of the servo set up
	
	//setting the ports
	DDRA = 0x00; PORTA = 0x00; //PA[0] connected to potentiometer giving input
	DDRC = 0x00; PORTC = 0xFF; //input for the motion sensor
	DDRB = 0xFF; PORTB = 0x00; //output for the LEDS
	
	ADC_init(); //initializing the ADC
	//unsigned short x = ADC;  // Value of ADC register now stored in variable x.
	
	//setting the intial state of each tick function
	checkweight = weightinit;
	checkmotion = motioninit;
	servomotor = servomove;
	
    //the counters used for timing
	unsigned char servocounter = 0;
	unsigned char motioncounter = 0;
	unsigned char weightcounter = 0;
	TimerSet(10); //period set to 10
	TimerOn();
	
	while(1)
	{
		input = Read_ADC(0);
		
        //ADC always port A so this is pin0 of port a
		motioninput = (PINC&(1<<0));
	
		if (weightcounter == 20) { //making it 200ms
			TickFct_checkweight();
			weightcounter = 0;
		}
	
		if (motioncounter == 5) { //the period for this is 50ms
			TickFct_checkmotions();
			motioncounter = 0;
		}
		
        if (servocounter == 10) { //making it 100ms
			TickFct_servomotors();
			servocounter = 0;
		}
		
				
		PORTB = tempB;//motiontempB;
		while(!TimerFlag);
		TimerFlag = 0;
		servocounter++; //incrementing the counter 
		motioncounter++;
		weightcounter++;
	
	} //end of the while(1) function
} //end of the main

