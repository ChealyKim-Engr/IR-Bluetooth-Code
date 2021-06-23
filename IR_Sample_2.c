
#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include "ST7735.h"
#include "SysTick.h"
#include "PLL.h"
#include "tm4c123gh6pm.h"

void PortF_Init(void);
void PortB_Init(void);
void mycolorLDC(void);
void waitScreen(void); 

int main(void){
	
	char samplingArray[1050];
	unsigned long index = 0, zero = 0, one = 1;;
	
	PortF_Init();
	PortB_Init();
  	PLL_Init();               // set system clock to 80 MHz
 	SysTick_Init();           // initialize SysTick timer
	
	mycolorLDC();
	while (1) {
		GPIO_PORTF_DATA_R = 0x04;
		if ((GPIO_PORTB_DATA_R&0x01)==0) {
			while (index < 1050) {
				if ((GPIO_PORTB_DATA_R&0x01)==0) {
					samplingArray[index] = 0;
					index++;
				}
				else if((GPIO_PORTB_DATA_R&0x01)==1) {
					samplingArray[index] = 1;
					index++;
				}
				////Sample every 10us 100KHz > 38KHz
				SysTick_Wait10us(1);
			}
		}
		GPIO_PORTF_DATA_R = 0x08;
		SysTick_Wait10ms(300);
		GPIO_PORTF_DATA_R = 0x00;
		for (index=0; index<1050; index++) {
			if (samplingArray[index]==0){
				GPIO_PORTF_DATA_R ^= 0x02;
				SysTick_Wait10ms(3);
				zero++;
			}
			else if(samplingArray[index]==1){
				GPIO_PORTF_DATA_R ^= 0x04;
				SysTick_Wait10ms(3);
				one++;		
			}
		}
		if ((zero==650)&&(one==400)) {
			GPIO_PORTF_DATA_R = 0x0E;
			mycolorLDC();					
		}
		else {
			GPIO_PORTF_DATA_R = 0x0A;
			waitScreen();
		}
	}
	
	while (1) {
		if ((zero==650)&&(one==400))
			GPIO_PORTF_DATA_R = 0x0E;
		else 
			GPIO_PORTF_DATA_R = 0x0A;
	}
}

////************************************End Main
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

void PortB_Init(void) {
  unsigned long volatile delay;
  SYSCTL_RCGC2_R |= 0x02;           // 1) activate Port B
  delay = SYSCTL_RCGC2_R;           // allow time for clock to stabilize
  GPIO_PORTB_AMSEL_R = 0x00;        // 3) disable analog functionality on PB
  GPIO_PORTB_PCTL_R &= ~0x000000FF; // 4) configure PB0,1 as GPIO
  GPIO_PORTB_DIR_R &= ~0x01;        // 5) PB0 as input
  GPIO_PORTB_DIR_R |=  0x02;        // 5) PB1 as output
  GPIO_PORTB_AFSEL_R &= ~0x03;      // 6) disable alt funct on PB0, 1
  GPIO_PORTB_DEN_R |= 0x03;         // 7) enable digital I/O on PB0, 1
}

void waitScreen(void) {
	  
   ST7735_InitR(INITR_REDTAB);
   ST7735_FillScreen(ST7735_YELLOW);
}

void mycolorLDC(void) {
	char irString[50] = "I R  C o m m a n d : ";
	char passStr[50] = "P A S S E D ";
	char irStr[50] = "T H E  I R !";
	char nose[10] = " J ";
  //PLL_Init(12);
  ST7735_InitR(INITR_REDTAB);
	
	ST7735_FillScreen(ST7735_CYAN);
	ST7735_FillRect(0, 0,128, 30, ST7735_YELLOW);
	  
	ST7735_FillCircle(100, 130, 26,ST7735_MAGENTA);
	ST7735_DrawString(0, 1, irString, ST7735_BLACK);
	ST7735_DrawString(0, 12, passStr, ST7735_BLACK);
	ST7735_DrawString(0, 14, irStr, ST7735_BLACK);
	ST7735_DrawLine(80, 120, 90, 120, ST7735_BLACK);		//eye
	ST7735_DrawLine(110, 120, 120, 120, ST7735_BLACK);	//eye
	ST7735_FillCircle(85, 126, 5,ST7735_BLACK);
	ST7735_FillCircle(115, 126, 5,ST7735_BLACK);
	ST7735_DrawString(14, 13, nose, ST7735_BLACK);
	
	ST7735_DrawLine(5, 35, 33, 35, ST7735_BLACK);				//7 * 4 +5
	ST7735_DrawLine(33, 35, 33, 110, ST7735_BLACK);		  //33
	ST7735_DrawLine(33, 110, 47, 110, ST7735_BLACK);		//33+14= 47
	
	ST7735_DrawLine(47, 35, 47, 110, ST7735_BLACK);			//47
	ST7735_DrawLine(47, 35, 61, 35, ST7735_BLACK);			//47+14=61
	ST7735_DrawLine(61, 35, 61, 110, ST7735_BLACK);			//61
	ST7735_DrawLine(61, 110, 68, 110, ST7735_BLACK);		//61+7=68
	ST7735_DrawLine(68, 35, 68, 110, ST7735_BLACK);			//68
	ST7735_DrawLine(68, 35, 75, 35, ST7735_BLACK);			//68+7=75
	ST7735_DrawLine(75, 35, 75, 110, ST7735_BLACK);			//75
	ST7735_DrawLine(75, 110, 82, 110, ST7735_BLACK);		//75+7=82
	
	ST7735_DrawLine(82, 35, 82, 110, ST7735_BLACK);			
	ST7735_DrawLine(82, 35, 96, 35, ST7735_BLACK);			
	ST7735_DrawLine(96, 35, 96, 110, ST7735_BLACK);		
	ST7735_DrawLine(96, 110, 103, 110, ST7735_BLACK);		
	ST7735_DrawLine(103, 35, 103, 110, ST7735_BLACK);		
	ST7735_DrawLine(103, 35, 120, 35, ST7735_BLACK);		
	ST7735_DrawLine(120, 35, 120, 110, ST7735_BLACK);		
	ST7735_DrawLine(120, 110, 127, 110, ST7735_BLACK);		
}
