#include "comm.h"

usciConfig dev[MAX_DEVS];										///< Device config buffer (indexed by comm ID always non-zero)
unsigned int devIndex = 0;										///< Device config buffer index
unsigned int devConf[4] = {0,0,0,0};							///< Currently applied configs buffer [A0, A1, B0, B1]
unsigned char usciStat[4] = {OPEN, OPEN, OPEN, OPEN};			///< Store status (OPEN, TX, or RX) for [A0, A1, B0, B1]

/**************************************************************************//**
 * \brief Registers an application for use of a USCI module.
 *
 * Create a USCI "socket" by affiliating a unique comm ID with an
 * endpoint configuration for TI's eUSCI module.
 *
 * \Param	conf	The USCI configuration structure to be used (see comm.h)
 * \Return	commID 	A positive (> 0) value representing the registered
 * 					app
 * \Retval 	-1		The maximum number of apps (MAX_DEVS) has been registered
 ******************************************************************************/
int registerComm(usciConfig conf)
{
	// Register a device for system operation
	dev[++devIndex] = conf;
	if(devIndex > MAX_DEVS) return -1;
	return devIndex;
}

/****************************************************************
 * USCI A0 Variable Declarations
 ******************************************************************************/
#ifdef USE_UCA0
unsigned char *uca0TxPtr;				///< USCI A0 TX Data Pointer
unsigned char *uca0RxPtr;				///< USCI A0 RX Data Pointer
unsigned int uca0TxSize = 0;			///< USCI A0 TX Size
unsigned int uca0RxSize = 0;			///< USCI A0 RX Size
// Conditional SPI Receive size
#ifdef USE_UCA0_SPI
unsigned int spiA0RxSize = 0;			///< USCI A0 To-RX Size (used for SPI RX)
#endif //USE_UCA0_SPI

/**************************************************************
 * General Purpose USCI A0 Functions
 ******************************************************************************/
// NOTE: This configuration is safe for use with single end-point UART and SPI config
/**************************************************************************//**
 * \brief	Configures USCI A0 for operation
 *
 * Write control registers and clear system variables for the
 * USCI A0 module, which can be used as either a UART or SPI.
 *
 * \param	commID	The communication ID for the registered app
 ******************************************************************************/
inline void confUCA0(unsigned int commID)
{
	if(devConf[UCA0_INDEX] == commID) return;	// Check if device is already configured

	UCA0CTL1 |= UCSWRST;							// Pause operation
	UCA0_IO_CONF(dev[commID].rAddr & ADDR_MASK);	// Port set up

	// Configure key control words
	UCA0CTLW0 = dev[commID].usciCtlW0;
	UCA0CTLW1 = dev[commID].usciCtlW1;
	UCA0BRW = dev[commID].baudDiv;
	uca0RxPtr = dev[commID].rxPtr;

	// Clear buffer sizes
	uca0RxSize = 0;
	uca0TxSize = 0;
#ifdef USE_UCA0_SPI
	spiA0RxSize = 0;
#endif //USE_UCA0_SPI

	UCA0IE |= UCRXIE + UCTXIE;					// Enable Interrupts
	UCA0CTL1 &= ~UCSWRST;						// Resume operation

	devConf[UCA0_INDEX] = commID;				// Store config
}
/**************************************************************************//**
 * \brief	Resets USCI A0 without writing over control regs
 *
 *	This function is included to soft-reset the USCI A0 module
 *	management variables without clearing the current config.
 *
 *	\param	commID	The comm ID of the registered app
 *	\sideeffect		Sets the RX pointer to that registered w/ commID
 ******************************************************************************/
void resetUCA0(unsigned int commID){
	uca0RxPtr = dev[commID].rxPtr;
	uca0RxSize = 0;
	uca0TxSize = 0;
#ifdef USE_UCA0_SPI
	spiA0RxSize = 0;
#endif //USE_UCA0_SPI
	usciStat[UCA0_INDEX] = OPEN;
	return;
}
/**************************************************************************//**
 * \brief	Get method for USCI A0 RX buffer size
 *
 *	Returns the number of bytes which have been written
 *	to the RX pointer (since the last read performed).
 *
 *	\param commID	Comm ID of app. (not used)
 *	\return			The number of valid bytes following the rxPtr.
 ******************************************************************************/
unsigned int getUCA0RxSize(commID){
	return uca0RxSize;
}
/**************************************************************************//**
 * \brief	Get method for USCI A0 status
 *
 *	Returns the status of the USCI module, either OPEN, TX, or RX
 *
 *	\param commID	Comm ID of app. (not used)
 *	\retval		0	Indicates the OPEN status
 *	\retval 	1	Indicates the TX Status
 *	\retval 	2	Indicates the RX Status
 ******************************************************************************/
unsigned char getUCA0Stat(void){
	return usciStat[UCA0_INDEX];
}

/*************************************************************************//**
 * \brief	Set method for USCI A0 Baud Rate Divisor
 *
 * Sets the baud rate divisor of the USCI module, this divisor is generally
 * performed relative to the SMCLK rate of the system.
 *
 * \param 	baudDiv	The new divisor to apply
 * \param	commID	The communications ID number of the application
 *****************************************************************************/
void setUCA0Baud(unsigned int baudDiv, unsigned int commID){
	dev[commID].baudDiv = baudDiv;		// Replace the baud divisor in memory
	devConf[UCA0_INDEX] = 0;			// Reset the device config storage (config will be performed on next read/write)
	return;
}
/***************************************************************
* UCA0 UART HANDLERS
 ******************************************************************************/
#ifdef USE_UCA0_UART
/**************************************************************************//**
 * \brief	Transmit method for USCI A0 UART operation
 *
 * This method initializes the transmission of len bytes from
 * the base of the *data pointer. The actual transmission itself
 * is finished a variable length of time from the write (based
 * upon len's value) in the TX ISR. Thus calling uartA0Write() twice in quick
 * succession will likely result in partial transmission of the first data.
 *
 * \param	*data	Pointer to data to be written
 * \param	len		Length (in bytes) of data to be written
 * \param	commID	Communication ID number of application
 *
 * \retval	-2		Incorrect resource code
 * \retval	-1		USCI A0 module busy
 * \retval	1		Transmit successfully started
 ******************************************************************************/
int uartA0Write(unsigned char *data, unsigned int len, unsigned int commID)
{
	if(dev[commID].rAddr != UCA0_UART) return -2;	// Check that the commID is affiliated with UCA0 in UART mode (no address)
	if(usciStat[UCA0_INDEX] != OPEN) return -1;		// Check that the USCI is available

	confUCA0(commID);

	// Copy over pointer and length
	uca0TxPtr = data;
	uca0TxSize = len-1;
	// Write TXBUF (start of transmit) and set status
	UCA0TXBUF = *uca0TxPtr;
	usciStat[UCA0_INDEX] = TX;

	return 1;
}
/**************************************************************************//**
 * \brief	Receive method for USCI A0 UART operation
 *
 * This method spoofs an asynchronous read by providing the min of the
 * bytes available and the requested length. It decrements the buffer size
 * appropriately and returns bytes "read".
 *
 * \param	len		The number of bytes to be read from the buffer
 * \param	commID	The comm ID of the application
 * \return	The number of bytes available to read in the buffer. If the buffer
 * 			is empty this value will be 0.
 * \sideeffect	The uca0RxSize variable is decremented by the min of itself and
 * 				the requested amount of bytes (len).
 ******************************************************************************/
int uartA0Read(unsigned int len, unsigned int commID)
{
	confUCA0(commID);
	if(len > uca0RxSize) {
		len = uca0RxSize;
	}
	uca0RxSize -= len;
	return len;
}
#endif // USE_UCA0_UART
/***********************************************************
 * UCA0 SPI HANDLERS
 ***********************************************************/
#ifdef USE_UCA0_SPI
/**************************************************************************//**
 * \brief	Transmit method for USCI A0 SPI operation
 *
 * This method initializes a transmission of len bytes from the base of the
 * *data pointer. Similarly to uartA0Write(), the transmission uses the USCI A0
 * TX ISR to complete, so 2 sequential calls may result in partial transmission.
 *
 * \param	*data	Pointer to data to be written
 * \param	len		Length (in bytes) of data to be written
 * \param 	commID	Communication ID number of application
 *
 * \retval	-1		USCI A0 Module busy
 * \retval	1		Transmit successfully started
 *******************************************************************************/
int spiA0Write(unsigned char *data, unsigned int len, unsigned int commID)
{
	if(usciStat[UCA0_INDEX] != OPEN) return -1;		// Busy
	confUCA0(commID);
	// Copy over pointer and length
	uca0TxPtr = data;
	uca0TxSize = len-1;
	// Start of TX
	UCA0TXBUF = *uca0TxPtr;
	usciStat[UCA0_INDEX] = TX;

	return 1;
}
/**************************************************************************//**
 * \brief	Receive method for USCI A0 SPI operation
 *
 * This method performs a synchronous read by storing the bytes to be read in
 * spiA0RxSize and then performing len dummy write to the bus to fetch the data
 * from a slave device. The RX size is cleared on this function call.
 *
 * \param	len		The number of bytes to be read from the bus
 * \param	commID	Communication ID number of the application
 *
 * 	\retval	-1		USCI A0 Module Busy
 * 	\retval	1		Receive successfully started
 ******************************************************************************/
int spiA0Read(unsigned int len, unsigned int commID)
{
	if(usciStat[UCA0_INDEX] != OPEN) return -1;		// Busy
	confUCA0(commID);
	// Clear RX Size and copy length
	uca0RxSize = 0;
	spiA0RxSize = len-1;
	// Start of RX
	UCA0TXBUF = 0xFF;								// Start TX
	usciStat[UCA0_INDEX] = RX;
	return 1;
}
#endif //USE_UCA0_SPI
/**********************************************************************//**
 * \brief	USCI A0 RX/TX Interrupt Service Routine
 *
 * This ISR manages all TX/RX proceedures with the exception of transfer
 * initialization. Once a transfer (read or write) is underway, this method
 * assures the correct amount of bytes are written to the correct location.
 *************************************************************************/
#pragma vector=USCI_A0_VECTOR
__interrupt void usciA0Isr(void)
{
	unsigned int dummy = 0xFF;
	// Transmit Interrupt Flag Set
	if(UCA0IFG & UCTXIFG){
		if(uca0TxSize > 0){
			UCA0TXBUF = *(uca0TxPtr++);				// Transmit the next outgoing byte
			uca0TxSize--;
		}
		else{
			UCA0IV &= ~UCIVTXIFG;					// Clear TX interrupt flag from vector on end of TX
			usciStat[UCA0_INDEX] = OPEN; 			// Set status open if done with transmit
		}
	}

	// Receive Interrupt Flag Set
	if(UCA0IFG & UCRXIFG){
#ifdef USE_UCA0_SPI
		if(usciStat[UCA0_INDEX] == RX)				// Check we are in RX mode for SPI
#endif // USE_UCA0_SPI
		if(UCA0STATW & UCRXERR) dummy = UCA0RXBUF;	// RX ERROR: Do a dummy read to clear interrupt flag
		else {										// Otherwise write the value to the RX pointer
			*(uca0RxPtr++) = UCA0RXBUF;
			uca0RxSize++;							// RX Size decrement in read function
#ifdef USE_UCA0_SPI
			if(usciStat[UCA0_INDEX] == RX && uca0RxSize < spiA0RxSize) UCA0TXBUF = dummy; // Perform another dummy write
			else
#endif
			usciStat[UCA0_INDEX] = OPEN;

		}
	}
}
#endif // USE_UCA0


/****************************************************************
 * USCI A1 Variable Declarations
 ***************************************************************/
#ifdef USE_UCA1
unsigned char *uca1TxPtr;			///< USCI A1 TX Data Pointer
unsigned char *uca1RxPtr;			///< USCI A1 RX Data Pointer
unsigned int uca1TxSize = 0;		///< USCI A1 TX Size
unsigned int uca1RxSize = 0;		///< USCI A1 RX Size
// Conditional SPI Receive size
#ifdef USE_UCA1_SPI
unsigned int spiA1RxSize = 0;					///< USCI A1 To-RX Size (used for SPI RX)
#endif //USE_UCA1_SPI

/**************************************************************
 * General Purpose USCI A1 Functions
 *************************************************************/
/**************************************************************************//**
 * \brief	Configures USCI A1 for operation
 *
 * Write control registers and clear system variables for the
 * USCI A1 module, which can be used as either a UART or SPI.
 *
 * \param	commID	The communication ID for the registered app
 ******************************************************************************/
inline void confUCA1(unsigned int commID)
{
	if(devConf[UCA1_INDEX] == commID) return;	// Check if device is already configured

	UCA1CTL1 |= UCSWRST;							// Pause operation
	UCA1_IO_CONF(dev[commID].rAddr & ADDR_MASK);	// Port set up

	// Configure key control words
	UCA1CTLW0 = dev[commID].usciCtlW0;
	UCA1CTLW1 = dev[commID].usciCtlW1;
	UCA1BRW = dev[commID].baudDiv;
	uca1RxPtr = dev[commID].rxPtr;

	// Clear buffer sizes
	uca1RxSize = 0;
	uca1TxSize = 0;
#ifdef USE_UCA1_SPI
	spiA1RxSize = 0;
#endif //USE_UCA1_SPI

	UCA1IE |= UCRXIE + UCTXIE;					// Enable Interrupts
	UCA1CTL1 &= ~UCSWRST;						// Resume operation

	devConf[UCA1_INDEX] = commID;				// Store config
}

/**************************************************************************//**
 * \brief	Resets USCI A1 without writing over control regs
 *
 *	This function is included to sof-reset the USCI A1 module
 *	management variables without clearing the current config.
 *
 *	\param		commID	The comm ID of the registered app
 *	\sideeffect	Sets the RX pointer to that registered w/ commID
 ******************************************************************************/
void resetUCA1(unsigned int commID){
	uca1RxPtr = dev[commID].rxPtr;
	uca1RxSize = 0;
	uca1TxSize = 0;
#ifdef USE_UCA1_SPI
	spiA1RxSize = 0;
#endif //USE_UCA1_SPI
	usciStat[UCA1_INDEX] = OPEN;
	return;
}
/**************************************************************************//**
 * \brief	Get method for USCI A1 RX buffer size
 *
 *	Returns the number of bytes which have been written
 *	to the RX pointer (since the last read performed).
 *
 *	\param commID	Comm ID of app. (not used)
 *	\return	The number of valid bytes following the rxPtr.
 ******************************************************************************/
unsigned int getUCA1RxSize(commID){
	return uca1RxSize;
}
/**************************************************************************//**
 * \brief	Get method for USCI A1 status
 *
 *	Returns the status of the USCI module, either OPEN, TX, or RX
 *
 *	\param commID	Comm ID of app. (not used)
 *	\retval		0	Indicates the OPEN status
 *	\retval 	1	Indicates the TX Status
 *	\retval 	2	Indicates the RX Status
 ******************************************************************************/
unsigned char getUCA1Stat(void){
	return usciStat[UCA1_INDEX];
}
/***************************************************************
* UCA1 UART HANDLERS
***************************************************************/
#ifdef USE_UCA1_UART
/**************************************************************************//**
 * \brief	Transmit method for USCI A1 UART operation
 *
 * This method initializes the transmission of len bytes from
 * the base of the *data pointer. The actual transmission itself
 * is finished a variable length of time from the write (based
 * upon len's value) in the TX ISR. Thus calling uartA1Write() twice in quick
 * succession will likely result in partial transmission of the first data.
 *
 * \param	*data	Pointer to data to be written
 * \param	len		Length (in bytes) of data to be written
 * \param	commID	Communication ID number of application
 *
 * \retval	-2		Incorrect resource code
 * \retval	-1		USCI A1 module busy
 * \retval	1		Transmit successfully started
 ******************************************************************************/
int uartA1Write(unsigned char *data, unsigned int len, unsigned int commID)
{
	if(dev[commID].rAddr != UCA1_UART) return -2;	// Check that the commID is affiliated with UCA1 in UART mode (no address)
	if(usciStat[UCA1_INDEX] != OPEN) return -1;		// Check that the USCI is available

	confUCA1(commID);

	// Copy over pointer and length
	uca1TxPtr = data;
	uca1TxSize = len-1;
	// Write TXBUF (start of transmit) and set status
	UCA1TXBUF = *uca1TxPtr;
	usciStat[UCA1_INDEX] = TX;

	return 1;
}
/**************************************************************************//**
 * \brief	Receive method for USCI A1 UART operation
 *
 * This method spoofs an asynchronous read by providing the min of the
 * bytes available and the requested length. It decrements the buffer size
 * appropriately and returns bytes "read".
 *
 * \param	len		The number of bytes to be read from the buffer
 * \param	commID	The comm ID of the application
 * \return	The number of bytes available to read in the buffer. If the buffer
 * 			is empty this value will be 0.
 * \sideeffect	The ucA1RxSize variable is decremented by the min of itself and
 * 				the requested amount of bytes (len).
 ******************************************************************************/
int uartA1Read(unsigned int len, unsigned int commID)
{
	confUCA1(commID);
	if(len > uca1RxSize) {
		len = uca1RxSize;
	}
	uca1RxSize -= len;
	return len;
}
#endif // USE_UCA1_UART
/***********************************************************
 * UCA1 SPI HANDLERS
 ***********************************************************/
#ifdef USE_UCA1_SPI
/**************************************************************************//**
 * \brief	Transmit method for USCI A1 SPI operation
 *
 * This method initializes a transmission of len bytes from the base of the
 * *data pointer. Similarly to uartA1Write(), the transmission uses the USCI A1
 * TX ISR to complete, so 2 sequential calls may result in partial transmission.
 *
 * \param	*data	Pointer to data to be written
 * \param	len		Length (in bytes) of data to be written
 * \param 	commID	Communication ID number of application
 *
 * \retval	-1		USCI A1 Module busy
 * \retval	1		Transmit successfully started
 *******************************************************************************/
int spiA1Write(unsigned char *data, unsigned int len, unsigned int commID)
{
	if(usciStat[UCA1_INDEX] != OPEN) return -1;		// Busy
	confUCA1(commID);
	// Copy over pointer and length
	uca1TxPtr = data;
	uca1TxSize = len-1;
	// Start of TX
	UCA1TXBUF = *uca1TxPtr;
	usciStat[UCA1_INDEX] = TX;

	return 1;
}
/**************************************************************************//**
 * \brief	Receive method for USCI A1 SPI operation
 *
 * This method performs a synchronous read by storing the bytes to be read in
 * spiA1RxSize and then performing len dummy write to the bus to fetch the data
 * from a slave device. The RX size is cleared on this function call.
 *
 * \param	len		The number of bytes to be read from the bus
 * \param	commID	Communication ID number of the application
 *
 * 	\retval	-1		USCI A1 Module Busy
 * 	\retval	1		Receive successfully started
 ******************************************************************************/
int spiA1Read(unsigned int len, unsigned int commID)
{
	if(usciStat[UCA1_INDEX] != OPEN) return -1;		// Busy
	confUCA1(commID);
	// Clear RX Size and copy length
	uca1RxSize = 0;
	spiA1RxSize = len-1;
	// Start of RX
	UCA1TXBUF = 0xFF;								// Start TX
	usciStat[UCA1_INDEX] = RX;
	return 1;
}
#endif //USE_UCA1_SPI

/**********************************************************************//**
 * \brief	USCI A1 RX/TX Interrupt Service Routine
 *
 * This ISR manages all TX/RX proceedures with the exception of transfer
 * initialization. Once a transfer (read or write) is underway, this method
 * assures the correct amount of bytes are written to the correct location.
 *************************************************************************/
#pragma vector=USCI_A1_VECTOR
__interrupt void usciA1Isr(void)
{
	unsigned int dummy = 0xFF;
	// Transmit Interrupt Flag Set
	if(UCA1IFG & UCTXIFG){
		if(uca1TxSize > 0){
			UCA1TXBUF = *(uca1TxPtr++);				// Transmit the next outgoing byte
			uca1TxSize--;
		}
		else{
			UCA1IV &= ~UCIVTXIFG;					// Clear TX interrupt flag from vector on end of TX
			usciStat[UCA1_INDEX] = OPEN; 			// Set status open if done with transmit
		}
	}

	// Receive Interrupt Flag Set
	if(UCA1IFG & UCRXIFG){
#ifdef USE_UCA1_SPI
		if(usciStat[UCA1_INDEX] == RX)				// Check we are in RX mode for SPI
#endif // USE_UCA1_SPI
		if(UCA1STATW & UCRXERR) dummy = UCA1RXBUF;	// RX ERROR: Do a dummy read to clear interrupt flag
		else {										// Otherwise write the value to the RX pointer
			*(uca1RxPtr++) = UCA1RXBUF;
			uca1RxSize++;							// RX Size decrement in read function
#ifdef USE_UCA1_SPI
			if(usciStat[UCA1_INDEX] == RX && uca1RxSize < spiA1RxSize) UCA1TXBUF = dummy; // Perform another dummy write
			else
#endif
			usciStat[UCA1_INDEX] = OPEN;

		}
	}
}
#endif // USE_UCA1



/****************************************************************
 * USCI B0 Variable Declarations
 ***************************************************************/
#ifdef USE_UCB0
unsigned char *ucb0TxPtr;				///< USCI B0 TX Data Pointer
unsigned char *ucb0RxPtr;				///< USCI B0 RX Data Pointer
unsigned int ucb0TxSize = 0;			///< USCI B0 TX Size
unsigned int ucb0RxSize = 0;			///< USCI B0 RX Size
// Conditional SPI Receive size
unsigned int spiB0RxSize = 0;			///< USCI B0 to-RX Size

/**************************************************************
 * General Purpose USCI B0 Functions
 *************************************************************/
/**************************************************************************//**
 * \brief	Configures USCI B0 for operation
 *
 * Write control registers and clear system variables for the
 * USCI B0 module, which can be used as either a UART or SPI.
 *
 * \param	commID	The communication ID for the registered app
 ******************************************************************************/
inline void confUCB0(unsigned int commID)
{
	if(devConf[UCB0_INDEX] == commID) return;	// Check if device is already configured

	UCB0CTL1 |= UCSWRST;							// Pause operation
	UCB0_IO_CONF(dev[commID].rAddr & ADDR_MASK);	// Port set up

	// Configure key control words
	UCB0CTLW0 = dev[commID].usciCtlW0;
	UCB0CTLW1 = dev[commID].usciCtlW1;
	UCB0BRW = dev[commID].baudDiv;
	ucb0RxPtr = dev[commID].rxPtr;

	// Clear buffer sizes
	ucb0RxSize = 0;
	ucb0TxSize = 0;
#ifdef USE_UCB0_SPI
	spiB0RxSize = 0;
#endif //USE_UCB0_SPI

	UCB0IE |= UCRXIE + UCTXIE;					// Enable Interrupts
	UCB0CTL1 &= ~UCSWRST;						// Resume operation

	devConf[UCB0_INDEX] = commID;				// Store config
}
/**************************************************************************//**
 * \brief	Resets USCI B0 without writing over control regs
 *
 *	This function is included to soft-reset the USCI B0 module
 *	management variables without clearing the current config.
 *
 *	\param		commID	The comm ID of the registered app
 *	\sideeffect	Sets the RX pointer to that registered w/ commID
 ******************************************************************************/
void resetUCB0(unsigned int commID){
	ucb0RxPtr = dev[commID].rxPtr;
	ucb0RxSize = 0;
	ucb0TxSize = 0;
#ifdef USE_UCB0_SPI
	spiB0RxSize = 0;
#endif //USE_UCB0_SPI
	usciStat[UCB0_INDEX] = OPEN;
	return;
}
/**************************************************************************//**
 * \brief	Get method for USCI B0 RX buffer size
 *
 *	Returns the number of bytes which have been written
 *	to the RX pointer (since the last read performed).
 *
 *	\param commID	Comm ID of app. (not used)
 *	\return			The number of valid bytes following the rxPtr.
 ******************************************************************************/
unsigned int getUCB0RxSize(commID){
	return ucb0RxSize;
}
/**************************************************************************//**
 * \brief	Get method for USCI B0 status
 *
 *	Returns the status of the USCI module, either OPEN, TX, or RX
 *
 *	\param commID	Comm ID of app. (not used)
 *	\retval		0	Indicates the OPEN status
 *	\retval 	1	Indicates the TX Status
 *	\retval 	2	Indicates the RX Status
 ******************************************************************************/
unsigned char getUCB0Stat(void){
	return usciStat[UCB0_INDEX];
}

/***********************************************************
 * UCB0 SPI HANDLERS
 ***********************************************************/
#ifdef USE_UCB0_SPI
/**************************************************************************//**
 * \brief	Transmit method for USCI B0 SPI operation
 *
 * This method initializes a transmission of len bytes from the base of the
 * *data pointer. Similarly to uartB0Write(), the transmission uses the USCI B0
 * TX ISR to complete, so 2 sequential calls may result in partial transmission.
 *
 * \param	*data	Pointer to data to be written
 * \param	len		Length (in bytes) of data to be written
 * \param 	commID	Communication ID number of application
 *
 * \retval	-1		USCI B0 Module busy
 * \retval	1		Transmit successfully started
 *******************************************************************************/
int spiB0Write(unsigned char *data, unsigned int len, unsigned int commID)
{
	if(usciStat[UCB0_INDEX] != OPEN) return -1;		// Busy
	confUCB0(commID);
	// Copy over pointer and length
	ucb0TxPtr = data;
	ucb0TxSize = len-1;
	// Start of TX
	UCB0TXBUF = *ucb0TxPtr;
	usciStat[UCB0_INDEX] = TX;

	return 1;
}
/**************************************************************************//**
 * \brief	Receive method for USCI B0 SPI operation
 *
 * This method performs a synchronous read by storing the bytes to be read in
 * spiB0RxSize and then performing len dummy write to the bus to fetch the data
 * from a slave device. The RX size is cleared on this function call.
 *
 * \param	len		The number of bytes to be read from the bus
 * \param	commID	Communication ID number of the application
 *
 * 	\retval	-1		USCI B0 Module Busy
 * 	\retval	1		Receive successfully started
 ******************************************************************************/
int spiB0Read(unsigned int len, unsigned int commID)
{
	if(usciStat[UCB0_INDEX] != OPEN) return -1;		// Busy
	confUCB0(commID);
	// Clear RX Size and copy length
	ucb0RxSize = 0;
	spiB0RxSize = len-1;
	// Start of RX
	UCB0TXBUF = 0xFF;								// Start TX
	usciStat[UCB0_INDEX] = RX;
	return 1;
}
#endif //USE_UCB0_SPI
/***********************************************************
 * UCB0 I2C HANDLERS
 ***********************************************************/
#ifdef USE_UCB0_I2C
//I2C Write
int i2cB0Write(unsigned char *data, unsigned int len, unsigned int commID)
{
	// Do this
}

int i2cB0Read(unsigned int len, unsigned int commID)
{
	// Do this too
}
#endif //USE_UCB0_I2C
/**********************************************************************//**
 * \brief	USCI B0 RX/TX Interrupt Service Routine
 *
 * This ISR manages all TX/RX proceedures with the exception of transfer
 * initialization. Once a transfer (read or write) is underway, this method
 * assures the correct amount of bytes are written to the correct location.
 *************************************************************************/
#pragma vector=USCI_B0_VECTOR
__interrupt void usciB0Isr(void)
{
	unsigned int dummy = 0xFF;
	// Transmit Interrupt Flag Set
	if(UCB0IFG & UCTXIFG){
		if(ucb0TxSize > 0){
			UCB0TXBUF = *(ucb0TxPtr++);				// Transmit the next outgoing byte
			ucb0TxSize--;
		}
		else{
			UCB0IV &= ~UCIVTXIFG;					// Clear TX interrupt flag from vector on end of TX
			usciStat[UCB0_INDEX] = OPEN; 			// Set status open if done with transmit
		}
	}

	// Receive Interrupt Flag Set
	if(UCB0IFG & UCRXIFG){
#ifdef USE_UCB0_SPI
		if(usciStat[UCB0_INDEX] == RX)				// Check we are in RX mode for SPI
#endif // USE_UCB0_SPI
		if(UCB0STATW & UCRXERR) dummy = UCB0RXBUF;	// RX ERROR: Do a dummy read to clear interrupt flag
		else {										// Otherwise write the value to the RX pointer
			*(ucb0RxPtr++) = UCB0RXBUF;
			ucb0RxSize++;							// RX Size decrement in read function
#ifdef USE_UCB0_SPI
			if(usciStat[UCB0_INDEX] == RX && ucb0RxSize < spiB0RxSize) UCB0TXBUF = dummy; // Perform another dummy write
			else
#endif
			usciStat[UCB0_INDEX] = OPEN;

		}
	}
}
#endif // USE_UCB0
