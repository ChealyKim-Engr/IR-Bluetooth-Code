// Runs on TM4C123
// Use PWM0/PB6 and PWM1/PB7 to generate pulse-width modulated outputs.

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "PLL.h"
#include "PWM.h"
#include "SysTick.h"
#include "UART0.h"
#include "UART1.h"
#include "Timer0.h"
#include "tm4c123gh6pm.h"

unsigned char IR_intr=0, fiveSecond = 0;

void WaitForInterrupt(void);  // low power mode
void PortF_Init(void);
void PortB_Init(void);
void IR_Signal(void);
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

void UserTask(void){
	if (IR_intr == 0)
		IR_intr = 1;
	else if(IR_intr == 1) 
		IR_intr = 0;
	fiveSecond++;
}

//---------------------OutCRLF---------------------
// Output a CR,LF to UART to go to a new line
// Input: none
// Output: none
void OutCRLF(void){
  UART0_OutChar(CR);
  UART0_OutChar(LF);
}

////**************************************************Start Main
int main(void){
		
	unsigned long start_2ms, start_1ms ;
	unsigned long lg_1_high_1ms, lg_1_low_500us;
	unsigned long lg_0_high_500us, lg_0_low_500us;

	unsigned char RX_1, cmprChar, index, dirctnA, dirctnB;
	char *fifty_percnt = "50%";
	char *sevnty5_percnt = "75%";
	char *onehndrd_percnt = "100";
	char *IR_one = "IR1";
//	  char *IR_two = "IR2";
//	  char *IR_three = "IR3";
//	  char *IR_four = "IR4";
	  
	char string[3];  // global to assist in debugging
	unsigned long cmpString1, cmpString2, cmpString3, cmpString4;
	unsigned long speed, preSpeed;

	PLL_Init();                      // bus clock at 80 MHz
	PortF_Init();
	PortB_Init();
	SysTick_Init(); 
	UART0_Init();              
	UART1_Init();
		
	Timer0_Init(&UserTask, 80000000); // initialize timer1 (16 Hz)
	
	PWM0A_Init(40000, 25000);     // initialize PWM0, 1000 Hz, 50% duty
	PWM0B_Init(40000, 25000);     // initialize PWM0, 1000 Hz, 50% duty
	
	GPIO_PORTF_DATA_R = 0x02;
	GPIO_PORTB_DATA_R |= 0x01; 		//MotorA foward
	PWM0A_Duty(15000);   					// 50%		
	GPIO_PORTB_DATA_R |= 0x02;  	//MotorB foward
	PWM0B_Duty(15000);    				// 50%	
	
	speed = 15000;
	preSpeed = 15000;
	index = 0;
	dirctnA = 1;
	dirctnB = 1;
			
	IR_Signal();	
	GPIO_PORTF_DATA_R = 0x02;
	SysTick_Wait10ms(100);
	IR_Signal();
	GPIO_PORTF_DATA_R = 0x0C;
	SysTick_Wait10ms(100);
	IR_Signal();
	GPIO_PORTF_DATA_R =0;
	while (1) {
		RX_1 = UART1_InChar();
		cmprChar = RX_1;
		while (RX_1 != 0) {
			UART0_OutChar(RX_1);
			if (index < 3) {
				string[index] = RX_1;
					index++;
				}
				RX_1 = UART1_InChar();
			}	
      index = 0;
      cmpString1 = strncmp(fifty_percnt, string, 3);
      cmpString2 = strncmp(sevnty5_percnt, string, 3);
      cmpString3 = strncmp(onehndrd_percnt, string, 3);	
			cmpString4 = strncmp(IR_one, string, 3);
			switch(cmprChar) {
				case 'w': {		//foward
					GPIO_PORTF_DATA_R = 0x0A;
					GPIO_PORTB_DATA_R |= 0x01; 	//MotorA, foward	
					GPIO_PORTB_DATA_R |= 0x02;  	//MotorB, foward
					dirctnA = 1;
					dirctnB = 1;
					speed = preSpeed;
					if (speed == 1) {		//previous foward	
						GPIO_PORTF_DATA_R = 0x08;
						PWM0A_Duty(1);   	//100%		
						PWM0B_Duty(1);	       	//100%	
						preSpeed = 1;
					}
					else if (speed == 10000) {	//previous foward	
						GPIO_PORTF_DATA_R = 0x04;
						PWM0A_Duty(10000);   	//75%		
						PWM0B_Duty(10000);	//75%	
						preSpeed = 10000;
					}
					else if (speed == 15000){	//previous foward	
						GPIO_PORTF_DATA_R = 0x02;
						PWM0A_Duty(15000);   	//50%		
						PWM0B_Duty(15000);	//50%	
						preSpeed = 15000;
					}
					else if (speed == 39000) {    //previous backward
						GPIO_PORTF_DATA_R = 0x08;	
						PWM0A_Duty(1);   	 //100%		
						PWM0B_Duty(1);	         //100%	
						preSpeed = 1;
					}
					else if (speed == 30000) {	//previous backward
						GPIO_PORTF_DATA_R = 0x04;
						PWM0A_Duty(10000);   	//75%		
						PWM0B_Duty(10000);	//75%
						preSpeed = 10000;
					}
					else if (speed == 25000){	//previous backward
						GPIO_PORTF_DATA_R = 0x02;	
						PWM0A_Duty(15000);   	//50%		
						PWM0B_Duty(15000);	//50%
						preSpeed = 15000;
					}
				}
				break;
				case 's': {		//backward
					GPIO_PORTF_DATA_R = 0x0C;
					GPIO_PORTB_DATA_R &= ~0x01; 	//MotorA, backward	
					GPIO_PORTB_DATA_R &= ~0x02;  	//MotorB, backward
					dirctnA = 0;
					dirctnB = 0;
					speed = preSpeed;
					if (speed == 1) {		//previous foward	
						GPIO_PORTF_DATA_R = 0x08;	
						PWM0A_Duty(39000);   	 //100%		
						PWM0B_Duty(39000); 	  //100%	
						preSpeed = 39000;
					}
					else if (speed == 10000) {	//previous foward	
						GPIO_PORTF_DATA_R = 0x04;
						PWM0A_Duty(30000);   	//75%		
						PWM0B_Duty(30000);	//75%	
						preSpeed = 30000;
					}
					else if (speed == 15000){	//previous foward		
						GPIO_PORTF_DATA_R = 0x02;
						PWM0A_Duty(25000);   	//50%		
						PWM0B_Duty(25000);	//50%	
						preSpeed = 25000;
					}
					else if (speed == 39000) {	//previous backward
						GPIO_PORTF_DATA_R = 0x08;	
						PWM0A_Duty(39000);   	//100%		
						PWM0B_Duty(39000);	//100%	
						preSpeed = 39000;
					}
					else if (speed == 30000) {	//previous backward
						GPIO_PORTF_DATA_R = 0x04;
						PWM0A_Duty(30000);   	//75%		
						PWM0B_Duty(30000);    	//75%
						preSpeed = 30000;
					}
					else if (speed == 25000){	//previous backward
						GPIO_PORTF_DATA_R = 0x02;	
						PWM0A_Duty(25000);   	//50%		
						PWM0B_Duty(25000);     	//50%						
						preSpeed = 25000;
					}
				}
				break;
				case 'a': {		//left
					if ((dirctnA == 0) && (dirctnB == 0)) {
						GPIO_PORTB_DATA_R |= 0x01; 	//MotorA, foward	
						GPIO_PORTB_DATA_R |= 0x02;  	//MotorB, foward						
						PWM0A_Duty(20000);   		//50%	
						PWM0B_Duty(20000);	   	//50%
						dirctnA = 1;
						dirctnB = 1;
					}
					GPIO_PORTF_DATA_R = 0x06;
					GPIO_PORTB_DATA_R |= 0x01; 	  	//MotorA, foward	
					GPIO_PORTB_DATA_R &= ~0x02;  		//MotorB, backward
					PWM0A_Duty(1);   			//90%		
					PWM0B_Duty(25000);		       	//50%
					SysTick_Wait10ms(50);    	  	//turn for 1 second
					cmprChar = 'q';
				}
				break;
				case 'd': {		//right
					if ((dirctnA == 0) && (dirctnB == 0)) {
						GPIO_PORTB_DATA_R |= 0x01; 	 //MotorA, foward	
						GPIO_PORTB_DATA_R |= 0x02;  	//MotorB, foward						
						PWM0A_Duty(20000);   		//50%		
						PWM0B_Duty(20000);	    	//50%	
						dirctnA = 1;
						dirctnB = 1;
					}
					GPIO_PORTF_DATA_R = 0x0A;
					GPIO_PORTB_DATA_R &= ~0x01; 		//MotorA, backward	
					GPIO_PORTB_DATA_R |= 0x02;  		//MotorB, foward
					PWM0A_Duty(25000);   			//50%		
					PWM0B_Duty(1);		          	//90%
					SysTick_Wait10ms(50);    		//turn for 1 second
					cmprChar = 'q';
				}
				break;
				case 'q': {		//stop
					GPIO_PORTF_DATA_R = 0x0C;
					if ((dirctnA == 1) && (dirctnB == 1)) {
						PWM0A_Duty(35000);   			//10%		MotorA, foward
						PWM0B_Duty(35000);		        //10%		MotorB, foward
					}
					else if ((dirctnA == 0) && (dirctnB == 0)) {
						PWM0A_Duty(4000);   			//10%		MotorA, backward
						PWM0B_Duty(4000);		        //10%		MOtorB, backward				
					}
				}
				break;
				default: {
					cmprChar = 'w';
				}
				break;		
			}
			if ((dirctnA == 1) && (dirctnB == 1)) {
				GPIO_PORTB_DATA_R |= 0x01; 	//MotorA, foward	
				GPIO_PORTB_DATA_R |= 0x02;  	//MotorB, foward
				if (cmpString1 == 0) {
					preSpeed = 15000;
					GPIO_PORTF_DATA_R = 0x02;
					PWM0A_Duty(15000);   	//50%		
					PWM0B_Duty(15000);	//50%
					preSpeed = 15000;
				}			
				else if (cmpString2 == 0) {
					preSpeed = 10000;
					GPIO_PORTF_DATA_R = 0x04;
					PWM0A_Duty(10000);   		//75%		
					PWM0B_Duty(10000);	         //75%
					preSpeed = 10000;
				}
				else if (cmpString3 == 0) {
					preSpeed = 1;
					GPIO_PORTF_DATA_R = 0x08;	
					PWM0A_Duty(1);   	      //100%		
					PWM0B_Duty(1);	              //100%
					preSpeed = 1;
				}	
			}
			else if ((dirctnA == 0) && (dirctnB == 0)) {
				GPIO_PORTB_DATA_R &= ~0x01; 		//MotorA, backward	
				GPIO_PORTB_DATA_R &= ~0x02;  		//MotorB, backward
				if (cmpString1 == 0) {
					GPIO_PORTF_DATA_R = 0x02;
					PWM0A_Duty(25000);   		//50%		
					PWM0B_Duty(25000);		//50%
					preSpeed = 25000;
				}			
				else if (cmpString2 == 0) {
					preSpeed = 30000;
					GPIO_PORTF_DATA_R = 0x04;
					PWM0A_Duty(30000);   		//75%		
					PWM0B_Duty(30000);	        //75%
					preSpeed = 30000;
				}
				else if (cmpString3 == 0) {
					preSpeed = 39000;
					GPIO_PORTF_DATA_R = 0x08;	
					PWM0A_Duty(39000);   		//100%		
					PWM0B_Duty(39000);	        //100%	
					preSpeed = 39000;
				}	
			}
			if (cmprChar == 'q') {
				if ((dirctnA == 1) && (dirctnB == 1)) {
					PWM0A_Duty(35000);   		//10%		MotorA, foward
					PWM0B_Duty(35000);		//10%		MotorB, foward
				}
				else if ((dirctnA == 0) && (dirctnB == 0)) {
					PWM0A_Duty(4000);   		//10%		MotorA, backward
					PWM0B_Duty(4000);	     	//10%		MOtorB, backward				
				}	
			}
			if (cmpString4 == 0) {
					fiveSecond = 0;
					IR_Signal();	
			}
		}
}
////*************************************************END MAIN

////*************************************************Function Definitiobn
void PortF_Init(void){ 
  volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x20;         // 1) activate Port F
  delay = SYSCTL_RCGC2_R;         // allow time for clock to stabilize
  GPIO_PORTF_LOCK_R = 0x4C4F434B; // 2) unlock Port F lock
  GPIO_PORTF_CR_R = 0x1F;         //    enable commit (allow configuration changes) on PF4-0
  GPIO_PORTF_AMSEL_R = 0x00;      // 3) disable analog functionality on PF4-0
  GPIO_PORTF_PCTL_R = 0x00000000; // 4) configure PF4-0 as GPIO
  GPIO_PORTF_DIR_R = 0x0E;        // 5) PF4 and PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;      // 6) disable alt funct on PF4-0
  GPIO_PORTF_DEN_R = 0x1F;        // 7) enable digital I/O on PF4-0
  GPIO_PORTF_PUR_R |= 0x11;     	//     enable weak pull-up on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     	// (d) PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    	//     PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;    	//     PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R = 0x11;      	// (e) clear flags 4,0
  GPIO_PORTF_IM_R |= 0x11;      	// (f) arm interrupt on PF4,PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00400000; // (g) priority 2
  NVIC_EN0_R = 0x40000000;      	// (h) enable interrupt 30 in NVIC
}

//PB6, PB7 is PWM, PB0, PB1 is Direction (PB6-->PB0, PB7-->PB1)
void PortB_Init(void) {
  unsigned long volatile delay;
SYSCTL_RCGC2_R |= 0x02;           // 1) activate Port B
  delay = SYSCTL_RCGC2_R;           // allow time for clock to stabilize
  GPIO_PORTB_AMSEL_R &= ~0x03;      // 3) disable analog functionality on PB0, PB1
  GPIO_PORTB_PCTL_R &= ~0x000000FF; // 4) configure PB0, PB1 as GPIO
  GPIO_PORTB_DIR_R |= 0x03;         // 5) PB0, PB1 as output
  GPIO_PORTB_AFSEL_R &= ~0x03;      // 6) disable alt funct on PB0, PB1
  GPIO_PORTB_DEN_R |= 0x03;         // 7) enable digital I/O on PB0, PB1
	
  GPIO_PORTB_AMSEL_R = 0x00;        // 3) disable analog functionality on PB
  GPIO_PORTB_PCTL_R &= ~0x00000F00; // 4) configure PB2 as GPIO
  GPIO_PORTB_DIR_R |= 0x04;         // 5) PB2 as output
  GPIO_PORTB_AFSEL_R &= ~0x04;      // 6) disable alt funct on PB
  GPIO_PORTB_DEN_R |= 0x04;         // 7) enable digital I/O on PB
}

void IR_Signal(void) {
	unsigned long start_2ms, start_1ms ;
	unsigned long lg_1_high_1ms, lg_1_low_500us;
	unsigned long lg_0_high_500us, lg_0_low_500us;
	while(fiveSecond < 5) {	
		start_2ms = 0 , start_1ms = 0;		
		lg_1_high_1ms = 0, lg_1_low_500us = 0;
		lg_0_high_500us = 0, lg_0_low_500us = 0;
	
		//************************************start 2ms high, 1ms low
		while(start_2ms < 78) {			//start high for 2ms, 2000us/26
			GPIO_PORTB_DATA_R |= 0x04; 	// 13 us high
			SysTick_Wait13us(1);    	// 13 us
			GPIO_PORTB_DATA_R &= ~0x04; // 13 us low
			SysTick_Wait13us(1);    	// 13 us	
			start_2ms++;
		}			
		while(start_1ms < 39 ) {      //start low for 1ms, 1000/26
			GPIO_PORTB_DATA_R &= ~0x04; // 26 us low
			SysTick_Wait13us(2);        // 26 us	
			start_1ms++;
		}
		
		//**************************************Modulated 0b10
		//Logic '1': 1ms high, 500 us low
		while(lg_1_high_1ms < 39) {		//lg_1_high_1ms, 1000/26
			GPIO_PORTB_DATA_R |= 0x04; 	// 13 us high
			SysTick_Wait13us(1);    	// 13 us
			GPIO_PORTB_DATA_R &= ~0x04;     // 13 us low
			SysTick_Wait13us(1);    	// 13 us	
			lg_1_high_1ms++;
		}
		while(lg_1_low_500us < 20 ) { // lg_1_low_500us, 500/26
			GPIO_PORTB_DATA_R &= ~0x04; // 26 us low
			SysTick_Wait13us(2);        // 26 us	
			lg_1_low_500us++;
		}		
		//Logic '0': 500 us highk 500 us low
		while(lg_0_high_500us < 20) {	//lg_0_high_500us, 500/26
			GPIO_PORTB_DATA_R |= 0x04; 	// 13 us high
			SysTick_Wait13us(1);    	// 13 us
			GPIO_PORTB_DATA_R &= ~0x04; // 13 us low
			SysTick_Wait13us(1);        // 13 us	
			lg_0_high_500us++;
		}
		while(lg_0_low_500us < 20 ) { // lg_0_low_500us, 500/26
			GPIO_PORTB_DATA_R &= ~0x04; // 26 us low
			SysTick_Wait13us(2);        // 26 us	
			lg_0_low_500us++;
		}
		lg_1_high_1ms = 0, lg_1_low_500us = 0;
		lg_0_high_500us = 0, lg_0_low_500us = 0;
	
		//**************************************Modulated 0b10		
		//Logic '1': 1ms high, 500 us low
		while(lg_1_high_1ms < 39) {		//lg_1_high_1ms, 1000/26
			GPIO_PORTB_DATA_R |= 0x04; 	// 13 us high
			SysTick_Wait13us(1);    	// 13 us
			GPIO_PORTB_DATA_R &= ~0x04;     // 13 us low
			SysTick_Wait13us(1);    	// 13 us	
			lg_1_high_1ms++;
		}
		while(lg_1_low_500us < 20 ) { // lg_1_low_500us, 500/26
			GPIO_PORTB_DATA_R &= ~0x04; // 26 us low
			SysTick_Wait13us(2);        // 26 us	
			lg_1_low_500us++;
		}		
		//Logic '0': 500 us highk 500 us low
		while(lg_0_high_500us < 20) {	//lg_0_high_500us, 500/26
			GPIO_PORTB_DATA_R |= 0x04; 	// 13 us high
			SysTick_Wait13us(1);    	// 13 us
			GPIO_PORTB_DATA_R &= ~0x04;     // 13 us low
			SysTick_Wait13us(1);    	// 13 us	
			lg_0_high_500us++;
		}
		while(lg_0_low_500us < 20 ) { // lg_0_low_500us, 500/26
			GPIO_PORTB_DATA_R &= ~0x04; // 26 us low
			SysTick_Wait13us(2);        // 26 us	
			lg_0_low_500us++;
		}
		lg_1_high_1ms = 0, lg_1_low_500us = 0;
		lg_0_high_500us = 0, lg_0_low_500us = 0;		
	
		//**************************************Modulated 0b10
		//Logic '1': 1ms high, 500 us low
		while(lg_1_high_1ms < 39) {		//lg_1_high_1ms, 1000/26
			GPIO_PORTB_DATA_R |= 0x04; 	// 13 us high
			SysTick_Wait13us(1);    	// 13 us
			GPIO_PORTB_DATA_R &= ~0x04; // 13 us low
			SysTick_Wait13us(1);        // 13 us	
			lg_1_high_1ms++;
		}
		while(lg_1_low_500us < 20 ) { // lg_1_low_500us, 500/26
			GPIO_PORTB_DATA_R &= ~0x04; // 26 us low
			SysTick_Wait13us(2);        // 26 us	
			lg_1_low_500us++;
		}		
		//Logic '0': 500 us highk 500 us low
		while(lg_0_high_500us < 20) {	//lg_0_high_500us, 500/26
			GPIO_PORTB_DATA_R |= 0x04; 	// 13 us high
			SysTick_Wait13us(1);    	// 13 us
			GPIO_PORTB_DATA_R &= ~0x04; // 13 us low
			SysTick_Wait13us(1);        // 13 us	
			lg_0_high_500us++;
		}
		while(lg_0_low_500us < 20 ) { // lg_0_low_500us, 500/26
			GPIO_PORTB_DATA_R &= ~0x04; // 26 us low
			SysTick_Wait13us(2);        // 26 us	
			lg_0_low_500us++;
		}
		SysTick_Wait10ms(2);  //skip 20ms
		if (IR_intr == 0)
			GPIO_PORTF_DATA_R = 0x08;
		else {
			GPIO_PORTF_DATA_R = 0x04;
		}
	}
}
