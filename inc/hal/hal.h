
#ifndef __HAL_H__
#define __HAL_H__

///////////////////////////////////////////////////////////////////////////
// Keep micro and board first for specifics used by other headers
#include "micro.h"
#if !defined(STACK) && defined(BOARD_HEADER)
#include BOARD_HEADER
#endif

#include "adc.h"
#include "button.h"
#include "buzzer.h"
#include "crc.h"
#include "endian.h"
#include "led.h"
#include "random.h"
#include "serial.h"
#include "spi.h"
#include "system-timer.h"

///////////////////////////////////////////////////////////////////////////
// Host processors do not use the following modules, therefore the header files should be ignored.
#ifndef EZSP_HOST
  #include "bootloader-interface.h"
  #include "diagnostic.h"
  #include "token.h"
  //No public HAL code in release 4.0 uses the symbol timer, therefore it should not be in doxygen.
  #ifndef DOXYGEN_SHOULD_SKIP_THIS
    #include "symbol-timer.h"
  #endif // DOXYGEN_SHOULD_SKIP_THIS
#endif //EZSP_HOST

///////////////////////////////////////////////////////////////////////////
#endif //__HAL_H__

