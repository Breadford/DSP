#ifndef COMMON_H
#define COMMON_H

#include <stdfix.h>
// potrebno prekopirati sa pocetka stdfix_emu.h ili ukljuciti ceo stdfix_emu.h!
#if defined(__CCC)

#include <stdfix.h>

#define FRACT_NUM(x) (x##r)
#define LONG_FRACT_NUM(x) (x##lr)
#define ACCUM_NUM(x) (x##lk)

#define FRACT_NUM_HEX(x) (x##r)

#define FRACT_TO_INT_BIT_CONV(x) (bitsr(x))
#define INT_TO_FRACT_BIT_CONV(x) (rbits(x))

#define long_accum long accum
#define long_fract long fract

#endif

/* Basic constants */
#define BLOCK_SIZE 16
#define MAX_NUM_CHANNEL 8
// Number of channels
#define INPUT_NUM_CHANNELS 2
#define OUTPUT_NUM_CHANNELS 4


// Channel IDs. 
// Should input and output channel IDs be separated?
#define LEFT_CH 0
#define RIGHT_CH 1
#define LS_CH 2
#define RS_CH 3
#define GAIN_PROC_ASM

/* TO DO: Move defined constants here */
/////////////////////////////////////////////////////////////////////////////////
// Constant definitions
#define MINUS_3DB 0.707946
#define MINUS_6DB 0.501187
#define MINUS_8DB 0.398107

/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////

/* DSP type definitions */
/* DSP integer */
/* DSP unsigned integer */
/* native integer */
/* DSP fixed-point fractional */

typedef fract DSPfract; 
typedef long_accum DSPaccum; 
typedef int DSPint; 


#endif
