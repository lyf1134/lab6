/*	Author: Yifei Liu
 *  Partner(s) Name: 
 *	Lab Section: 023
 *	Assignment: Lab 6  Exercise 3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link:https://drive.google.com/file/d/1g34KG-dM6UA6uiMy6dEyJ3LSztRL5frX/view?usp=sharing
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include <avr/interrupt.h>

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B 	= 0x0B;	// bit3 = 1: CTC mode (clear timer on compare)
					// bit2bit1bit0=011: prescaler /64
					// 00001011: 0x0B
					// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
					// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A 	= 125;	// Timer interrupt will be generated when TCNT1==OCR1A
					// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
					// So when TCNT1 register equals 125,
					// 1 ms has passed. Thus, we compare to 125.
					// AVR timer interrupt mask register

	TIMSK1 	= 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1 = 0;

	// TimerISR will be called every _avr_timer_cntcurr milliseconds
	_avr_timer_cntcurr = _avr_timer_M;

	//Enable global interrupts
	SREG |= 0x80;	// 0x80: 1000000
}

void TimerOff() {
	TCCR1B 	= 0x00; // bit3bit2bit1bit0=0000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect)
{
	// CPU automatically calls when TCNT0 == OCR0 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; 			// Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { 	// results in a more efficient compare
		TimerISR(); 				// Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
enum LA_States { LA_SMStart, LA_s0, LA_s1,LA_s2,LA_s3} LA_State;
unsigned char A0,A1,tmpb,i;
void TickFct_Latch()
{
  switch(LA_State) {   // Transitions
     case LA_SMStart:  // Initial transition
        LA_State = LA_s0;
	tmpb=0x07;
        break;

     case LA_s0:
	if(A0&&A1){
	   tmpb=0x00;
	   LA_State = LA_s3;
	   break;
	}
        if (A0) {
           LA_State = LA_s1;
	   tmpb=tmpb+0x01;
	   i=0;
        }
        else if (A1) {
           LA_State = LA_s2;
	   if(tmpb>0x00){
		tmpb=tmpb-0x01;
	   }
	   i=0;
        }
	else{
	   LA_State = LA_s0;
	}
        break;

     case LA_s1:
        if (A0) {
           LA_State = LA_s1;
	   i++;
	   if(A1){
	      tmpb=0x00;
    	      LA_State = LA_s3;
	   }
	   else if(i%10==0){
	      tmpb+=0x01;
	   }
        }
        else {
           LA_State = LA_s0;
        }
        break;

     case LA_s2:
        if (A1) {
           LA_State = LA_s2;
	   i++;
	   if(A0){
	      tmpb=0x00;
	      LA_State = LA_s3;
	   }
	   else if(i%10==0){
	      if(tmpb>0x00)
		 tmpb-=0x01;
	   }
        }
        else {
           LA_State = LA_s0;
        }
        break;
     case LA_s3:
	if(!A1&&!A0){
	   LA_State = LA_s0;
	}
	else{
	   LA_State = LA_s3;
	}
	break;

     default:
        LA_State = LA_SMStart;
        break;
  } // Transitions

  switch(LA_State) {   // State actions

     default:
	if(tmpb>0x09)
	   tmpb=0x09;
        break;
   } // State actions
}
int main(void) {
    /* Insert DDR and PORT initializations */
    DDRB=0xFF;PORTB=0x00;
    DDRA=0x00;PORTA=0xFF;
    /* Insert your solution below */
    TimerSet(100);
    TimerOn();
    LA_State=LA_SMStart;
    tmpb=0;
    i=0;
    while (1) {
	A0=(~PINA)&0x01;
	A1=((~PINA)&0x02)>>1;
	TickFct_Latch();
        while(!TimerFlag);
	TimerFlag=0;
	PORTB=tmpb;
    }
    return 1;
}
