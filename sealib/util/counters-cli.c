// File: counters-cli.c
//
// Used for testing the counters library via a command line interface.
// For documentation on the counters library see counters.h.
//
// Copyright 2007 by Ember Corporation. All rights reserved.                *80*

#include PLATFORM_HEADER

#include "stack/include/ember.h"
#include "hal/hal.h" 
#include "app/util/serial/serial.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/common/common.h"
#include "app/util/counters/counters.h"
#include "app/util/counters/counters-ota.h"

void clearCountersCommand(void)
{
  emberClearCounters();
}

PGM_NO_CONST PGM_P titleStrings[] = {
  EMBER_COUNTER_STRINGS
};

PGM_NO_CONST PGM_P unknownCounter = "???";

void printCountersCommand(void)
{ 
  int8u i=0;
  while ( i < EMBER_COUNTER_TYPE_COUNT ) {
    int16u data = emberCounters[i];
    emberSerialPrintfLine(serialPort, "%d) %p: %d", 
                          i,
                          (titleStrings[i] == NULL
                           ? unknownCounter
                           : titleStrings[i]),
                          data);
    emberSerialWaitSend(serialPort);
    i++;
  }
}

// For applications short on const space.
void simplePrintCountersCommand(void)
{
  int8u i;
  for ( i = 0; i < EMBER_COUNTER_TYPE_COUNT; i++ ) {
    emberSerialPrintfLine(serialPort, "%d: %d",
                          i,
                          emberCounters[i]);
    emberSerialWaitSend(serialPort);
  }
}

void sendCountersRequestCommand(void)
{
  emberSendCountersRequest(emberUnsignedCommandArgument(0),
                           emberUnsignedCommandArgument(1));
}

void printCountersResponse(EmberMessageBuffer message)
{
  int8u i;
  int8u length = emberMessageBufferLength(message);
  for (i = 0; i < length; i += 3) {
    emberSerialPrintfLine(serialPort, "%d: %d",
                          emberGetLinkedBuffersByte(message, i),
                          emberGetLinkedBuffersLowHighInt16u(message, i + 1));
    emberSerialWaitSend(serialPort);
  }
}
