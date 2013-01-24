// A Simple HAL File Created for the MSP430FR5739 Experimenter Board
#ifndef HAL_H_
#define HAL_H_
#include "useful.h"

// I/O Controls
#define		SW_1_OPEN	B8_0(&P4IN)
#define		SW_2_OPEN	B8_1(&P4IN)
#define		SW_CONF()	P4OUT |= 0x03;\
						P4DIR &= ~(0x03);\
						P4REN |= 0x03

#define MCLK_ON()		PJDIR |= 0x03;\
						PJSEL1 &= ~(0x03);\
						PJSEL0 |= 0x03

#define MCLK_OFF()		PJSEL0 &= ~(0x03);\
						PJOUT &= ~(0x03)

// LED Control
#define		LED1	B8_0(&PJOUT)
#define		LED2	B8_1(&PJOUT)
#define		LED3	B8_2(&PJOUT)
#define		LED4	B8_3(&PJOUT)
#define		LED5	B8_4(&P3OUT)
#define		LED6	B8_5(&P3OUT)
#define		LED7	B8_6(&P3OUT)
#define		LED8	B8_7(&P3OUT)

#define		LED_SET(x)	if((x) & BIT0) LED1 = 1;\
						else LED1 = 0;\
						if((x) & BIT1) LED2 = 1;\
						else LED2 = 0;\
						if((x) & BIT2) LED3 = 1;\
						else LED3 = 0;\
						if((x) & BIT3) LED4 = 1;\
						else LED4 = 0;\
						if((x) & BIT4) LED5 = 1;\
						else LED5 = 0;\
						if((x) & BIT5) LED6 = 1;\
						else LED6 = 0;\
						if((x) & BIT6) LED7 = 1;\
						else LED7 = 0;\
						if((x) & BIT7) LED8 = 1;\
						else LED8 = 0

#define		LED_CONF()	PJDIR |= 0x0F;\
						P3DIR |= 0xF0

#endif /* HAL_H_ */
