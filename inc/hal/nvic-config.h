/** @file hal/micro/cortexm3/nvic-config.h
 *
 * <!-- Copyright 2008 by Ember Corporation. All rights reserved.        *80*-->
 */
 
/** @addtogroup nvic_config
 * Nested Vectored Interrupt Controller configuration header.
 *
 * This header defines the functions called by all of the NVIC exceptions/
 * interrupts.  The following are the nine peripheral ISRs available for
 * modification.  To use one of these ISRs, it must be instantiated
 * somewhere in the HAL.  Each ISR may only be instantiated once.  It is
 * not necessary to instantiate all or any of these ISRs (a stub will
 * be automatically generated if an ISR is not instantiated).
 *
 * - \code void halTimer1Isr(void); \endcode
 * - \code void halTimer2Isr(void); \endcode
 * - \code void halSc1Isr(void);    \endcode
 * - \code void halSc1Isr(void);    \endcode
 * - \code void halAdcIsr(void);    \endcode
 * - \code void halIrqAIsr(void);   \endcode
 * - \code void halIrqBIsr(void);   \endcode
 * - \code void halIrqCIsr(void);   \endcode
 * - \code void halIrqDIsr(void);   \endcode
 *
 * @note This file should \b not be modified.
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS

// \b NOTE NOTE NOTE NOTE NOTE NOTE - The physical layout of this file, that
// means the white space, is CRITICAL!  Since this file is \#include'ed by
// the assembly file isr-stubs.s79, the white space in this file translates
// into the white space in that file and the assembler has strict requirements.
// Specifically, there must be white space *before* each "SEGMENT()" and there
// must be an *empty line* between each "EXCEPTION()" and "SEGMENT()".
//
// \b NOTE NOTE NOTE - The order of the EXCEPTIONS in this file is critical
// since it translates to the order they are placed into the vector table in
// cstartup.
//
// The purpose of this file is to consolidate NVIC configuration, this
// includes basic exception handler (ISR) function definitions, into a single
// place where it is easily tracked and changeable.
//
// There are 5 bits of priority yielding 32 levels of priority.  A lower
// priority number yields the highest priority.  As such, 0 is the highest
// priority while 31 is the lowest priority.
//
// NOTE: Do NOT exceed a priority level of 31 as the value will be truncated
// and treated as a much higher priority.  Only the upper 5 of the 8 bit
// priority fields are effective - bits [7:3].  As such, when the priorities
// defined here are used they are bit shifted left by 3 to move them into the
// upper 5 bits.

//The 'PRIGROUP' field is 5 bits large and indicates how many of the most
//significant bits within the 8-bit priority field for a given exception
//number are allocated to preemptive priority grouping versus non-preemptive
//subpriority.  This value sets the "binary" point in the priority field
//where the separation occurs.
//The table below shows for each PRIGROUP value (the PRIGROUP value is the
//number on the far left) the priority groups in ()
//with the subpriorities in [].  When defining an exception's priority, use
//one of the numbers in [].
//0=7:1=(0)[0],(1)[1],(2)[2],(3)[3],(4)[4],(5)[5],(6)[6],(7)[7]
//      (8)[8],(9)[9],(10)[10],(11)[11],(12)[12],(13)[13],(14)[14],(15)[15],
//      (16)[16],(17)[17],(18)[18],(19)[19],(20)[20],(21)[21],(22)[22],(23)[23],
//      (24)[24],(25)[25],(26)[26],(27)[27],(28)[28],(29)[29],(30)[30],(31)[31]
//1=6:2=(0)[0],(1)[1],(2)[2],(3)[3],(4)[4],(5)[5],(6)[6],(7)[7]
//      (8)[8],(9)[9],(10)[10],(11)[11],(12)[12],(13)[13],(14)[14],(15)[15],
//      (16)[16],(17)[17],(18)[18],(19)[19],(20)[20],(21)[21],(22)[22],(23)[23],
//      (24)[24],(25)[25],(26)[26],(27)[27],(28)[28],(29)[29],(30)[30],(31)[31]
//2=5:3=(0)[0],(1)[1],(2)[2],(3)[3],(4)[4],(5)[5],(6)[6],(7)[7],
//      (8)[8],(9)[9],(10)[10],(11)[11],(12)[12],(13)[13],(14)[14],(15)[15],
//      (16)[16],(17)[17],(18)[18],(19)[19],(20)[20],(21)[21],(22)[22],(23)[23],
//      (24)[24],(25)[25],(26)[26],(27)[27],(28)[28],(29)[29],(30)[30],(31)[31]
//3=4:4=(0)[0-1],(1)[2-3],(2)[4-5],(3)[6-7],
//      (4)[8-9],(5)[10-11],(6)[12-13],(7)[14-15],
//      (8)[16-17],(9)[18-19],(10)[20-21],(11)[22-23],
//      (12)[24-25],(13)[26-27],(14)[28-29],(15)[30-31]
//4=3:5=(0)[0-3],(1)[4-7],(2)[8-11],(3)[12-15],
//      (4)[16-19],(5)[20-23],(6)[24-27],(7)[28-31]
//5=2:6=(0)[0-7],(1)[8-15],(2)[16-23],(3)[24-31]
//6=1:7=(0)[0-15],(1)[16-31]
//7=0:8=(0)[0-31]
//Provide 8 levels of preemptive priority and 4 of "tie-breaker" priority.
//Priorities from highest to lowest are as follows:
// 0  - faults
// 4  - management interrupt (dma ram protection violation)
// 8  - pendsv (deep sleep)
// 12 - interrupts disabled
// 16 - normal interrupts
// 20 - not used
// 24 - not used
// 28 - debug
#define PRIGROUP_POSITION 4

// Priority level used by DISABLE_INTERRUPTS() and INTERRUPTS_OFF()
// Must be lower priority than pendsv 
#define INTERRUPTS_DISABLED_PRIORITY  (12 << 3)


//Exceptions with fixed priorities cannot be changed by software.  Simply make
//them 0 since they are high priorities anyways.
#define FIXED 0
//Reserved exceptions are not instatiated in the hardware.  Therefore
//exception priorities don't exist so just default them to lowest level.
#define NONE  31

#ifndef SEGMENT
  #define SEGMENT()
#endif
#ifndef SEGMENT2
  #define SEGMENT2()
#endif
#ifndef PERM_EXCEPTION
  #define PERM_EXCEPTION(vectorNumber, functionName, priority) \
    EXCEPTION(vectorNumber, functionName, priority)
#endif

    // SEGMENT()
    //   **Place holder required by isr-stubs.s79 to define __CODE__**
    // SEGMENT2()
    //   **Place holder required by isr-stubs.s79 to define __THUMB__**
    // EXCEPTION(vectorNumber, functionName, priorityLevel)
    //   vectorNumber = exception number defined by hardware (not used anywhere)
    //   functionName = name of the function that the exception should trigger
    //   priorityLevel = priority level of the function
    // PERM_EXCEPTION
    //   is used to define an exception that should not be intercepted by the
    //   interrupt debugging logic, or that not should have a weak stub defined.  
    //   Otherwise the definition is the same as EXCEPTION

//INTENTIONALLY INDENTED AND SPACED APART! Keep it that way!  See comment above!
    SEGMENT()
    SEGMENT2()
    PERM_EXCEPTION(01, halEntryPoint,   FIXED)  //Reset Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(02, halNmiIsr,            FIXED)  //NMI Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(03, halHardFaultIsr,      FIXED)  //Hard Fault Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(04, halMemoryFaultIsr,    0)      //Memory Fault Handler 
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(05, halBusFaultIsr,       0)      //Bus Fault Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(06, halUsageFaultIsr,     0)      //Usage Fault Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(07, halReserved07Isr,     NONE)   //Reserved
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(08, halReserved08Isr,     NONE)   //Reserved
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(09, halReserved09Isr,     NONE)   //Reserved
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(10, halReserved10Isr,     NONE)   //Reserved
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(11, halSvCallIsr,         16)     //SVCall Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(12, halDebugMonitorIsr,   16)     //Debug Monitor Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(13, halReserved13Isr,     NONE)   //Reserved
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(14, halPendSvIsr,         8)      //PendSV Handler
    
    SEGMENT()
    SEGMENT2()
    //EXCEPTION(15, halSysTickIsr,        16)     //SysTick Handler
      EXCEPTION(15, SysTickIsr,        16)     //SysTick Handler
    
    //The following handlers map to "External Interrupts 16 and above"
    //In the NVIC Interrupt registers, this corresponds to bits 16:0 with bit 0
    //being TIMER1 (exception 16) and bit 15 being IRQD (exception 15)
    SEGMENT()
    SEGMENT2()
    EXCEPTION(16, halTimer1Isr,         16)     //Timer 1 Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(17, halTimer2Isr,         16)     //Timer 2 Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(18, halManagementIsr,     8)      //Management Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(19, halBaseBandIsr,       16)     //BaseBand Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(20, halSleepTimerIsr,     16)     //Sleep Timer Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(21, halSc1Isr,            16)      // 16)     //SC1 Handler
	
    SEGMENT()
    SEGMENT2()
    EXCEPTION(22, halSc2Isr,            16)     //SC2 Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(23, halSecurityIsr,       16)     //Security Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(24, halStackMacTimerIsr,  16)     //MAC Timer Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(25, emRadioTransmitIsr,   16)     //MAC TX Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(26, emRadioReceiveIsr,    16)     //MAC RX Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(27, halAdcIsr,            16)     //ADC Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(28, halIrqAIsr,           16)     //GPIO IRQA Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(29, halIrqBIsr,           16)     //GPIO IRQB Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(30, halIrqCIsr,           16)     //GPIO IRQC Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(31, halIrqDIsr,           16)     //GPIO IRQD Handler
    
    SEGMENT()
    SEGMENT2()
    EXCEPTION(32, halDebugIsr,          28)     //Debug Handler

#undef SEGMENT
#undef SEGMENT2
#undef PERM_EXCEPTION

#endif //DOXYGEN_SHOULD_SKIP_THIS

