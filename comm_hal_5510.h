/******************************************************************************
 * This file contains the pin input/output lists for the on-board eUSCI modules
 * it is separated from remaining HAL documents for use with MSPs in many
 * different hardware contexts
 ******************************************************************************/
#ifndef COMM_HAL
#define COMM_HAL

#include "msp430f5510.h"
// MSP430F5510 EUSCI Module Pinouts
//*********** UCA0 **************//
// UCA0TXD/SIMO = P3.3 (Pin 37)
// UCA0RXD/SOMI = P3.4 (Pin 38)
// UCA0SCLK = P2.7 (Pin 33)
//*********** UCA1 **************//
// UCA1TXD/SIMO = P4.4 (Pin 45)
// UCA1RXD/SOMI = P4.5 (Pin 46)
// UCA1SCLK = P4.0 (Pin 41)
//*********** UCB0 **************//
// UCB0SIMO/SDA = P3.0 (Pin 34)
// UCB0SOMI/SCL = P3.1 (Pin 35)
// UCB0SCLK = P3.2 (Pin 36)
//*********** UCB1 *************//
// UCB1SIMO/SDA = P4.1 (Pin 42)
// UCB1SOMI/SCL = P4.2 (Pin 43)
// UCB1SCLK = P4.3 (Pin 44)
//******************************//

#ifdef USE_UCA0_UART // UCA0 UART Mode Defines
	#define	UCA0_IO_CONF(x)	P3SEL |= (BIT3 + BIT4)					///< USCI A0 UART I/O Configuration
	#define UCA0_IO_CLEAR()	P3SEL &= ~(BIT3 + BIT4)					///< USCI A0 UART I/O Clear
#endif
#ifdef USE_UCA0_SPI // UCA0 SPI Mode Defines
	#define UCA0_IO_CONF(x) P3SEL |= (BIT3 + BIT4); P2SEL |= BIT7			///< USCI A0 SPI I/O Configuration
	#define UCA0_IO_CLEAR()	P3SEL &= ~(BIT3 + BIT4); P2SEL &= ~BIT7			///< USCI A0 SPI I/O Clear
#endif
#ifdef USE_UCA1_UART	// UCA1 UART Mode Defines
	#define	UCA1_IO_CONF(x)	P4SEL |= (BIT4 + BIT5)					///< USCI A1 UART I/O Configuration
	#define UCA1_IO_CLEAR()	P4SEL &= ~(BIT4 + BIT5)					///< USCI A1 UART I/O Clear
#endif
#ifdef USE_UCA1_SPI	// UCA1 SPI Mode Defines
	#define	UCA1_IO_CONF(x) P4SEL |= (BIT0 + BIT4 + BIT5)				///< USCI A1 SPI I/O Configuration
	#define UCA1_IO_CLEAR()	P4SEL &= ~(BIT0 + BIT4 + BIT5)				///< USCI A1 SPI I/O Clear
#endif
#ifdef USE_UCB0_SPI // UCB0 SPI Mode Defines
	#define UCB0_IO_CONF(x)	P3SEL |= (BIT0 + BIT1 + BIT2)				///< USCI B0 SPI I/O Configuration
	#define	UCB0_IO_CLEAR()	P3SEL &= ~(BIT0 + BIT1 + BIT2)				///< USCI B0 SPI I/O Clear
#endif
#ifdef USE_UCB0_I2C // UCB0 I2C Mode Defines
	#define	UCB0_IO_CONF(x) P3SEL |= (BIT0 + BIT1)//; I2C_ADDR(x)			///< USCI B0 I2C I/O Configuration
	#define	UCB0_IO_CLEAR()	P3SEL &= ~(BIT0 + BIT1 + BIT2)				///< USCI B0 I2C I/O Clear
#endif
#ifdef USE_UCB1_SPI // UCB1 SPI Mode Defines
	#define	UCB1_IO_CONF(x)	P4SEL |= (BIT1 + BIT2 + BIT3)				///< USCI B1 SPI I/O Configuration
	#define UCB1_IO_CLEAR()	P4SEL &= ~(BIT1 + BIT2 + BIT3)				///< USCI B1 SPI I/O Clear
#endif
#ifdef USE_UCB1_I2C // UCB1 I2C Mode Defines
	#define UCB1_IO_CONF(x) P4SEL |= (BIT1 + BIT2)//; I2C_ADDR(x)			///< USCI B1 I2C I/O Configuration
	#define UCB1_IO_CLEAR()	P4SEL &= ~(BIT1 + BIT2)					///< USCI B1 I2C I/O Clear
#endif
#endif // COMM_HAL

