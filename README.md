MSP430FR_USCI
=============

C Libraries for MSP430 Universal Serial Communications Interface (USCI)

The current USCI library has been tested working (UART and SPI) with the MSP430FR5739 MCU from Texas Instruments. Doxygen command file (Doxyfile) and compiled documentation (under "html" subdirectory or linked to by "Documentation.html") are included in this revision. A list of recent fixes/changes is available through the git repository history view.

Notes:
	- All data transfers begin by calling a configuration function with this comm ID to set up the device and clear any remaining flagging, this is followed by an ISR-based variable length transmit/receive routine which transfers bytes from/to the data pointer provided by the application developer. Thus confUCXX need not be called by the application developer
	- Though this library contains code capable of running multiple endpoint configurations (i.e. UART and SPI) simultaneously on one USCI module the conditional compilation macros have been set for error flagging in this case. If the developer is confident that no hardware errors will occur on multi-protocol bus-sharing he/she is free to remove this comiler error flagging and use multiple endpoint configurations
	- Chip-select management (for SPI mode) is currently left to the user (as various devices may respect different CS management rules) in the future this functionality may be roled into the usciConf but for now is left to the user
	- I2C address selection should be managed by the library automatically
	- Additional HAL file attempts to make this C/H library more hardware agnostic, supporting multiple products in the MSP430F5/6xxx line (testing still underway) but for now the MSP430FR5739 code is the only verified platform base
	
Current TODO List:
  - I2C functionality (borrowing an existing TI library to verify against) 
  - Accomodate eUSCI vs. USCI differences for more applications in the MSP430F5/6xxx series products
  - Build a more simplified API document (and some well-commented examples) for users not familiar with Doxygen comments

In addition to the items above it is worth noting that other seasoned developers may have suggestions for additional/altnerative generic serial input APIs. Additional development/collaboration is both desired and encouraged. Ideally this library will work with any MSP430 device using the current e_USCI peripheral architecture, meaning this library has much broader applications than the inital system it is being developed for.


