/** 
 * @file: counters-cli.h
 *
 * Used for testing the counters library via a command line interface.
 * For documentation on the counters library see counters.h.
 *
 * Copyright 2007 by Ember Corporation. All rights reserved.                *80*
 */

void printCountersCommand(void);
void simplePrintCountersCommand(void);
void clearCountersCommand(void);

/** Args: destination id, clearCounters (boolean) */
void sendCountersRequestCommand(void);

/** Utility function for printing out the OTA counters response. */
void printCountersResponse(EmberMessageBuffer message);

/** Use this macro in the emberCommandTable for convenience. */
#define COUNTER_COMMANDS \
  { "cnt_print",     printCountersCommand,    "" },  \
  { "cnt_clear",     clearCountersCommand,    "" },

/** Use this macro in the emberCommandTable for convenience.
 *  For applications short on const space. 
 */
#define SIMPLE_COUNTER_COMMANDS                          \
  { "cnt_print",     simplePrintCountersCommand,  "" },  \
  { "cnt_clear",     clearCountersCommand,        "" },

/** Use this macro in the emberCommandTable for convenience.
 * This command requests counters over the air from a remote node.
 */
#define OTA_COUNTER_COMMANDS \
  { "cnt_req",       sendCountersRequestCommand,  "vu" },
