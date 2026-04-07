/**
********************************************************************************
* @file Main.c
* This is the main function where all begin.
* This is the product software.
********************************************************************************
*/
/******************************************************************************\
* INCLUSIONS                                                                   *
\******************************************************************************/
// global types and constants declaration file
#include "Global.h"
// Definition of the tempos and tasks name
#include "ConfigDefine.h"
// Definition of the tempos and tasks tables
#include "ConfigTable.h"
// System Scheduler interface
#include "skscKernelScheduler.h"
// Buffer manager
#include "sbmaBufferManager.h"
// Tempo manager
#include "stmaTempoManager.h"
// Service de sommeil
#include "slpmLowPowerManager.h"
// Microcontroller interface
#include "pmcoMicroController.h"

/******************************************************************************\
* PRIVATE SYMBOLIC CONSTANTS and MACROS                                        *
\******************************************************************************/
// NOTHING HERE

/******************************************************************************\
* PRIVATE TYPES, STRUCTURES, UNIONS and ENUMS                                  *
\******************************************************************************/
// NOTHING HERE

/******************************************************************************\
* PRIVATE MEMBER VARIABLES                                                     *
\******************************************************************************/
// NOTHING HERE

/******************************************************************************\
* PRIVATE FUNCTION PROTOTYPES                                                  *
\******************************************************************************/
// NOTHING HERE

/******************************************************************************\
* PRIVATE FUNCTION CODE                                                        *
\******************************************************************************/
// NOTHING HERE

/******************************************************************************\
* PUBLIC FUNCTION CODE                                                         *
\******************************************************************************/
/**
* This is the main function.
* All begin here. Create and init everything you need before starting the scheduler.
*/
#ifndef SWEET_SIMU_PC
void main(void)
{
#else
void SweetMain(void)
{
#endif

    // Disable IT during BOOT
    pmcoDisableInterrupt();

    // Config watchdog timer
    pmcoWatchdogManagement();

    // Init hardware
    pmcoInitHardware();
    
#ifdef COMP_BUFFER_MANAGER
    // Init buffer manager
    sbmaInit();
#endif // COMP_BUFFER_MANAGER

    // ***************************************************************** //
    // **** Init of the tasks and process for this product software **** //
    // ***************************************************************** //
    skscSchedulerInitAllTasks();

    // Interrupts ON
    pmcoEnableInterrupt();

    // Init Boot with IT ON
//    pmcoStartBoot();

#ifdef POLYSPACE
}

/**
 * This is the main task function for Polyspace multitasking analysis.
 * This function is an entry point for the analysis. skscSchedule can not be
 * called from the main function because it contains an infinite loop.
 */
void main_task(void)
{
#endif

    // ******************************** //
    // **** Start system scheduler **** //
    // ******************************** //
    skscSchedule();

}

/******************************************************************************\
* END of FILE                                                                  *
\******************************************************************************/

