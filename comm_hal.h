/******************************************************************************
 * This file contains the pin input/output lists for the onboard eUSCI modules
 * it is seperated from remaining HAL documents for use with MSPs in many
 * different hardware contexts
 ******************************************************************************/
#ifndef COMM_HAL
#define COMM_HAL

#include "useful.h"

// If we are using the MSP430FR5739 (FRAM series)
#ifdef MSP430FR5739_COMM_LIB
#ifdef USE_UCB1
#error No USCI B1 Module Available in the MSP430FR5739
#endif //USE_UCB1
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
#endif // MSP430FR5739_COMM_LIB


#ifdef MSP430F5510_COMM_LIB
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
#endif // MSP430F5510_COMM_LIB

