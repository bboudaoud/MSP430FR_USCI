/******************************************************************************
 * This file contains the pin input/output lists for the onboard eUSCI modules
 * it is seperated from remaining HAL documents for use with MSPs in many
 * different hardware contexts
 ******************************************************************************/
#ifndef COMM_HAL
#define COMM_HAL

#include "msp430fr5739.h"
// MSP430FR5739 EUSCI Module Pinouts
//*********** UCA0 **************//
// UCA0TXD/SIMO	= P2.0 (Pin 21)
// UCA0RXS/SOMI = P2.1 (Pin 22)
// UCA0SCLK = P1.5 (Pin 10)
//*********** UCA1 *************//
// UCA1TXD/SIMO = P2.5 (Pin 17)
// UCA1RXD/SOMI = P2.6 (Pin 18)
// UCA1CLK = P2.4 (Pin 35)
//*********** UCB0 *************//
// UCB0SIMO/UCBSDA = P1.6 (Pin 28)
// UCB0SOMI/UCBSCL = P1.7 (Pin 29)
// UCB0CLK = P2.2 (Pin 23)
//******************************//

#ifdef USE_UCA0_UART	// UCA0 UART Mode Defines
	#define UCA0_IO_CONF(x)	P2SEL1 |= BIT0 + BIT1; P2SEL0 &= ~(BIT0 + BIT1)						///< USCI A0 UART I/O Configuration
	#define	UCA0_IO_CLEAR()	P2SEL1 &= ~(BIT0 + BIT1); P2SEL0 &= ~(BIT0 + BIT1)					///< USCI A0 UART I/O Clear
#endif
#ifdef USE_UCA0_SPI	// UCA0 SPI Mode Defines
	#define UCA0_IO_CONF(x)	P2SEL1 |= BIT0 + BIT1; P2SEL0 &= ~(BIT0 + BIT1); P1SEL1 |= BIT5; P1SEL0 &= ~(BIT5)	///< USCI A0 SPI I/O Configuration
	#define	UCA0_IO_CLEAR()	P2SEL1 &= ~(BIT0 + BIT1); P2SEL0 &= ~(BIT0 + BIT1); P1SEL1 &= ~BIT5; P1SEL0 &= ~BIT5	///< USCI A0 SPI I/O Clear
#endif
#ifdef USE_UCA1_UART	// UCA1 UART Mode Defines
	#define UCA1_IO_CONF(x)	P2SEL1 |= BIT5 + BIT6; P2SEL0 &= ~(BIT5 + BIT6)						///< USCI A1 UART I/O Configuration
	#define UCA1_IO_CLEAR()	P2SEL1 &= ~(BIT5 + BIT6); P2SEL0 &= ~(BIT5 + BIT6)					///< USCI A1 UART I/O Clear
#endif
#ifdef USE_UCA1_SPI	// UCA1 SPI Mode Defines
	#define UCB0_IO_CONF(x)	P1SEL1 |= BIT6 + BIT7; P1SEL0 &= ~(BIT6 + BIT7); P2SEL1 |= BIT2; P2SEL0 &= ~BIT2	///< USCI B0 SPI I/O Configuration
	#define UCB0_IO_CLEAR()	P1SEL1 &= ~(BIT6 + BIT7); P1SEL0 &= ~(BIT6 + BIT7); P2SEL1 &= ~BIT2; P2SEL0 &= ~BIT2 	///< USCI B0 SPI I/O Clear
#endif
#ifdef USE_UCB0_SPI	// UCB0 SPI Mode Defines
	#define UCB0_IO_CONF(x)	P1SEL1 |= BIT6 + BIT7; P1SEL0 &= ~(BIT6 + BIT7); P2SEL1 |= BIT2; P2SEL0 &= ~BIT2	///< USCI B0 SPI I/O Configuration
	#define UCB0_IO_CLEAR()	P1SEL1 &= ~(BIT6 + BIT7); P1SEL0 &= ~(BIT6 + BIT7); P2SEL1 &= ~BIT2; P2SEL0 &= ~BIT2 	///< USCI B0 SPI I/O Clear
#endif
#ifdef USE_UCB0_I2C	// UCB0 I2C Mode Defines
	#define UCB0_IO_CONF(x)	P1SEL1 |= BIT6 + BIT7; P1SEL0 &= ~(BIT6 + BIT7); P2SEL1 |= BIT2; P2SEL0 &= ~BIT2 //; I2C_ADDR(x)	///< USCI B0 I2C I/O Configuration
	#define UCB0_IO_CLEAR()	P1SEL1 &= ~(BIT6 + BIT7); P1SEL0 &= ~(BIT6 + BIT7); P2SEL1 &= ~BIT2; P2SEL0 &= ~BIT2			///< USCI B0 I2C I/O Clear
	#define I2C_ADDR(x)	UCB0I2CSA = x
#endif
#ifdef USE_UCB1
#error No USCI B1 Module Available in the MSP430FR5739
#endif //USE_UCB1
#endif // COMM_HAL