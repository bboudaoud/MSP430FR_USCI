MSP430FR_USCI
=============

C Libraries for MSP430 Universal Serial Communications Interface (USCI)

Currently 2 versions of the USCI Library for the MSP430FR5739 have been started. Both use a comm registration system to store the serial endpoint configuration of a particular application and return a uniquely identifying comm ID number which is then used to identify all of that application's data transfers.

All data transfers begin by calling a configuration function with this comm ID to set up the device and clear any remaining flagging, this is followed by an ISR-based variable length transmit/receive routine which transfers bytes from/to the data pointer provided by the application developer.

Simple/General USCI Lib (comm.c/h):
  - Include the basic uart/spi/i2c read/write functions with conditional compilation to determine state of each USCI
  - Includes compiler error flagging for multiple serial endpoint configurations and popular present configs
  - UART communication has been fully tested and verified working, SPI testing has began, I2C has not started
  - Still does not have I2C code for USCI B0 included
  - ISR based TX/RX for multi-byte data exchanges
  - All functions/variables/macros use Doxygen formatted comments for easy creation of documentation

General (Power User) USCI Lib (comm2.c/h):
  - Generalized USCI read/write functions
  - No multiple serial endpoint configuration protection
  - Same core ISR based functionality for multi-byte read/write

Current TODO List:
  - Add a single byte, blocking exchange TX/RX SPI function (shift out a byte and return contents of TXBUF)
  - Decide what to do regarding CS management (should leave control to application developer)
  - Complete and test basic I2C ISR functionality (what to do on multi-byte transfer)

In addition to the items above it is worth noting that other seasoned developers may have suggestions for additional/altnerative generic serial input APIs. Additional development/collaboration is both desired and encouraged. Ideally this library will work with any MSP430 device using the current e_USCI peripheral architecture, meaning this library has much broader applications than the inital system it is being developed for.


