// A group of useful macros for all MSP430 platforms
#ifndef USEFUL_H_
#define USEFUL_H_

// Critical Section Code
#define enter_critical(SR_state)           do {\	
  (SR_state) = (_get_SR_register() & 0x08); \
  _disable_interrupts(); \
} while (0) ///< Critical section entrance macro

#define exit_critical(SR_state)         _bis_SR_register(SR_state) ///< Critical section exit macro

// System reset code
#define systemReset()           do { \
  	IE1 = 0 ; \
    IFG1 &= ~OFIFG ; \
    WDTCTL = 0 ; \
    _DINT() ; \
    _c_int00(); \
} while (0)	///< Software-based (WDT Violation) System Reset Macro

// Bit Access Structure
typedef struct Bits8 ///< Bitwise access structure for a single byte
{
    volatile unsigned Bitx0 : 1 ;
    volatile unsigned Bitx1 : 1 ;
    volatile unsigned Bitx2 : 1 ;
    volatile unsigned Bitx3 : 1 ;
    volatile unsigned Bitx4 : 1 ;
    volatile unsigned Bitx5 : 1 ;
    volatile unsigned Bitx6 : 1 ;
    volatile unsigned Bitx7 : 1 ;
} Bits ;

// Bit Access Defines
#define B8_0(x) (((Bits *) (x))->Bitx0)		///< Bit 0 (LSB) Access Macro
#define B8_1(x) (((Bits *) (x))->Bitx1)		///< Bit 1 Access Macro
#define B8_2(x) (((Bits *) (x))->Bitx2)		///< Bit 2 Access Macro
#define B8_3(x) (((Bits *) (x))->Bitx3)		///< Bit 3 Access Macro
#define B8_4(x) (((Bits *) (x))->Bitx4)		///< Bit 4 Access Macro
#define B8_5(x) (((Bits *) (x))->Bitx5)		///< Bit 5 Access Macro
#define B8_6(x) (((Bits *) (x))->Bitx6)		///< Bit 6 Access Macro
#define B8_7(x) (((Bits *) (x))->Bitx7)		///< Bit 7 (MSB) Access Macro



#endif /*USEFUL_H_*/
