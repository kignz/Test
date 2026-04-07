/**
********************************************************************************
* @file slpmLowPowerManager.c
* This is the code of the Low Power Manager.
* The Low Power Manager is a semaphore which is used by the scheduler to check
* if entering in Low Power is enabled. This semaphore is reserved when a ressource
* in use not allowed entering in Low power Mode.
********************************************************************************
*/
/******************************************************************************\
* INCLUSIONS                                                                   *
\******************************************************************************/
// global types and constants declaration file
#include "Global.h"

// Definition of the tempos and tasks name
#include "ConfigDefine.h"

// Microcontroller interface
#include "pmcoMicroController.h"

// System Scheduler interface
#include "skscKernelScheduler.h"

// Short soft tempo process interface
#include "stmaTempoManager.h"

// include of this code file
#include "slpmLowPowerManager.h"

/******************************************************************************\
* PUBLIC SYMBOLIC CONSTANTS and MACROS                                         *
\******************************************************************************/
// NOTHING HERE

/******************************************************************************\
* PRIVATE TYPES, STRUCTURES, UNIONS ans ENUMS                                  *
\******************************************************************************/
// NOTHING HERE

/******************************************************************************\
* PRIVATE MEMBER VARIABLES                                                     *
\******************************************************************************/
/**
* This is the semaphore for Low Power Management.
* This variable when free enable entering in Low Power Mode.
*/
static uint16_t mSemaLowPower = LOWPOWER_SEMA_NONE;

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
* Function to release the semaphore for a specific ressource.
* This function enable entering in Low Power Mode for a specific ressource.
* @param [in] parSemaElt : specific ressource of the semaphore.
* @code
*   slpmLowPowerSemaEnable(slpmSEMA_LOWPOWER_UART);
* @endcode
*/
void slpmLowPowerEnable(t_EnumLowPowerSema parSemaElt)
{
    // release the semaphore for the specific ressource
    mSemaLowPower &= (parSemaElt ^ 0xFFFF);
}

/**
* Function to reserve the semaphore for a specific ressource.
* This function disable entering in Low Power Mode for a specific ressource.
* @param [in] parSemaElt : specific ressource of the semaphore.
* @code
*   slpmLowPowerSemaDisable(slpmSEMA_LOWPOWER_UART);
* @endcode
*/
void slpmLowPowerDisable(t_EnumLowPowerSema parSemaElt)
{
    // reserve the semaphore for the specific ressource
    mSemaLowPower |= parSemaElt;
}

/**
* Function to manage entering in Low Power Mode.
* This function manage entering in Low Power Mode. It test if all the conditions
* are ok to enter LPM : no event pending, semaphore LPM ressources µC is free,
* no tempo are running (compil options).
*/
void slpmLowPowerManager(void)
{
    // disable interrupt to test conditions entering LPPM
    skscEnterCriticalRegion();

    // if all the ressources of the product are released and no event pending
    if( skscEventPending() == FALSE )
    {
        // => Enable LowPower because no Event is pending
        slpmLowPowerEnable(LOWPOWER_SEMA_EVENT_PENDING);
#ifdef COMP_LPM_COND_TEMPO_SHORT
        // test if no short tempo are running
        if(stmaShortTempoRunning() == 0)
        {
            // => Enable LowPower because no short tempo is running
            slpmLowPowerEnable(LOWPOWER_SEMA_SHORT_TEMPO_RUNNING);
#endif
#ifdef COMP_LPM_COND_TEMPO_LONG
            // test if no long tempo are running
            if(stmaLongTempoRunning() == 0)
            {
                // => Enable LowPower because no long tempo is running
                slpmLowPowerEnable(LOWPOWER_SEMA_LONG_TEMPO_RUNNING);
#endif
                // Enable interrupt
                skscLeaveCriticalRegion();
                // If no semaphore is locked
                if(mSemaLowPower == LOWPOWER_SEMA_NONE)
                {


                    // Conditions are OK for LPM : manage enter in LPM
                    // Call µC function entering in LPM
                    while( mSemaLowPower == LOWPOWER_SEMA_NONE )
                    {
                        // Enter in low power
                        pmcoLowPowerEnter();
                    }
                    // Leave low power mode
                    pmcoLowPowerLeave();
                }

#ifdef COMP_LPM_COND_TEMPO_LONG
            }
            else
            {
                // Event waiting for treatment or µC ressources not allowed LPM.
                // Enable interrupt
                skscLeaveCriticalRegion();
            }
#endif
#ifdef COMP_LPM_COND_TEMPO_SHORT
        }
        else
        {
            // Event waiting for treatment or µC ressources not allowed LPM.
            // Enable interrupt
            skscLeaveCriticalRegion();
        }
#endif
    }
    else
    {
        // Event waiting for treatment or µC ressources not allowed LPM.
        // Enable interrupt
        skscLeaveCriticalRegion();
    }
}


/*******************************************************************************
* END of FILE
*******************************************************************************/

