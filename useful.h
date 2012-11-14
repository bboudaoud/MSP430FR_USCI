// A group of useful macros for all MSP430 platforms
#ifndef USEFUL_H_
#define USEFUL_H_

// Critical Section Code
#define enter_critical(SR_state)           do { \
  (SR_state) = (_get_SR_register() & 0x08); \
  _disable_interrupts(); \
} while (0)

#define exit_critical(SR_state)         _bis_SR_register(SR_state)

// System reset code
#define systemReset()           do { \
  	IE1 = 0 ; \
    IFG1 &= ~OFIFG ; \
    WDTCTL = 0 ; \
    _DINT() ; \
    _c_int00(); \
} while (0)

// Bit Access Structure
typedef struct Bits8 
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
#define B8_0(x) (((Bits *) (x))->Bitx0)				// Bit in Byte Defines
#define B8_1(x) (((Bits *) (x))->Bitx1)
#define B8_2(x) (((Bits *) (x))->Bitx2)
#define B8_3(x) (((Bits *) (x))->Bitx3)
#define B8_4(x) (((Bits *) (x))->Bitx4)
#define B8_5(x) (((Bits *) (x))->Bitx5)
#define B8_6(x) (((Bits *) (x))->Bitx6)
#define B8_7(x) (((Bits *) (x))->Bitx7)



#endif /*USEFUL_H_*/
