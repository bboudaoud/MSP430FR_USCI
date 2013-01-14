#ifndef COMM_H_
#define COMM_H_
#include "msp430fr5739.h"	// Includes USCI Control Register names for USCI config
#include "timing.h"			// Includes system clock macros for baud rate generation (used in UBR_DIV macro)
#include "useful.h"			// Includes bitwise access structure and macros (used in CS logic)


#define MAX_DEVS	8			///< Maximum number of devices to be registered

// USCI Library Conditional Compilation Macros
// NOTE: Only define at most 1 config for each USCI module, otherwise a Multiple Serial Endpoint error will be created on compilation
//#define USE_UCA0_UART			///< USCI A0 UART Mode Conditional Compilation Flag
//#define USE_UCA0_SPI			///< USCI A0 SPI Mode Conditional Compilation Flag
//#define USE_UCA1_UART			///< USCI A1 UART Mode Conditional Compilation Flag
//#define USE_UCA1_SPI			///< USCI A1 SPI Mode Conditional Compilation Flag
#define USE_UCB0_SPI			///< USCI B0 SPI Mode Conditional Compilation Flag
//#define USE_UCB0_I2C			///< USCI B0 I2C Mode Conditional Compilation Flag

/// USCI Configuration Data Structure
typedef struct uconf
{
	unsigned int rAddr;			///< 16-Bit Resource Code [ USCI # (2 bits) ] [  USCI mode (2 bits) ] [ CS or I2C Address (12 bits) ]
	unsigned int usciCtlW0;		///< 16-Bit USCI Control Word0 (see TI User Guide)
	unsigned int usciCtlW1;		///< 16-Bit USCI Control Word1 (see TI User Guide)
	unsigned int baudDiv;		///< Sourced clock rate divisor (can use FREQ_2_BAUDDIV(x) macro included below)
	unsigned char *rxPtr;		///< Data write back pointer
} usciConfig;

/*********************************************************
 * Resource address control codes
 ********************************************************/
// Resource Address Masking
#define	USCI_MASK	0xC000		///< USCI device name (UCXX) mask for resource address code
#define MODE_MASK	0x3000		///< USCI mode (UART/SPI/I2C) name mask for resource address code
#define	UMODE_MASK	0xF000		///< USCI name/mode mask for resource address code
#define	ADDR_MASK	0x0FFF		///< USCI address mask for resource address code
// Resource Codes
#define UCA0_RCODE		0x4000	///< USCI A0 resource code
#define	UCA1_RCODE		0x8000	///< USCI A1 resource code
#define UCB0_RCODE		0xC000	///< USCI B0 resource code
#define UART_MODE		0x1000	///< UART mode code
#define SPI_MODE		0x2000	///< SPI mode code
#define I2C_MODE		0x3000	///< I2C mode code
// Resource and Mode combo codes
#define UCA0_UART		UCA0_RCODE + UART_MODE
#define UCA0_SPI		UCA0_RCODE + SPI_MODE
#define UCA1_UART		UCA1_RCODE + UART_MODE
#define UCA1_SPI		UCA1_RCODE + SPI_MODE
#define UCB0_SPI		UCB0_RCODE + SPI_MODE
#define UCB0_I2C		UCB0_RCODE + I2C_MODE

/**********************************************************
 * USCI Register Values
 **********************************************************/
// USCI CTL Word 0 Defaults
// UART MODE
#define UART_8N1	UCSSEL1										///< UCTLW0: 8 bit UART (no parity, 1 stop bit) w/ baud from SMCLK
#define	UART_7N1	UCSSEL1 + UC7BIT							///< UCTLW0: 7 bit UART (no parity, 1 stop bit) w/ baud from SMCLK
// SPI MODE
#define SPI_8M0		UCSYNC + UCSSEL1 + UCMST					///< UCTLW0: 8 bit Mode 0 SPI Master w/ baud from SMCLK
#define SPI_8M1		UCSYNC + UCSSEL1 + UCCKPH + UCMST			///< UCTLW0: 8 bit Mode 1 SPI Master w/ baud from SMCLK
#define SPI_8M2		UCSYNC + UCSSEL1 + UCCKPL + UCMST			///< UCTLW0: 8 bit Mode 2 SPI Master w/ baud from SMCLK
#define SPI_8M3		UCSYNC + UCSSEL1 + UCCKPH + UCCKPL + UCMST	///< UCTLW0: 8 Bit Mode 3 SPI Master w/ baud from SMCLK
#define	SPI_S8M0	UCSYNC										///< UCTLW0: 8 Bit Mode 0 SPI Slave
#define SPI_S8M1	UCSYNC + UCCKPH								///< UCTLW0: 8 Bit Mode 1 SPI Slave
#define SPI_S8M2	UCSYNC + UCCKPL								///< UCTLW0: 8 Bit Mode 2 SPI Slave
#define SPI_S8M3	UCSYNC + UCCKPH + UCCKPL					///< UCTLW0: 8 Bit Mode 3 SPI Slave
// I2C MODE
#define I2C_10SMT	UCA10 + UCSLA10 + UCMST + UCMODE_3 + UCSSEL1 + UCTR + UCSYNC	///< UCTLW0: 10 bit addressed I2C (master and slave), single master mode, transmitter w/ baud from SMCLK
#define I2C_10SMR	UCA10 + UCSLA10 + UCMST + UCMODE_3 + UCSSEL1 + UCSYNC			///< UCTLW0: 10 bit addressed I2C (master and slave), single master mode, receiver w/ baud from SMCLK
#define I2C_7SMT	UCMST + UCMODE_3 + UCSYNC + UCSSEL1 + UCTR						///< UCTLW0: 7 bit addressed I2C (master and slave), single master mode, transmitter w/ baud from SMCLK
#define I2C_7SMR	UCMST + UCMODE_3 + UCSYNC + UCSSEL1								///< UCTLW0: 7 bit addressed I2C (master and slave), single master mode, receiver w/ baud from SMCLK
// USCI CTL Work 1 Defaults
#define DEF_CTLW1	0x0003	///< CTLW1: 200ns deglitch time
// USCI Baud Rate Defaults
#define UCLK_FREQ		SMCLK_FREQ		///< USCI Clock Rate [use SMCLK to source our UART (from timing.h)]
#define UBR_DIV(x)		UCLK_FREQ/x		///< Baud rate frequency to divisor macro (uses timing.h)
// USCI Interrupt Vector values
#define UCIVTXIFG		0x04			///< USCI Transmit Interrupt Flag Mask
#define UCIVRXIFG		0x02			///< USCI Receive Interrupt Flag Mask

// Resource config buffer index
#define UCA0_INDEX		0				///< USCI A0 shared buffer index
#define UCA1_INDEX 		1				///< USCI A1 shared buffer index
#define UCB0_INDEX		2				///< USCI B0 shared buffer index

// USCI Status Codes
#define 	OPEN		0				///< USCI OPEN Status code
#define		TX			1				///< USCI TX Status code
#define		RX			2				///< USCI RX Status code

// Read/Write Routine Return Codes
#define 	USCI_CONF_ERROR		-2		///< USCI configuration error return code
#define		USCI_BUSY_ERROR		-1		///< USCI busy error return code
#define		USCI_SUCCESS		1		///< TX/RX success return code

// App. registration function prototype
int registerComm(usciConfig conf);


/*************************************************************************
 * UCA0 Macro Logic
 ************************************************************************/
// Config function prototype
inline void confUCA0(unsigned int commID);
/************************* UCA0 UART MODE ********************************/
#ifdef USE_UCA0_UART
// Function prototypes
int uartA0Write(unsigned char* data, unsigned int len, unsigned int commID);
int uartA0Read(unsigned int len, unsigned int commID);
// Other useful macros
#define USE_UCA0																///< UCA0 Active Definition
#define UCA0_IO_CONF(x)	P2SEL1 |= BIT0 + BIT1; P2SEL0 &= ~(BIT0 + BIT1)			///< USCI A0 UART I/O Configuration
#define	UCA0_IO_CLEAR()	P2SEL1 &= ~(BIT0 + BIT1); P2SEL0 &= ~(BIT0 + BIT1)		///< USCI A0 UART I/O Clear
// Multiple endpoint config detection
#ifdef USE_UCA0_SPI
#error Multiple Serial Endpoint Configuration on USCI A0
#endif // USE_UCA0_UART and USE_UCA0_SPI
#endif
/************************* UCA0 SPI MODE ********************************/
#ifdef USE_UCA0_SPI
// Function prototypes
int spiA0Write(unsigned char* data, unsigned int len, unsigned int commID);
int spiA0Read(unsigned int len, unsigned int commID);
// Other useful Macros
#define USE_UCA0																///< USCI A0 Active Definition
#ifdef USE_UCA0_UART
#error Multiple Serial Endpoint Configuration on USCI A0
#endif // USE_UCA0_UART AND USE_UCA0_SPI
#define UCA0_IO_CONF(x)	P2SEL1 |= BIT0 + BIT1; P2SEL0 &= ~(BIT0 + BIT1); P1SEL1 |= BIT5; P1SEL0 &= ~(BIT5)		///< USCI A0 SPI I/O Configuration
#define	UCA0_IO_CLEAR()	P2SEL1 &= ~(BIT0 + BIT1); P2SEL0 &= ~(BIT0 + BIT1); P1SEL1 &= ~BIT5; P1SEL0 &= ~BIT5	///< USCI A0 SPI I/O Clear
#endif // USE_UCA0_SPI


/**************************************************************************
 * UCA1 Macro Logic
 *************************************************************************/
/************************* UCA1 UART MODE ********************************/
#ifdef USE_UCA1_UART
// Function prototypes
int uartA1Write(unsigned char* data, unsigned int len, unsigned int commID);
int uartA1Read(unsigned int len, unsigned int commID);
// Other useful macros
#define USE_UCA1															///< USCI A1 Active Definition
#define UCA1_IO_CONF(x)	P2SEL1 |= BIT5 + BIT6; P2SEL0 &= ~(BIT5 + BIT6)		///< USCI A1 UART I/O Configuration
#define UCA1_IO_CLEAR()	P2SEL1 &= ~(BIT5 + BIT6); P2SEL0 &= ~(BIT5 + BIT6)	///< USCI A1 UART I/O Clear
// Multiple endpoint config detection
#ifdef USE_UCA1_SPI
#error Multiple Serial Endpoint Configuration on USCI A1
#endif // USE_UCA1_UART and USE_UCA1_SPI
#endif // USE_UCA1_UART
/*************************** UCA1 SPI MODE *******************************/
#ifdef USE_UCA1_SPI
// Function prototypes
int spiA1Write(unsigned char* data, unsigned int len, unsigned int commID);
int spiA1Read(unsigned int len, unsigned int commID);
// Other useful macros
#define USE_UCA1																			///< USCI A1 Active Definition
#define UCA1_IO_CONF(x)	P2SEL1 |= BIT4 + BIT5 + BIT6; P2SEL0 &= ~(BIT4 + BIT5 + BIT6); 		///< USCI A1 SPI I/O Configuration
#define	UCA1_IO_CLEAR()	P2SEL1 &= ~(BIT4 + BIT5 + BIT6); P2SEL0 &= ~(BIT4 + BIT5 + BIT6);	///< USCI A1 SPI I/O Clear
// Multiple endpoint config detection
#ifdef USE_UCA1_UART
#error Multiple Serial Endpoint Configuration on USCI A1
#endif // USE_UCA1_UART and USE_UCA1_SPI
#endif // USE_UCA1_SPI


/**************************************************************************
 * UCB0 Macro Logic
 *************************************************************************/
/************************* UCB0 SPI MODE *********************************/
#ifdef USE_UCB0_SPI
// Function prototypes
int spiB0Write(unsigned char* data, unsigned int len, unsigned int commID);
int spiB0Read(unsigned int len, unsigned int commID);
// Other useful macros
#define USE_UCB0		///< USCI B0 Active Definition
#define UCB0_IO_CONF(x)	P1SEL1 |= BIT6 + BIT7; P1SEL0 &= ~(BIT6 + BIT7); P2SEL1 |= BIT2; P2SEL0 &= ~BIT2		///< USCI B0 SPI I/O Configuration
#define UCB0_IO_CLEAR()	P1SEL1 &= ~(BIT6 + BIT7); P1SEL0 &= ~(BIT6 + BIT7); P2SEL1 &= ~BIT2; P2SEL0 &= ~BIT2 	///< USCI B0 SPI I/O Clear
// Multiple endpoint config detection
#ifdef USE_UCB0_I2C
#error Mulitple Serial Endpoint Configuration on USCI B0
#endif // USE_UCB0_SPI and USE_UCB0_I2C
#endif // USE_UCB0_SPI
/************************* UCB0 I2C MODE *********************************/
#ifdef USE_UCB0_I2C
// Function prototypes
int i2cB0Write(unsigned char* data, unsigned int len, unsigned int commID);
int i2cB0Read(unsigned int len, unsigned int commID);
// Other useful macros
#define USE_UCB0																	///< USCI B0 Active Definition
#define UCB0_IO_CONF(x)	P1SEL1 |= BIT6 + BIT7; P1SEL0 &= ~(BIT6 + BIT7); P2SEL1 |= BIT2; P2SEL0 &= ~BIT2 //; I2C_ADDR(x)	///< USCI B0 I2C I/O Configuration
#define UCB0_IO_CLEAR()	P1SEL1 &= ~(BIT6 + BIT7); P1SEL0 &= ~(BIT6 + BIT7); P2SEL1 &= ~BIT2; P2SEL0 &= ~BIT2
#define I2C_ADDR(x)	UCB0I2CSA = x
/// Muleiple endpoint config detection
#ifdef USE_UCB0_SPI
#error Multiple Serial Endpoint Configuration on USCI B0
#endif // USE_UCB0_SPI and USE_UCB0_I2C
#endif // USE_UCB0_I2C
#endif /* COMM_H_ */
