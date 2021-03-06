/** @file hal/micro/cortexm3/micro.h
 * @brief Utility and convenience functions for EM35x microcontroller.
 * See @ref micro for documentation.
 *
 * <!-- Copyright 2008 by Ember Corporation.             All rights reserved.-->
 */

/** @addtogroup micro
 * See also hal/micro/cortexm3/micro.h for source code.
 *@{
 */

#ifndef __EM3XX_MICRO_H__
#define __EM3XX_MICRO_H__

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifndef __MICRO_H__
#error do not include this file directly - include micro/micro.h
#endif

// Micro specific serial defines
#define EM_NUM_SERIAL_PORTS 2
#define EMBER_SPI_MASTER 4
#define EMBER_SPI_SLAVE 5
#define EMBER_I2C 6

// Define the priority registers of system handlers and interrupts.
// This example shows how to save the current ADC interrupt priority and 
// then set it to 24:
//    int8u oldAdcPriority = INTERRUPT_PRIORITY_REGISTER(ADC);
//    INTERRUPT_PRIORITY_REGISTER(ADC) = 24;

// For Cortex-M3 faults and exceptions
#define HANDLER_PRIORITY_REGISTER(handler) \
  (*( ((int8u *)SCS_SHPR_7to4_ADDR) + handler##_VECTOR_INDEX - 4) )

// For EM3XX-specific interrupts
#define INTERRUPT_PRIORITY_REGISTER(interrupt) \
  (* ( ((int8u *)NVIC_IPR_3to0_ADDR) + interrupt##_VECTOR_INDEX - 16) )


// The reset types of the EM300 series have both a base type and an 
//  extended type.  The extended type is a 16-bit value which has the base
//  type in the upper 8-bits, and the extended classification in the 
//  lower 8-bits
// For backwards compatibility with other platforms, only the base type is 
//  returned by the halGetResetInfo() API.  For the full extended type, the
//  halGetExtendedResetInfo() API should be called.
 
#define RESET_BASE_TYPE(extendedType)   ((int8u)(((extendedType) >> 8) & 0xFF))
#define RESET_EXTENDED_FIELD(extendedType) ((int8u)(extendedType & 0xFF))
#define RESET_VALID_SIGNATURE           (0xF00F)
#define RESET_INVALID_SIGNATURE         (0xC33C)

// Define the base reset cause types
#define RESET_BASE_DEF(basename, value, string)  RESET_##basename = value,
#define RESET_EXT_DEF(basename, extname, extvalue, string)  /*nothing*/
enum {
  #include "reset-def.h"
  NUM_RESET_BASE_TYPES
};
#undef RESET_BASE_DEF
#undef RESET_EXT_DEF

// Define the extended reset cause types
#define RESET_EXT_VALUE(basename, extvalue)  \
  (((RESET_##basename)<<8) + extvalue)
#define RESET_BASE_DEF(basename, value, string)  /*nothing*/
#define RESET_EXT_DEF(basename, extname, extvalue, string)  \
  RESET_##basename##_##extname = RESET_EXT_VALUE(basename, extvalue),
enum {
  #include "reset-def.h"
};
#undef RESET_EXT_VALUE
#undef RESET_BASE_DEF
#undef RESET_EXT_DEF

#endif // DOXYGEN_SHOULD_SKIP_THIS

/** @name Vector Table Index Definitions
 * These are numerical definitions for vector table. Indices 0 through 15 are
 * Cortex-M3 standard exception vectors and indices 16 through 32 are EM3XX
 * specific interrupt vectors.
 *
 *@{
 */

/**
 * @brief A numerical definition for a vector.
 */
#define STACK_VECTOR_INDEX          0 // special case: stack pointer at reset
#define RESET_VECTOR_INDEX          1 
#define NMI_VECTOR_INDEX            2
#define HARD_FAULT_VECTOR_INDEX     3
#define MEMORY_FAULT_VECTOR_INDEX   4
#define BUS_FAULT_VECTOR_INDEX      5
#define USAGE_FAULT_VECTOR_INDEX    6
#define RESERVED07_VECTOR_INDEX     7
#define RESERVED08_VECTOR_INDEX     8
#define RESERVED09_VECTOR_INDEX     9
#define RESERVED10_VECTOR_INDEX     10
#define SVCALL_VECTOR_INDEX         11
#define DEBUG_MONITOR_VECTOR_INDEX  12
#define RESERVED13_VECTOR_INDEX     13
#define PENDSV_VECTOR_INDEX         14
#define SYSTICK_VECTOR_INDEX        15
#define TIMER1_VECTOR_INDEX         16
#define TIMER2_VECTOR_INDEX         17
#define MANAGEMENT_VECTOR_INDEX     18
#define BASEBAND_VECTOR_INDEX       19
#define SLEEP_TIMER_VECTOR_INDEX    20
#define SC1_VECTOR_INDEX            21
#define SC2_VECTOR_INDEX            22
#define SECURITY_VECTOR_INDEX       23
#define MAC_TIMER_VECTOR_INDEX      24
#define MAC_TX_VECTOR_INDEX         25
#define MAC_RX_VECTOR_INDEX         26
#define ADC_VECTOR_INDEX            27
#define IRQA_VECTOR_INDEX           28
#define IRQB_VECTOR_INDEX           29
#define IRQC_VECTOR_INDEX           30
#define IRQD_VECTOR_INDEX           31
#define DEBUG_VECTOR_INDEX          32
/**
 * @brief Number of vectors.
 */
#define VECTOR_TABLE_LENGTH         33
/** @}  Vector Table Index Definitions */

/**
 * @brief Records the specified reset cause then forces a reboot.
 */
void halInternalSysReset(int16u extendedCause);

/**
 * @brief Returns the Extended Reset Cause information
 *
 * @return A 16-bit code identifying the base and extended cause of the reset
 */
int16u halGetExtendedResetInfo(void);

/** @brief Calls ::halGetExtendedResetInfo() and supplies a string describing 
 *  the extended cause of the reset.  halGetResetString() should also be called
 *  to get the string for the base reset cause
 *
 * @appusage Useful for diagnostic printing of text just after program 
 * initialization.
 *
 * @return A pointer to a program space string.
 */
PGM_P halGetExtendedResetString(void);


//[[ ram vectors are not public
#ifndef DOXYGEN_SHOULD_SKIP_THIS

/** @brief Install the given address at the given vector number, overriding
 *  the default value found in Flash.
 *
 * The first RAM Vector to be Registered will automatically cause the RAM
 * Vector table to be filled from the Flash Vector table and the RAM table
 * will be enabled.
 *
 * @param vectorNumber  The vector number that is being changed.  There are
 * 31 valid vectors, 2-32, and the list can be found in nvic-config.h.
 * 
 * @param newVector  The 32-bit address to install into the vector.  This
 * is generally the address of the function that should be called when
 * the vector is triggered.
 *
 * @return The 32-bit address of the previous vector in the RAM Table that
 * is being replaced.
 */
int32u halRegisterRamVector(int8u vectorNumber, int32u newVector);


/** @brief Uninstall the given vector number, restoring the vector from the
 *  Flash vector table.
 *
 * The last RAM Vector to be UnRegistered will automatically cause the RAM
 * Vector table to be deactivated and Flash Vector table will regain control.
 *
 * @param vectorNumber  The vector number that is being changed.  There are
 * 31 valid vectors, 2-32, and the list can be found in nvic-config.h.
 * 
 * @return The 32-bit address of the previous vector in the RAM Table that
 * is being replaced.
 */
int32u halUnRegisterRamVector(int8u vectorNumber);

#endif // DOXYGEN_SHOULD_SKIP_THIS
//]]

#include "micro-common.h"

#endif //__EM3XX_MICRO_H__

/**@} // END micro group
 */

