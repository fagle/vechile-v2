
#include "config.h"
#include "ember.h"
#include "hal.h"
#include "serial.h"

// This state is kept track of so the IRQ ISRs knows when to notify buttonIsrs.
int8u button0State, button1State;

void halInternalInitButton(void)
{
  //start from a fresh state just in case
  BUTTON0_INTCFG = 0;                   //disable BUTTON0 triggering
  BUTTON1_INTCFG = 0;                   //disable BUTTON1 triggering
  INT_CFGCLR = BUTTON0_INT_EN_BIT; //clear BUTTON0 top level int enable
  INT_CFGCLR = BUTTON1_INT_EN_BIT; //clear BUTTON1 top level int enable
  INT_GPIOFLAG = BUTTON0_FLAG_BIT;      //clear stale BUTTON0 interrupt
  INT_GPIOFLAG = BUTTON1_FLAG_BIT;      //clear stale BUTTON1 interrupt
  INT_MISS = BUTTON0_MISS_BIT;          //clear stale missed BUTTON0 interrupt
  INT_MISS = BUTTON1_MISS_BIT;          //clear stale missed BUTTON1 interrupt
  
  // Configure BUTTON0
  BUTTON0_SEL();                              //point IRQ at the desired pin
  BUTTON0_INTCFG  = (0 << GPIO_INTFILT_BIT);  //no filter
  BUTTON0_INTCFG |= (3 << GPIO_INTMOD_BIT);   //3 = both edges
  button0State = halButtonPinState(BUTTON0);
  
  // Configure BUTTON1
  BUTTON1_SEL();                              //point IRQ at the desired pin
  BUTTON1_INTCFG  = (0 << GPIO_INTFILT_BIT);  //no filter
  BUTTON1_INTCFG |= (3 << GPIO_INTMOD_BIT);   //3 = both edges
  button1State = halButtonPinState(BUTTON1);
  
  // Enable Interrupts
  INT_CFGSET = BUTTON0_INT_EN_BIT;  //set top level interrupt enable
  INT_CFGSET = BUTTON1_INT_EN_BIT;  //set top level interrupt enable
}

int8u halButtonState(int8u button)
{
  // Note: this returns the "soft" state rather than reading the port
  //  so it jives with the interrupts and their callbacks
  switch(button) {
    case BUTTON0:
      return button0State;
    case BUTTON1:
      return button1State;
    default:
      return BUTTON_RELEASED;
  }
}

int8u halButtonPinState(int8u button)
{
  // Note: this returns the current state of the button's pin.  It may not
  // jive with the interrupts and their callbacks, but it is useful for
  // checking the state of the button on startup.
  switch(button) {
    case BUTTON0:
      return (BUTTON0_IN & BIT(BUTTON0&7)) ? BUTTON_RELEASED : BUTTON_PRESSED;
    case BUTTON1:
      return (BUTTON1_IN & BIT(BUTTON1&7)) ? BUTTON_RELEASED : BUTTON_PRESSED;
    default:
      return BUTTON_RELEASED;
  }
}


//DEBOUNCE operation is based upon the theory that when multiple reads in a row
//return the same value, we have passed any debounce created by the mechanical
//action of a button.  The define "DEBOUNCE" says how many reads in a row
//should return the same value.  The value '5', below, is the recommended value
//since this should require the signal to have stabalized for approximately
//100us which should be much longer than any debounce action.
//Uncomment the following line to enable software debounce operation:
//#define DEBOUNCE 5

//Typically, software debounce is disabled by defaulting to a value of '0'
//which will cause the preprocessor to strip out the debounce code and save
//flash space.
#ifndef DEBOUNCE
#define DEBOUNCE 0
#endif//DEBOUNCE


void BUTTON0_ISR(void)
{
  int8u buttonStateNow;
  #if (DEBOUNCE > 0)
    int8u buttonStatePrev;
    int32u debounce;
  #endif //(DEBOUNCE > 0)

  //clear int before read to avoid potential of missing interrupt
  INT_MISS = BUTTON0_MISS_BIT;     //clear missed BUTTON0 interrupt flag
  INT_GPIOFLAG = BUTTON0_FLAG_BIT; //clear top level BUTTON0 interrupt flag
  
  buttonStateNow = halButtonPinState(BUTTON0);
  #if (DEBOUNCE > 0)
    //read button until get "DEBOUNCE" number of consistent readings
    for( debounce = 0;
         debounce < DEBOUNCE;
         debounce = (buttonStateNow == buttonStatePrev) ? debounce + 1 : 0 ) {
      buttonStatePrev = buttonStateNow;
      //clear int before read to avoid potential of missing interrupt
      INT_GPIOFLAG = BUTTON0_FLAG_BIT; //clear top level BUTTON0 interrupt flag
      buttonStateNow = halButtonPinState(BUTTON0);
    }
  #endif //(DEBOUNCE > 0)
  if(button0State != buttonStateNow) { //state changed, notify app
    button0State = buttonStateNow;
    halButtonIsr(BUTTON0,  button0State);
  } else {  //state unchanged, then notify app of a double-transition
    button0State = !button0State; //invert temporarily for calling Isr
    halButtonIsr(BUTTON0, button0State);
    button0State = !button0State; //and put it back to current state
    halButtonIsr(BUTTON0, button0State);
  }
  
}

void BUTTON1_ISR(void)
{
  int8u buttonStateNow;
  #if (DEBOUNCE > 0)
    int8u buttonStatePrev;
    int32u debounce;
  #endif //(DEBOUNCE > 0)
  
  //clear int before read to avoid potential of missing interrupt
  INT_MISS = BUTTON1_MISS_BIT;     //clear missed BUTTON1 interrupt flag
  INT_GPIOFLAG = BUTTON1_FLAG_BIT; //clear top level BUTTON1 interrupt flag
  
  buttonStateNow = halButtonPinState(BUTTON1);
  #if (DEBOUNCE > 0)
    //read button until get "DEBOUNCE" number of consistent readings
    for( debounce = 0;
         debounce < DEBOUNCE;
         debounce = (buttonStateNow == buttonStatePrev) ? debounce + 1 : 0 ) {
      buttonStatePrev = buttonStateNow;
      //clear int before read to avoid potential of missing interrupt
      INT_GPIOFLAG = BUTTON1_FLAG_BIT; //clear top level BUTTON1 interrupt flag
      buttonStateNow = halButtonPinState(BUTTON1);
    }
  #endif //(DEBOUNCE > 0)
  if(button1State != buttonStateNow) { //state changed, notify app
    button1State = buttonStateNow;
    halButtonIsr(BUTTON1,  button1State);
  } else {  //state unchanged, then notify app of a double-transition
    button1State = !button1State; //invert temporarily for calling Isr
    halButtonIsr(BUTTON1, button1State);
    button1State = !button1State; //and put it back to current state
    halButtonIsr(BUTTON1, button1State);
  }
}

