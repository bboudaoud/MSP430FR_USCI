#include "comm.h"
#include "comm_hal.h"

usciConfig *dev[MAX_DEVS];					///< Device config buffer (indexed by comm ID always non-zero)
unsigned int devIndex = 0;					///< Device config buffer index
unsigned int devConf[4] = {0,0,0,0};				///< Currently applied configs buffer [A0, A1, B0, B1]
unsigned char usciStat[4] = {OPEN, OPEN, OPEN, OPEN};		///< Store status (OPEN, TX, or RX) for [A0, A1, B0, B1]

/**************************************************************************//**
 * \brief Registers an application for use of a USCI module.
 *
 * Create a USCI "socket" by affiliating a unique comm ID with an
 * endpoint configuration for TI's eUSCI module.
 *
 * \Param	conf	The USCI configuration structure to be used (see comm.h)
 * \Return	commID 	A positive (> 0) value representing the registered
 * 					app
 * \Retval 	-1	The maximum number of apps (MAX_DEVS) has been registered
 ******************************************************************************/
int registerComm(usciConfig *conf)
{
	if(devIndex >= MAX_DEVS) return -1;	// Check device list not full
	dev[++devIndex] = conf;			// Copy config pointer into device list
	return devIndex;
}

/****************************************************************
 * USCI A0 Variable Declarations
 ******************************************************************************/
#ifdef USE_UCA0
unsigned char *uca0TxPtr;			///< USCI A0 TX Data Pointer
unsigned char *uca0RxPtr;			///< USCI A0 RX Data Pointer
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
 * NOTE: This config function is already called before any read/write function call
 * and therefore should (in almost all cases) never be called by the user.
 *
 * \param	commID	The communication ID for the registered app
 ******************************************************************************/
void confUCA0(unsigned int commID)
{
	unsigned int status;

	if(devConf[UCA0_INDEX] == commID) return;		// Check if device is already configured
	enter_critical(status);					// Perform config in critical section
	UCA0CTL1 |= UCSWRST;					// Pause operation
	UCA0_IO_CLEAR();					// Clear I/O for configuration

	// Configure key control words
	UCA0CTLW0 = dev[commID]->usciCtlW0 | UCSWRST;
#ifdef UCA0CTLW1	// Check for control word 1 define (eUSCI vs USCI) future patch
	UCA0CTLW1 = dev[commID]->usciCtlW1;
#endif // UCA0CTLW1
	UCA0BRW = dev[commID]->baudDiv;
	uca0RxPtr = dev[commID]->rxPtr;

	// Clear buffer sizes
	uca0RxSize = 0;
	uca0TxSize = 0;
#ifdef USE_UCA0_SPI
	spiA0RxSize = 0;
#endif //USE_UCA0_SPI

	UCA0_IO_CONF(dev[commID]->rAddr & ADDR_MASK);		// Port set up
	UCA0CTL1 &= ~UCSWRST;					// Resume operation (clear software reset)
	UCA0IE |= UCRXIE + UCTXIE;				// Enable Interrupts	

	devConf[UCA0_INDEX] = commID;				// Store config
	exit_critical(status);					// End critical section
}
/**************************************************************************//**
 * \brief	Resets USCI A0 without writing over control regs
 *
 * This function is included to soft-reset the USCI A0 module
 * management variables without clearing the current config.
 *
 * \param	commID	The comm ID of the registered app
 *\sideeffect	Sets the RX pointer to that registered w/ commID
 ******************************************************************************/
void resetUCA0(unsigned int commID){
	uca0RxPtr = dev[commID]->rxPtr;
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
 * Returns the number of bytes which have been written
 * to the RX pointer (since the last read performed).
 *
 * \param commID	Comm ID of app. (not used)
 * \return			The number of valid bytes following the rxPtr.
 ******************************************************************************/
unsigned int getUCA0RxSize(unsigned int commID){
	return uca0RxSize;
}
/**************************************************************************//**
 * \brief	Get method for USCI A0 status
 *
 * Returns the status of the USCI module, either OPEN, TX, or RX
 *
 * \param commID	Comm ID of app. (not used)
 * \retval		0	Indicates the OPEN status
 * \retval 	1	Indicates the TX Status
 * \retval 	2	Indicates the RX Status
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
	dev[commID]->baudDiv = baudDiv;		// Replace the baud divisor in memory
	devConf[UCA0_INDEX] = 0;		// Reset the device config storage (config will be performed on next read/write)
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
 * \param	len	The number of bytes to be read from the buffer
 * \param	commID	The comm ID of the application
 * \return	The number of bytes available to read in the buffer. If the buffer
 * 			is empty this value will be 0.
 * \sideeffect	The uca0RxSize variable is decremented by the min of itself and
 * 				the requested amount of bytes (len).
 * \sa		This function does not make a call to #confUCA0 or check it the
 * 		state of the USCI module as it does  not interact with the
 * 		hardware module.
 ******************************************************************************/
int uartA0Read(unsigned int len, unsigned int commID)
{
	// Read length determination = max(requested, available)
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
 * \param	len	Length (in bytes) of data to be written
 * \param 	commID	Communication ID number of application
 *
 * \retval	-1	USCI A0 Module busy
 * \retval	1	Transmit successfully started
 *******************************************************************************/
int spiA0Write(unsigned char *data, unsigned int len, unsigned int commID)
{
	if(usciStat[UCA0_INDEX] != OPEN) return -1;	// Check that USCI is available

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
 * \param	len	The number of bytes to be read from the bus
 * \param	commID	Communication ID number of the application
 *
 * \retval	-1	USCI A0 Module Busy
 * \retval	1	Receive successfully started
 *
 * \sideeffect	Reset the UCA0 RX size and data pointer
 ******************************************************************************/
int spiA0Read(unsigned int len, unsigned int commID)
{
	if(usciStat[UCA0_INDEX] != OPEN) return -1;	// Check that USCI is available

	confUCA0(commID);
	
	// Clear RX Size/Buff and copy length
	uca0RxSize = 0;					// Reset the rx size
	uca0RxPtr = dev[commID]->rxPtr;			// Reset the rx pointer
	spiA0RxSize = len-1;
	// Start of RX
	UCA0TXBUF = 0xFF;				// Start TX
	usciStat[UCA0_INDEX] = RX;
	return 1;
}
/**************************************************************************//**
 * \brief	Byte Swap method for USCI A0 SPI operation
 *
 * This blocking method allows the user to transmit a byte and receive the
 * response simultaneously clocked back in. TX and RX buffer sizes/content
 * are unaffected.
 *
 * \param	byte	The byte to be sent via SPI
 * \param 	commID	Communication ID number of the application
 *
 * \return	The byte shifted in from the SPI
 *************************************************************************/
unsigned char spiA0Swap(unsigned char byte, unsigned int commID)
{
	if(usciStat[UCA0_INDEX] != OPEN) return -1;	// Check that USCI is available

	confUCA0(commID);
	
	UCA0TXBUF = byte;
	while(UCA0STATW & UCBUSY);			// Wait for TX complete
	return UCA0RXBUF;				// Return RX contents
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
			UCA0TXBUF = *(++uca0TxPtr);		// Transmit the next outgoing byte
			uca0TxSize--;
		}
		else{
			UCA0IFG &= ~UCTXIFG;			// Clear TX interrupt flag from vector on end of TX
			usciStat[UCA0_INDEX] = OPEN; 		// Set status open if done with transmit
		}
	}

	// Receive Interrupt Flag Set
	if(UCA0IFG & UCRXIFG){
#ifdef USE_UCA0_SPI
		if(usciStat[UCA0_INDEX] == RX)			// Check we are in RX mode for SPI
#endif // USE_UCA0_SPI
		if(UCA0STATW & UCRXERR) dummy = UCA0RXBUF;	// RX ERROR: Do a dummy read to clear interrupt flag
		else {						// Otherwise write the value to the RX pointer
			*(uca0RxPtr++) = UCA0RXBUF;
			uca0RxSize++;				// RX Size decrement in read function
#ifdef USE_UCA0_SPI
			if(usciStat[UCA0_INDEX] == RX && uca0RxSize < spiA0RxSize) UCA0TXBUF = dummy; // Perform another dummy write
			else
#endif
			usciStat[UCA0_INDEX] = OPEN;

		}
	}
	UCA0IFG &= ~UCRXIFG;					// Clear RX interrupt flag from vector on end of RX
}
#endif // USE_UCA0


/****************************************************************
 * USCI A1 Variable Declarations
 ***************************************************************/
#ifdef USE_UCA1
unsigned char *uca1TxPtr;		///< USCI A1 TX Data Pointer
unsigned char *uca1RxPtr;		///< USCI A1 RX Data Pointer
unsigned int uca1TxSize = 0;		///< USCI A1 TX Size
unsigned int uca1RxSize = 0;		///< USCI A1 RX Size
// Conditional SPI Receive size
#ifdef USE_UCA1_SPI
unsigned int spiA1RxSize = 0;		///< USCI A1 To-RX Size (used for SPI RX)
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
 * NOTE: This config function is already called before any read/write function call
 * and therefore should (in almost all cases) never be called by the user.
 *
 * \param	commID	The communication ID for the registered app
 ******************************************************************************/
void confUCA1(unsigned int commID)
{
	unsigned int status;
	if(devConf[UCA1_INDEX] == commID) return;		// Check if device is already configured
	enter_critical(status);					// Perform config in critical section
	UCA1CTL1 |= UCSWRST;					// Pause operation
	UCA1_IO_CLEAR();					// Clear I/O for config

	// Configure key control words
	UCA1CTLW0 = dev[commID]->usciCtlW0 | UCSWRST;
#ifdef UCA1CTLW1	// Check for UCA1CTLW1 defined
	UCA1CTLW1 = dev[commID]->usciCtlW1;
#endif // UCA1CTLW1
	UCA1BRW = dev[commID]->baudDiv;
	uca1RxPtr = dev[commID]->rxPtr;

	// Clear buffer sizes
	uca1RxSize = 0;
	uca1TxSize = 0;
#ifdef USE_UCA1_SPI
	spiA1RxSize = 0;
#endif //USE_UCA1_SPI

	UCA1_IO_CONF(dev[commID]->rAddr & ADDR_MASK);		// Port set up
	UCA1CTL1 &= ~UCSWRST;					// Resume operation
	UCA1IE |= UCRXIE + UCTXIE;				// Enable Interrupts	

	devConf[UCA1_INDEX] = commID;				// Store config
	exit_critical(status);					// End critical section
}

/**************************************************************************//**
 * \brief	Resets USCI A1 without writing over control regs
 *
 * This function is included to sof-reset the USCI A1 module
 * management variables without clearing the current config.
 *
 * \param	commID	The comm ID of the registered app
 * \sideeffect	Sets the RX pointer to that registered w/ commID
 ******************************************************************************/
void resetUCA1(unsigned int commID){
	uca1RxPtr = dev[commID]->rxPtr;
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
 * Returns the number of bytes which have been written
 * to the RX pointer (since the last read performed).
 *
 * \param 	commID	Comm ID of app. (not used)
 * \return	The number of valid bytes following the rxPtr.
 ******************************************************************************/
unsigned int getUCA1RxSize(unsigned int commID){
	return uca1RxSize;
}
/**************************************************************************//**
 * \brief	Get method for USCI A1 status
 *
 *Returns the status of the USCI module, either OPEN, TX, or RX
 *
 * \param	commID	Comm ID of app. (not used)
 * \retval	0	Indicates the OPEN status
 * \retval 	1	Indicates the TX Status
 * \retval 	2	Indicates the RX Status
 ******************************************************************************/
unsigned char getUCA1Stat(void){
	return usciStat[UCA1_INDEX];
}

/*************************************************************************//**
 * \brief	Set method for USCI A1 Baud Rate Divisor
 *
 * Sets the baud rate divisor of the USCI module, this divisor is generally
 * performed relative to the SMCLK rate of the system.
 *
 * \param 	baudDiv	The new divisor to apply
 * \param	commID	The communications ID number of the application
 *****************************************************************************/
void setUCA1Baud(unsigned int baudDiv, unsigned int commID){
	dev[commID]->baudDiv = baudDiv;		// Replace the baud divisor in memory
	devConf[UCA1_INDEX] = 0;		// Reset the device config storage (config will be performed on next read/write)
	return;
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
	if(usciStat[UCA1_INDEX] != OPEN) return -1;	// Check that the USCI is available

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
 * \param	commID		The comm ID of the application
 * \return	The number of bytes available to read in the buffer. If the buffer
 * 		is empty this value will be 0.
 * \sideeffect	The ucA1RxSize variable is decremented by the min of itself and
 * 		the requested amount of bytes (len).
 * \sa		This function does not make a call to #confUCA0 or check it the
 * 		state of the USCI module as it does  not interact with the
 * 		hardware module.
 ******************************************************************************/
int uartA1Read(unsigned int len, unsigned int commID)
{
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
	if(usciStat[UCA1_INDEX] != OPEN) return -1;	// Check that the USCI is available

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
 * \param	len	The number of bytes to be read from the bus
 * \param	commID	Communication ID number of the application
 *
 * \retval	-1	USCI A1 Module Busy
 * \retval	1	Receive successfully started
 *
 * \sideeffect		Reset the UCA1 RX size and data pointer
 ******************************************************************************/
int spiA1Read(unsigned int len, unsigned int commID)
{
	if(usciStat[UCA1_INDEX] != OPEN) return -1;	// Check that the USCI is available
	
	confUCA1(commID);
	
	// Clear RX Size and copy length
	uca1RxSize = 0;					// Reset RX size
	uca1RxPtr = dev[commID]->rxPtr;			// Reset RX pointer
	spiA1RxSize = len-1;
	// Start of RX
	UCA1TXBUF = 0xFF;				// Start TX
	usciStat[UCA1_INDEX] = RX;
	return 1;
}

/**************************************************************************//**
 * \brief	Byte Swap method for USCI A1 SPI operation
 *
 * This blocking method allows the user to transmit a byte and receive the
 * response simultaneously clocked back in. TX and RX buffer sizes/content
 * are unaffected.
 *
 * \param	byte	The byte to be sent via SPI
 * \param 	commID	Communication ID number of the application
 *
 * \return	The byte shifted in from the SPI
 *************************************************************************/
unsigned char spiA1Swap(unsigned char byte, unsigned int commID)
{
	if(usciStat[UCA1_INDEX] != OPEN) return -1;	// Check that the USCI is available

	confUCA1(commID);
	
	UCA1TXBUF = byte;
	while(UCA1STATW & UCBUSY);			// Wait for TX complete
	return UCA1RXBUF;				// Return RX contents
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
			UCA1TXBUF = *(++uca1TxPtr);		// Transmit the next outgoing byte
			uca1TxSize--;
		}
		else{
			UCA1IFG &= ~UCTXIFG;			// Clear TX interrupt flag from vector on end of TX
			usciStat[UCA1_INDEX] = OPEN; 		// Set status open if done with transmit
		}
	}

	// Receive Interrupt Flag Set
	if(UCA1IFG & UCRXIFG){
#ifdef USE_UCA1_SPI
		if(usciStat[UCA1_INDEX] == RX)			// Check we are in RX mode for SPI
#endif // USE_UCA1_SPI
		if(UCA1STATW & UCRXERR) dummy = UCA1RXBUF;	// RX ERROR: Do a dummy read to clear interrupt flag
		else {						// Otherwise write the value to the RX pointer
			*(uca1RxPtr++) = UCA1RXBUF;
			uca1RxSize++;				// RX Size decrement in read function
#ifdef USE_UCA1_SPI
			if(usciStat[UCA1_INDEX] == RX && uca1RxSize < spiA1RxSize) UCA1TXBUF = dummy; // Perform another dummy write
			else
#endif
			usciStat[UCA1_INDEX] = OPEN;

		}
	}
	UCA1IFG &= ~UCRXIFG;					// Clear RX interrupt flag from vector on end of RX
}
#endif // USE_UCA1

/****************************************************************
 * USCI B0 Variable Declarations
 ***************************************************************/
#ifdef USE_UCB0
unsigned char *ucb0TxPtr;			///< USCI B0 TX Data Pointer
unsigned char *ucb0RxPtr;			///< USCI B0 RX Data Pointer
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
 * NOTE: This config function is already called before any read/write function call
 * and therefore should (in almost all cases) never be called by the user.
 *
 * \param	commID	The communication ID for the registered app
 ******************************************************************************/
void confUCB0(unsigned int commID)
{
	unsigned int status;
	if(devConf[UCB0_INDEX] == commID) return;	// Check if device is already configured
	enter_critical(status);				// Perform config in critical section
	UCB0CTL1 |= UCSWRST;				// Assert USCI software reset
	UCB0_IO_CLEAR();				// Clear I/O for configuration

	// Configure key control words
	UCB0CTLW0 = dev[commID]->usciCtlW0 | UCSWRST;
#ifdef UCB0CTLW1	// Check for UCB0CTLW1 defined
	UCB0CTLW1 = dev[commID]->usciCtlW1;
#endif //UCB0CTLW1
	UCB0BRW = dev[commID]->baudDiv;
	ucb0RxPtr = dev[commID]->rxPtr;

	// Clear buffer sizes
	ucb0RxSize = 0;
	ucb0TxSize = 0;
#ifdef USE_UCB0_SPI
	spiB0RxSize = 0;
#endif //USE_UCB0_SPI

	UCB0_IO_CONF(dev[commID]->rAddr & ADDR_MASK);	// Port set up
	UCB0CTL1 &= ~UCSWRST;				// Resume operation
	UCB0IE |= UCRXIE + UCTXIE;			// Enable Interrupts	

	devConf[UCB0_INDEX] = commID;			// Store config
	exit_critical(status);				// End critical section
}
/**************************************************************************//**
 * \brief	Resets USCI B0 without writing over control regs
 *
 * This function is included to soft-reset the USCI B0 module
 * management variables without clearing the current config.
 *
 * \param	commID	The comm ID of the registered app
 * \sideeffect	Sets the RX pointer to that registered w/ commID
 ******************************************************************************/
void resetUCB0(unsigned int commID){
	ucb0RxPtr = dev[commID]->rxPtr;
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
 * Returns the number of bytes which have been written
 * to the RX pointer (since the last read performed).
 *
 * \param 	commID	Comm ID of app. (not used)
 * \return	The number of valid bytes following the rxPtr.
 ******************************************************************************/
unsigned int getUCB0RxSize(unsigned int commID){
	return ucb0RxSize;
}
/**************************************************************************//**
 * \brief	Get method for USCI B0 status
 *
 * Returns the status of the USCI module, either OPEN, TX, or RX
 *
 * \param 	commID	Comm ID of app. (not used)
 * \retval	0	Indicates the OPEN status
 * \retval 	1	Indicates the TX Status
 * \retval 	2	Indicates the RX Status
 ******************************************************************************/
unsigned char getUCB0Stat(void){
	return usciStat[UCB0_INDEX];
}

/*************************************************************************//**
 * \brief	Set method for USCI B0 Baud Rate Divisor
 *
 * Sets the baud rate divisor of the USCI module, this divisor is generally
 * performed relative to the SMCLK rate of the system.
 *
 * \param 	baudDiv	The new divisor to apply
 * \param	commID	The communications ID number of the application
 *****************************************************************************/
void setUCB0Baud(unsigned int baudDiv, unsigned int commID){
	dev[commID]->baudDiv = baudDiv;		// Replace the baud divisor in memory
	devConf[UCB0_INDEX] = 0;		// Reset the device config storage (config will be performed on next read/write)
	return;
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
	if(usciStat[UCB0_INDEX] != OPEN) return -1;	// Check that the USCI is available

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
 * \param	len	The number of bytes to be read from the bus
 * \param	commID	Communication ID number of the application
 *
 * \retval	-1	USCI B0 Module Busy
 * \retval	1	Receive successfully started
 *
 * \sideeffect	Reset the UCB0 RX size and data pointer
 ******************************************************************************/
int spiB0Read(unsigned int len, unsigned int commID)
{
	if(usciStat[UCB0_INDEX] != OPEN) return -1;	// Check that the USCI is available

	confUCB0(commID);

	// Clear RX Size and copy length
	ucb0RxSize = 0;					// Reset the rx size
	ucb0RxPtr = dev[commID]->rxPtr;			// Reset the rx pointer
	spiB0RxSize = len-1;
	// Start of RX
	UCB0TXBUF = 0xFF;				// Start TX
	usciStat[UCB0_INDEX] = RX;
	return 1;
}

/**************************************************************************//**
 * \brief	Byte Swap method for USCI B0 SPI operation
 *
 * This blocking method allows the user to transmit a byte and receive the
 * response simultaneously clocked back in. TX and RX buffer sizes/content
 * are unaffected.
 *
 * \param	byte	The byte to be sent via SPI
 * \param 	commID	Communication ID number of the application
 *
 * \return	The byte shifted in from the SPI
 *************************************************************************/
unsigned char spiB0Swap(unsigned char byte, unsigned int commID)
{
	if(usciStat[UCB0_INDEX] != OPEN) return -1;	// Check that the USCI is available
	
	confUCB0(commID);
	
	ucb0TxSize = 0;
	UCB0TXBUF = byte;
	while(UCB0STATW & UCBUSY);			// Wait for TX complete
	return UCB0RXBUF;				// Return RX contents
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
			UCB0TXBUF = *(++ucb0TxPtr);			// Transmit the next outgoing byte
			ucb0TxSize--;
		}
		else{
			usciStat[UCB0_INDEX] = OPEN; 			// Set status open if done with transmit
			UCB0IFG &= ~UCTXIFG;				// Clear TX interrupt flag from vector on end of TX
		}
	}

	// Receive Interrupt Flag Set
	if(UCB0IFG & UCRXIFG){	// Check for interrupt flag and RX mode
#ifdef USE_UCB0_SPI
		if(usciStat[UCB0_INDEX] == RX)				// Check we are in RX mode for SPI
#endif // USE_UCB0_SPI
		if(UCB0STATW & UCRXERR) dummy = UCB0RXBUF;		// RX ERROR: Do a dummy read to clear interrupt flag
		else {							// Otherwise write the value to the RX pointer
			*(ucb0RxPtr++) = UCB0RXBUF;
			ucb0RxSize++;					// RX Size decrement in read function
#ifdef USE_UCB0_SPI
			if(usciStat[UCB0_INDEX] == RX && ucb0RxSize < spiB0RxSize) UCB0TXBUF = dummy; // Perform another dummy write
			else
#endif
			usciStat[UCB0_INDEX] = OPEN;

		}
	}

	UCB0IFG &= ~UCRXIFG;						// Clear RX interrupt flag from vector on end of RX
}
#endif // USE_UCB0

/****************************************************************
 * USCI B1 Variable Declarations
 ***************************************************************/
#ifdef USE_UCB1
unsigned char *ucb1TxPtr;			///< USCI B1 TX Data Pointer
unsigned char *ucb1RxPtr;			///< USCI B1 RX Data Pointer
unsigned int ucb1TxSize = 0;			///< USCI B1 TX Size
unsigned int ucb1RxSize = 0;			///< USCI B1 RX Size
// Conditional SPI Receive size
unsigned int spiB1RxSize = 0;			///< USCI B1 to-RX Size

/**************************************************************
 * General Purpose USCI B1 Functions
 *************************************************************/
/**************************************************************************//**
 * \brief	Configures USCI B1 for operation
 *
 * Write control registers and clear system variables for the
 * USCI B0 module, which can be used as either a UART or SPI.
 *
 * NOTE: This config function is already called before any read/write function call
 * and therefore should (in almost all cases) never be called by the user.
 *
 * \param	commID	The communication ID for the registered app
 ******************************************************************************/
void confUCB1(unsigned int commID)
{
	unsigned int status;
	if(devConf[UCB1_INDEX] == commID) return;	// Check if device is already configured
	enter_critical(status);				// Perform config in critical section
	UCB1CTL1 |= UCSWRST;				// Assert USCI software reset
	UCB1_IO_CLEAR();				// Clear I/O for configuration

	// Configure key control words
	UCB1CTLW0 = dev[commID]->usciCtlW0 | UCSWRST;
#ifdef UCB1CTLW1	// Check for UCB1CTLW1 defined
	UCB1CTLW1 = dev[commID]->usciCtlW1;
#endif //UCB1CTLW1
	UCB1BRW = dev[commID]->baudDiv;
	ucb1RxPtr = dev[commID]->rxPtr;

	// Clear buffer sizes
	ucb1RxSize = 0;
	ucb1TxSize = 0;
#ifdef USE_UCB1_SPI
	spiB1RxSize = 0;
#endif //USE_UCB1_SPI

	UCB1_IO_CONF(dev[commID]->rAddr & ADDR_MASK);	// Port set up
	UCB1CTL1 &= ~UCSWRST;				// Resume operation
	UCB1IE |= UCRXIE + UCTXIE;			// Enable Interrupts	

	devConf[UCB1_INDEX] = commID;			// Store config
	exit_critical(status);				// End critical section
}
/**************************************************************************//**
 * \brief	Resets USCI B1 without writing over control regs
 *
 * This function is included to soft-reset the USCI B1 module
 * management variables without clearing the current config.
 *
 * \param	commID	The comm ID of the registered app
 * \sideeffect	Sets the RX pointer to that registered w/ commID
 ******************************************************************************/
void resetUCB1(unsigned int commID){
	ucb1RxPtr = dev[commID]->rxPtr;
	ucb1RxSize = 0;
	ucb1TxSize = 0;
#ifdef USE_UCB1_SPI
	spiB1RxSize = 0;
#endif //USE_UCB1_SPI
	usciStat[UCB1_INDEX] = OPEN;
	return;
}
/**************************************************************************//**
 * \brief	Get method for USCI B1 RX buffer size
 *
 * Returns the number of bytes which have been written
 * to the RX pointer (since the last read performed).
 *
 * \param commID	Comm ID of app. (not used)
 * \return			The number of valid bytes following the rxPtr.
 ******************************************************************************/
unsigned int getUCB1RxSize(unsigned int commID){
	return ucb1RxSize;
}
/**************************************************************************//**
 * \brief	Get method for USCI B1 status
 *
 * Returns the status of the USCI module, either OPEN, TX, or RX
 *
 * \param 	commID	Comm ID of app. (not used)
 * \retval	0	Indicates the OPEN status
 * \retval 	1	Indicates the TX Status
 * \retval 	2	Indicates the RX Status
 ******************************************************************************/
unsigned char getUCB1Stat(void){
	return usciStat[UCB1_INDEX];
}

/*************************************************************************//**
 * \brief	Set method for USCI B1 Baud Rate Divisor
 *
 * Sets the baud rate divisor of the USCI module, this divisor is generally
 * performed relative to the SMCLK rate of the system.
 *
 * \param 	baudDiv	The new divisor to apply
 * \param	commID	The communications ID number of the application
 *****************************************************************************/
void setUCB1Baud(unsigned int baudDiv, unsigned int commID){
	dev[commID]->baudDiv = baudDiv;		// Replace the baud divisor in memory
	devConf[UCB1_INDEX] = 0;		// Reset the device config storage (config will be performed on next read/write)
	return;
}

/***********************************************************
 * UCB0 SPI HANDLERS
 ***********************************************************/
#ifdef USE_UCB1_SPI
/**************************************************************************//**
 * \brief	Transmit method for USCI B1 SPI operation
 *
 * This method initializes a transmission of len bytes from the base of the
 * *data pointer. Similarly to uartB0Write(), the transmission uses the USCI B1
 * TX ISR to complete, so 2 sequential calls may result in partial transmission.
 *
 * \param	*data	Pointer to data to be written
 * \param	len		Length (in bytes) of data to be written
 * \param 	commID	Communication ID number of application
 *
 * \retval	-1		USCI B1 Module busy
 * \retval	1		Transmit successfully started
 *******************************************************************************/
int spiB1Write(unsigned char *data, unsigned int len, unsigned int commID)
{
	if(usciStat[UCB1_INDEX] != OPEN) return -1;	// Check that the USCI is available

	confUCB1(commID);
	
	// Copy over pointer and length
	ucb1TxPtr = data;
	ucb1TxSize = len-1;
	// Start of TX
	UCB1TXBUF = *ucb0TxPtr;
	usciStat[UCB1_INDEX] = TX;

	return 1;
}
/**************************************************************************//**
 * \brief	Receive method for USCI B1 SPI operation
 *
 * This method performs a synchronous read by storing the bytes to be read in
 * spiB1RxSize and then performing len dummy write to the bus to fetch the data
 * from a slave device. The RX size is cleared on this function call.
 *
 * \param	len		The number of bytes to be read from the bus
 * \param	commID	Communication ID number of the application
 *
 * 	\retval	-1		USCI B1 Module Busy
 * 	\retval	1		Receive successfully started
 *
 * 	\sideeffect		Reset the UCB0 RX size and data pointer
 ******************************************************************************/
int spiB0Read(unsigned int len, unsigned int commID)
{
	if(usciStat[UCB1_INDEX] != OPEN) return -1;	// Check that the USCI is available

	confUCB1(commID);

	// Clear RX Size and copy length
	ucb1RxSize = 0;					// Reset the rx size
	ucb1RxPtr = dev[commID]->rxPtr;			// Reset the rx pointer
	spiB1RxSize = len-1;
	// Start of RX
	UCB1TXBUF = 0xFF;				// Start TX
	usciStat[UCB1_INDEX] = RX;
	return 1;
}

/**************************************************************************//**
 * \brief	Byte Swap method for USCI B1 SPI operation
 *
 * This blocking method allows the user to transmit a byte and receive the
 * response simultaneously clocked back in. TX and RX buffer sizes/content
 * are unaffected.
 *
 * \param	byte	The byte to be sent via SPI
 * \param 	commID	Communication ID number of the application
 *
 * \return	The byte shifted in from the SPI
 *************************************************************************/
unsigned char spiB1Swap(unsigned char byte, unsigned int commID)
{
	if(usciStat[UCB1_INDEX] != OPEN) return -1;	// Check that the USCI is available
	
	confUCB1(commID);
	
	ucb1TxSize = 0;
	UCB1TXBUF = byte;
	while(UCB1STATW & UCBUSY);			// Wait for TX complete
	return UCB1RXBUF;				// Return RX contents
}
#endif //USE_UCB1_SPI
/***********************************************************
 * UCB1 I2C HANDLERS
 ***********************************************************/
#ifdef USE_UCB1_I2C
//I2C Write
int i2cB1Write(unsigned char *data, unsigned int len, unsigned int commID)
{
	// Do this
}

int i2cB1Read(unsigned int len, unsigned int commID)
{
	// Do this too
}
#endif //USE_UCB1_I2C
/**********************************************************************//**
 * \brief	USCI B1 RX/TX Interrupt Service Routine
 *
 * This ISR manages all TX/RX proceedures with the exception of transfer
 * initialization. Once a transfer (read or write) is underway, this method
 * assures the correct amount of bytes are written to the correct location.
 *************************************************************************/
#pragma vector=USCI_B1_VECTOR
__interrupt void usciB1Isr(void)
{
	unsigned int dummy = 0xFF;
	// Transmit Interrupt Flag Set
	if(UCB1IFG & UCTXIFG){
		if(ucb1TxSize > 0){
			UCB1TXBUF = *(++ucb1TxPtr);			// Transmit the next outgoing byte
			ucb1TxSize--;
		}
		else{
			usciStat[UCB1_INDEX] = OPEN; 			// Set status open if done with transmit
			UCB1IFG &= ~UCTXIFG;				// Clear TX interrupt flag from vector on end of TX
		}
	}

	// Receive Interrupt Flag Set
	if(UCB1IFG & UCRXIFG){	// Check for interrupt flag and RX mode
#ifdef USE_UCB1_SPI
		if(usciStat[UCB1_INDEX] == RX)				// Check we are in RX mode for SPI
#endif // USE_UCB0_SPI
		if(UCB1STATW & UCRXERR) dummy = UCB1RXBUF;		// RX ERROR: Do a dummy read to clear interrupt flag
		else {							// Otherwise write the value to the RX pointer
			*(ucb1RxPtr++) = UCB1RXBUF;
			ucb1RxSize++;					// RX Size decrement in read function
#ifdef USE_UCB1_SPI
			if(usciStat[UCB1_INDEX] == RX && ucb1RxSize < spiB1RxSize) UCB1TXBUF = dummy; // Perform another dummy write
			else
#endif
			usciStat[UCB1_INDEX] = OPEN;

		}
	}

	UCB1IFG &= ~UCRXIFG;						// Clear RX interrupt flag from vector on end of RX
}
#endif // USE_UCB1

