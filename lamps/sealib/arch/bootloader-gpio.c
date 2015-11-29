#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "hal.h"
#include "bootloader-gpio.h"

// Function Name: bootloadForceActivation
// Description:   Decides whether to continue launching the bootloader or vector
//                to the application instead. This decision is usually based on
//                some sort of GPIO logic.
//                (ie. a hardware trigger).
// NOTE!!!        This function is called extremely early in the boot process
//                prior to the chip being fully configured and before memory 
//                is initialized, so limited functionality is available.
//                If this function returns FALSE any state altered should
//                be returned to its reset state
// Parameters:    none
// Returns:       FALSE (0) if bootloader should not be launched.
//                (This will try to execute the application if possible.)
//                TRUE (1) if bootloader should be launched.
//
boolean bootloadForceActivation( void ) 
{ 
  #ifdef USE_BUTTON_RECOVERY
    int32u i;
    boolean pressed;
    // this provides an example of an alternative recovery mode activation
    //  method by utilizing one of the breakout board buttons

    // Since the button may have decoupling caps, they may not be charged
    //  after a power-on and could give a false positive result.  To avoid
    //  this issue, drive the output as an output for a short time to charge
    //  them up as quickly as possible
    halGpioConfig(PORTA_PIN(7),GPIOCFG_OUT);
    GPIO_PASET = PA7;
    for(i=0; i<100; i++)
      __no_operation();
    
    // Reconfigure as an input with pullup to read the button state
    halGpioConfig(PORTA_PIN(7),GPIOCFG_IN_PUD);
    // (IO was already set to enable the pullup above)
    // We have to delay again here so that if the button is depressed the
    //  cap has time to discharge again after being charged up 
    //  by the above delay
    for(i=0; i<500; i++)
      __no_operation();
    pressed = (GPIO_PAIN & BIT(&)) ? FALSE : TRUE;
    //restore IO to its reset state
    halGpioConfig(PORTA_PIN(7),GPIOCFG_IN);
  
    return pressed;
  #else
    return FALSE;; 
  #endif
}


//
// Function Name: bootloadGpioInit
// Description:   This function selects the chip's GPIO configuration. These 
//                settings are the minimum base configuration needed to 
//                support the bootloader functionality. These settings can 
//                be modified for custom applications as long as the base 
//                settings are preserved.
// Parameters:    none
// Returns:       none
//
void bootloadGpioInit(void) 
{
  halInternalEnableWatchDog();

  #ifndef NO_LED
    halGpioConfig(BOARD_ACTIVITY_LED, GPIOCFG_OUT);
    halClearLed(BOARD_ACTIVITY_LED);
    halGpioConfig(BOARD_HEARTBEAT_LED, GPIOCFG_OUT);
    halSetLed(BOARD_HEARTBEAT_LED);  
    CONFIGURE_EXTERNAL_REGULATOR_ENABLE();  
  #endif
}


// bootloadStateIndicator
//
// Called by the bootloader state macros (in bootloader-gpio.h). Used to blink
// the LED's or otherwise signal bootloader activity.
//
// Current settings: 
// Enabled: Turn on yellow activity LED on bootloader up and successful bootload
//          Turn off yellow activity LED on bootload failure
// Disabled: Blink red heartbeat LED during idle polling.
void bootloadStateIndicator(enum blState_e state) 
{
  // sample state indication using LEDs
  #ifndef NO_LED
    static int16u pollCntr = 0;
  
    switch(state) {
      case BL_ST_UP:                      // bootloader up
        halSetLed(BOARD_ACTIVITY_LED);
        break;

      case BL_ST_DOWN:                    // bootloader going down
        break;

      case BL_ST_POLLING_LOOP:            // Polling for serial or radio input in 
                                          // standalone bootloader.
        if(0 == pollCntr--) {
          halToggleLed(BOARD_HEARTBEAT_LED);
          pollCntr = 10000;
        }
        break;

      case BL_ST_DOWNLOAD_LOOP:           // processing download image
        halClearLed(BOARD_HEARTBEAT_LED);
        halToggleLed(BOARD_ACTIVITY_LED);
        break;
      
      case BL_ST_DOWNLOAD_SUCCESS: 
        halSetLed(BOARD_ACTIVITY_LED);
        break;
      
      case BL_ST_DOWNLOAD_FAILURE:
        halClearLed(BOARD_ACTIVITY_LED);
        break;

      default:
        break;
    }
  #endif
}
