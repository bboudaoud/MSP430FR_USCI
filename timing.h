// System Timing Header File
#ifndef TIMING_H_
#define TIMING_H_
#include "msp430fr5739.h"

// System defines used for comm libs (not these definitions are contingent on use of clkInit function below)
#define	DCO_FREQ	8000000
#define	MCLK_FREQ	8000000
#define SMCLK_FREQ	8000000

// Clock calib function arguments
#define 	DCO_1MHZ		0
#define 	DCO_4MHZ		1
#define		DCO_8MHZ		2
#define 	DCO_16MHZ		3
#define 	DCO_20MHZ		4
#define		DCO_24MHZ		5

inline void clkInit(unsigned char speed){
	/* 6 clock speeds are provided in the MSP these are:
	 * 	speed = 0 => 1 MHz
	 * 	speed = 1 => 4 MHz
	 * 	speed = 2 => 8 MHz
	 * 	speed = 3 => 16 MHz
	 * 	speed = 4 => 20 MHz
	 * 	speed = 5 => 24 MHz
	 * 	*/
	unsigned char locSpeed = speed;

	CSCTL0 = CSKEY;                      		// Unlock register

	if(speed > 2) {									// Does RSEL need to be set?
		locSpeed = speed - 3;
		CSCTL1 |= DCORSEL;
	}
	else CSCTL1 &= ~DCORSEL;

	switch(locSpeed) {
		case 0:
			CSCTL1 &= ~(DCOFSEL0 + DCOFSEL1);
			break;
		case 1:
			CSCTL1 |= DCOFSEL0;
			CSCTL1 &= ~DCOFSEL1;
			break;
		case 2:
			CSCTL1 |= (DCOFSEL1 + DCOFSEL0);
			break;
		default:
			break;

	}
	// Should we offer an alternative config here???
	CSCTL2 = SELA_1 + SELS_3 + SELM_3;        		// Set ACLK = vlo; SMCLK = MCLK = DCO
	CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        		// Set all dividers to 1 (ACLK, SMCLK, MCLK)
	CSCTL0_H = 0x01;                          		// Lock Register
}




#endif /* TIMING_H_ */
