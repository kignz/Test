/**
********************************************************************************
* @file stmaTempoManager.c
* This is the code file for the tempo management process.
* This file code all the interface required to create and start tempo
* in the application. The  tempo are processed by the scheduler. This is a
* périodic process called by the scheduler on top long tempo set.
********************************************************************************
*/
/******************************************************************************\
* INCLUSIONS                                                                   *
\******************************************************************************/
// global types and constants declaration file
#include "Global.h"

// Definition of the tempos and tasks name
#include "ConfigDefine.h"

// System Scheduler interface
#include "skscKernelScheduler.h"

// Service de sommeil
#include "slpmLowPowerManager.h"

// Critical section management
#include "pmcoMicroController.h"

// Definition of the tempos and tasks tables
#include "ConfigTable.h"

// include of this code file
#include "stmaTempoManager.h"

/******************************************************************************\
* PRIVATE SYMBOLIC CONSTANTS and MACROS                                        *
\******************************************************************************/
/// @cond
// Number of ms tic for a minute
#define TIC_MINUTE                         (60 / TIMER_MANAGER_LONG_TICK_SECOND)

// Tempo stopped
#define TEMPO_STOPPED                      (0)
/// @endcond

/******************************************************************************\
* PRIVATE TYPES, STRUCTURES, UNIONS and ENUMS                                  *
\******************************************************************************/

/**
* Type for the structure of the long tempo info block.
* This structure define the type for the long tempo info block.
*/
typedef struct
{
    unsigned int accuracy : 6;  /**< accuracy of the long tempo */
    unsigned int type     : 1;  /**< type of the long tempo */
    unsigned int unused   : 1;  /**< field not used */
}t_infoLongTempo;

/******************************************************************************\
* PRIVATE MEMBER VARIABLES                                                     *
\******************************************************************************/

/************************\
* SHORT TEMPO MANAGEMENT *
\************************/
#ifdef COMP_SHORT_TEMPO_USED
// Prototype of the initialisation function for the short tempo manager
static void stmaShortTempoInit(void);
#ifndef POLYSPACE
// Prototype of the management process for the short tempo
static void stmaShortTempoProcess(void);
#endif
#endif

/**
* Variable of the Tempo Short Manager structure.
* This constant defined the tempo short manager used by the Scheduler.
* It is the init function and the process function of the tempo short manager.
*/
const t_skscTempoProcess stmaShortTempoManager =
{
#ifdef COMP_SHORT_TEMPO_USED
    stmaShortTempoInit,
    stmaShortTempoProcess,
#else
    FCT_NULL,
    FCT_NULL,
#endif // COMP_SHORT_TEMPO_USED
};

#ifdef COMP_SHORT_TEMPO_USED
/**
* Array of the remaining duration of short tempo.
*/
static t_stmaShortTempoDuration         mArrayShortTempo[NB_MAX_SHORT_TIMER];

/**
* Number of short tempo running.
*/
static t_stmaNbTempoRunning             mShortTempoRunning;
#endif // COMP_SHORT_TEMPO_USED


/***********************\
* LONG TEMPO MANAGEMENT *
\***********************/
#ifdef COMP_LONG_TEMPO_USED
// Prototype of the initialisation function for the short tempo manager
static void stmaLongTempoInit(void);
#ifndef POLYSPACE
// Prototype of the management process for the short tempo
static void stmaLongTempoProcess(void);
#endif
#endif
/**
* Variable of the Tempo Long Manager structure.
* This constant defined the tempo long manager used by the Scheduler.
* It is the init function and the process function of the tempo long manager.
*/
const t_skscTempoProcess stmaLongTempoManager =
{
#ifdef COMP_LONG_TEMPO_USED
    stmaLongTempoInit,
    stmaLongTempoProcess,
#else
    FCT_NULL,
    FCT_NULL,
#endif // COMP_LONG_TEMPO_USED
};

#ifdef COMP_LONG_TEMPO_USED
/**
* Array of the remaining duration of long tempo.
*/
static t_stmaLongTempoDuration          mArrayLongTempo[NB_MAX_LONG_TIMER];

/**
* Array of long tempo information.
*/
static t_infoLongTempo                  mLongTempoInfo[NB_MAX_LONG_TIMER];

/**
* Number of long tempo running
*/
static t_stmaNbTempoRunning             mLongTempoRunning;
#endif // COMP_LONG_TEMPO_USED

/******************************************************************************\
* PRIVATE FUNCTION PROTOTYPES                                                  *
\******************************************************************************/
// NOTHING HERE

/******************************************************************************\
* PRIVATE FUNCTION CODE                                                        *
\******************************************************************************/

/******************************************************************************\
* PUBLIC FUNCTION CODE                                                         *
\******************************************************************************/


/************************\
* SHORT TEMPO MANAGEMENT *
\************************/

#ifdef COMP_SHORT_TEMPO_USED

/**
* Function init of the short tempo process
* This function init the members variables for tempo process
* This function is call once in the main at the init section before launching
* the scheduler.
*/
static void stmaShortTempoInit(void)
{
    //***********************
    // declare local variable
    //***********************
    // local index
    uint8_t wIndex;


    // Init of tempo short state
    for (wIndex = 0; wIndex < NB_MAX_SHORT_TIMER; wIndex++)
    {
        // all tempo are stopped
        mArrayShortTempo[wIndex] = TEMPO_STOPPED;
    }

    // all tempo are stopped
    mShortTempoRunning = 0;
}


/**
* Function process of the short tempo.
* This function is called by the scheduler when the top synchro short tempo is
* set to process the short tempo : decount and call the associate function if
* tempo finished
* This function is used to declared the scheduler structure variable.
*/
#ifndef POLYSPACE
static
#endif
void stmaShortTempoProcess(void)
{
    //***********************
    // declare local variable
    //***********************
    // local index
    uint8_t wIndex;
    bool_t wTreatment;

    //The Tempo treatment is going to be performed
    slpmLowPowerEnable(LOWPOWER_SEMA_SHORT_TEMPO_RUNNING);

    // Test if at least one tempo is in duration
    if (mShortTempoRunning > 0)
    {
        // Compute the array of tempo to decrement the actived tempo
        for (wIndex = 0; wIndex < NB_MAX_SHORT_TIMER; wIndex++)
        {
            // No treatment
            wTreatment = FALSE;

            // Enter critical section
            skscEnterCriticalRegion();

                // Is this tempo running
                if (mArrayShortTempo[wIndex] > TEMPO_STOPPED)
                {
                    // decrement and test if tempo is finished
                    mArrayShortTempo[wIndex]--;
                    if (mArrayShortTempo[wIndex] == TEMPO_STOPPED)
                    {
                        // Enable treatment of the tempo
                        wTreatment = TRUE;

                        // Substract this tempo
                        mShortTempoRunning--;
                    }
                }

            // Exit critical section
            skscLeaveCriticalRegion();

            // If the tempo is finished
            if (wTreatment == TRUE)
            {
                // If the function exists
                if (ArrayShortTempoTreatment[wIndex] != PTR_NULL)
                {
                    // Call the associated function
                    (*ArrayShortTempoTreatment[wIndex])();
                }
                else
                {
                    MAC_ERROR(ERROR_SHORT_TEMPO_PTR);
                }
            }
        }
    }
}


/**
* Function to start a short tempo.
* @param [in] parTempoId : Id for the short tempo.
* @param [in] parDuration : value of the duration for this tempo.
* @code
*   stmaShortTempoStart(USER_SHORT_TIMER_1, TempoDurationMs / TIMER_MANAGER_SHORT_TICK);
* @endcode
*/
void stmaShortTempoStart(t_EnumShortTempoId parTempoId, t_stmaShortTempoDuration parDuration)
{
    if (parTempoId < NB_MAX_SHORT_TIMER)
    {
        // Enter critical section
        skscEnterCriticalRegion();

            // If the tempo is stopped
            if (mArrayShortTempo[parTempoId] == TEMPO_STOPPED)
            {
                // One more tempo running
                mShortTempoRunning++;
#ifdef COMP_LPM_COND_TEMPO_SHORT
                // Disable low power during short tempo running
                slpmLowPowerDisable(LOWPOWER_SEMA_SHORT_TEMPO_RUNNING);
#endif
            }

            // If duration is 0
            if (parDuration == 0)
            {
                // Init new duration for this tempo
                mArrayShortTempo[parTempoId] = 1;
            }
            else
            {
                // Init new duration for this tempo
                mArrayShortTempo[parTempoId] = parDuration;
            }

        // Exit critical section
        skscLeaveCriticalRegion();
    }
    // Error tempo unknown
    else
    {
        MAC_ERROR(ERROR_SHORT_TEMPO_NUM);
    }
}


/**
* Function to stop a short tempo.
* @param [in] parTempoId : Id for the short tempo.
* @code
*   stmaShortTempoStop(USER_SHORT_TIMER_1);
* @endcode
*/
void stmaShortTempoStop(t_EnumShortTempoId parTempoId)
{
    if (parTempoId < NB_MAX_SHORT_TIMER)
    {
        // Enter critical section
        skscEnterCriticalRegion();

        // If the tempo is running
        if (mArrayShortTempo[parTempoId] != TEMPO_STOPPED)
        {
            // Substract this tempo
            mShortTempoRunning--;

            // Stop the tempo
            mArrayShortTempo[parTempoId] = TEMPO_STOPPED;
        }

#ifdef COMP_LPM_COND_TEMPO_SHORT
        // If no more short tempo running
        if(mShortTempoRunning == 0)
        {
            // Enable low power
            slpmLowPowerEnable(LOWPOWER_SEMA_SHORT_TEMPO_RUNNING);
        }
#endif

        // Exit critical section
        skscLeaveCriticalRegion();
    }
    // Error tempo unknown
    else
    {
        MAC_ERROR(ERROR_SHORT_TEMPO_NUM);
    }
}


/**
* Function to read the state of a short tempo.
* This function return the state of the specified short tempo.
* @param [in] parTempoId : Id for the short tempo.
* @return : a t_stmaEnumTempoState current state of the tempo
*/
t_stmaEnumTempoState stmaShortTempoStateGet(t_EnumShortTempoId parTempoId)
{
    t_stmaEnumTempoState wReturn = stmaSTA_TEMPO_STOPPED;

    if (parTempoId < NB_MAX_SHORT_TIMER)
    {
        // If the tempo is running
        if (mArrayShortTempo[parTempoId] != TEMPO_STOPPED)
        {
            wReturn = stmaSTA_TEMPO_RUNNING;
        }
    }
    // Error tempo unknown
    else
    {
        MAC_ERROR(ERROR_SHORT_TEMPO_NUM);
    }

    // this tempo is off or unknown
    return (wReturn);
}


/**
* Function to read the duration of a short tempo.
* This function return the current remainig duration of the tempo.
* @param [in] parTempoId : Id for the short tempo.
* @return : a t_stmaShortTempoDuration remaining duration of the tempo.
*/
t_stmaShortTempoDuration stmaShortTempoCounterGet(t_EnumShortTempoId parTempoId)
{
    t_stmaShortTempoDuration wReturn = TEMPO_STOPPED;

    if (parTempoId < NB_MAX_SHORT_TIMER)
    {
        // return the remaining duration of this tempo
        wReturn = mArrayShortTempo[parTempoId];
    }
    else
    {
        // tempo unknown
        MAC_ERROR(ERROR_SHORT_TEMPO_NUM);
    }

    return(wReturn);
}


/**
* Function to get the number of short tempo running.
* @return : t_stmaNbTempoRunning number of short tempo running.
*/
t_stmaNbTempoRunning stmaShortTempoRunning(void)
{
    return (mShortTempoRunning);
}

#endif // COMP_SHORT_TEMPO_USED


/***********************\
* LONG TEMPO MANAGEMENT *
\***********************/

#ifdef COMP_LONG_TEMPO_USED

/**
* Function init of the long tempo process
* This function init the members variables for tempo process
* This function is call once in the main at the init section before launching
* the scheduler.
*/
static void stmaLongTempoInit(void)
{
    //***********************
    // declare local variable
    //***********************
    // local index
    uint8_t wIndex;


    // Init of tempo long state
    for (wIndex = 0; wIndex < NB_MAX_LONG_TIMER; wIndex++)
    {
        // all tempo are stopped
        mArrayLongTempo[wIndex] = TEMPO_STOPPED;
    }

    // all tempo are stopped
    mLongTempoRunning = 0;
}


/**
* Function process of the long tempo.
* This function is called by the scheduler when the top synchro long tempo is
* set to process the long tempo : decount and call the associate function if
* tempo finished
* This function is used to declared the scheduler structure variable.
*/
#ifndef POLYSPACE
static
#endif
void stmaLongTempoProcess(void)
{
    //***********************
    // declare local variable
    //***********************
    // local index
    uint8_t wIndex;
    bool_t wTreatment;

    //The Tempo treatment is going to be performed
    slpmLowPowerEnable(LOWPOWER_SEMA_LONG_TEMPO_RUNNING);

    // Test if at least one tempo is in duration
    if (mLongTempoRunning > 0)
    {
        // Compute the array of tempo to decrement the actived tempo
        for (wIndex = 0; wIndex < NB_MAX_LONG_TIMER; wIndex++ )
        {
            // Initialize the treatment
            wTreatment = FALSE;

            // Enter critical section
            skscEnterCriticalRegion();

                // Is this tempo running
                if (mArrayLongTempo[wIndex] > TEMPO_STOPPED)
                {
                    // Computing depend of the type of tempo
                    if((mLongTempoInfo[wIndex].type)== stmaTEMPO_MINUTE)
                    {
                        /* minute */
                        // Add accuracy
                        mLongTempoInfo[wIndex].accuracy++;

                        // If minute is elapsed
                        if (mLongTempoInfo[wIndex].accuracy >= TIC_MINUTE)
                        {
                           wTreatment = TRUE;
                        }
                    }
                    else
                    {
                        /* Décrémente la temporisation */
                        wTreatment = TRUE;
                    }

                    // Is this tempo running
                    if (wTreatment == TRUE)
                    {
                        // decrement and test if tempo is finished
                        mArrayLongTempo[wIndex]--;
                        if (mArrayLongTempo[wIndex] == TEMPO_STOPPED)
                        {
                            // Substract this tempo
                            mLongTempoRunning--;
                        }
                        else
                        {
                            // No treatment tempo is not elapsed
                            wTreatment = FALSE;
                        }

                        // Clear the accuracy counter
                        mLongTempoInfo[wIndex].accuracy = 0;
                    }
                }

            // Exit critical section
            skscLeaveCriticalRegion();

            // If the tempo is finished
            if (wTreatment == TRUE)
            {
                // If the function exists
                if (ArrayLongTempoTreatment[wIndex] != PTR_NULL)
                {
                  // Call the associated function
                  (*ArrayLongTempoTreatment[wIndex])();
                }
                else
                {
                    MAC_ERROR(ERROR_LONG_TEMPO_PTR);
                }
            }
        }
    }
}


/**
* Function to start a long tempo.
* @param [in] parTempoId : Id for the long tempo.
* @param [in] parDuration : value of the duration for this tempo.
* @param [in] parType : type of the tempo.
* @code
*   stmaLongTempoStart(USER_LONG_TIMER_1, TempoDurationSecond / TIMER_MANAGER_LONG_TICK_SECOND, stmaTEMPO_SECOND);
* @endcode
*/
void stmaLongTempoStart(t_EnumLongTempoId parTempoId, t_stmaLongTempoDuration parDuration, t_stmaEnumTypeLong parType)
{
    if (parTempoId < NB_MAX_LONG_TIMER)
    {
        // Enter critical section
        skscEnterCriticalRegion();

            // Init new duration for this tempo
            if (mArrayLongTempo[parTempoId] == TEMPO_STOPPED)
            {
                // One more tempo running
                mLongTempoRunning++;
#ifdef COMP_LPM_COND_TEMPO_LONG
                // Disable low power during long tempo running
                slpmLowPowerDisable(LOWPOWER_SEMA_LONG_TEMPO_RUNNING);
#endif
            }

            // Set the type of the tempo
            mLongTempoInfo[parTempoId].type = parType;

            // Clear the accuracy of the tempo
            mLongTempoInfo[parTempoId].accuracy = 0;

            // If duration is 0
            if (parDuration == 0)
            {
                // Init new duration for this tempo
                mArrayLongTempo[parTempoId] = 1;
            }
            else
            {
                // Init new duration for this tempo
                mArrayLongTempo[parTempoId] = parDuration;
            }

        // Exit critical section
        skscLeaveCriticalRegion();
    }
    // Error tempo unknown
    else
    {
        MAC_ERROR(ERROR_LONG_TEMPO_NUM);
    }
}


/**
* Function to stop a long tempo.
* @param [in] parTempoId : Id for the long tempo.
* @code
*   stmaTempoLongStop(USER_LONG_TIMER_1);
* @endcode
*/
void stmaLongTempoStop(t_EnumLongTempoId parTempoId)
{
    if (parTempoId < NB_MAX_LONG_TIMER)
    {
        // Enter critical section
        skscEnterCriticalRegion();

        // If the tempo is running
        if (mArrayLongTempo[parTempoId] != TEMPO_STOPPED)
        {
            // Substract this tempo
            mLongTempoRunning--;

            // Stop the tempo
            mArrayLongTempo[parTempoId] = TEMPO_STOPPED;
        }

#ifdef COMP_LPM_COND_TEMPO_LONG
        // If no more long tempo running
        if(mLongTempoRunning == 0)
        {
            // Enable low power
            slpmLowPowerEnable(LOWPOWER_SEMA_LONG_TEMPO_RUNNING);
        }
#endif

        // Exit critical section
        skscLeaveCriticalRegion();
    }
    // Error tempo unknown
    else
    {
        MAC_ERROR(ERROR_LONG_TEMPO_NUM);
    }
}


/**
* Function to read the state of a long tempo.
* This function return the state of the sepecified long tempo.
* @param [in] parTempoId : Id for the long tempo.
* @return : a t_stmaEnumTempoState current state of the tempo
*/
t_stmaEnumTempoState stmaLongTempoStateGet(t_EnumLongTempoId parTempoId)
{
    t_stmaEnumTempoState wReturn = stmaSTA_TEMPO_STOPPED;

    if (parTempoId < NB_MAX_LONG_TIMER)
    {
        // If the tempo is running
        if (mArrayLongTempo[parTempoId] != TEMPO_STOPPED)
        {
            wReturn = stmaSTA_TEMPO_RUNNING;
        }
    }
    // Error tempo unknown
    else
    {
        MAC_ERROR(ERROR_LONG_TEMPO_NUM);
    }

    // this tempo is off or unknown
    return(wReturn);
}


/**
* Function to read the duration of a long tempo.
* This function return the current remainig duration of the tempo.
* @param [in] parTempoId : Id for the long tempo.
* @return : a t_stmaLongTempoDuration remaining duration of the tempo.
*/
t_stmaLongTempoDuration stmaLongTempoCounterGet(t_EnumLongTempoId parTempoId)
{
    t_stmaLongTempoDuration wReturn = TEMPO_STOPPED;

    if (parTempoId < NB_MAX_LONG_TIMER)
    {
        // return the remaining duration of this tempo
        wReturn = mArrayLongTempo[parTempoId];
    }
    else
    {
        // tempo unknown
        MAC_ERROR(ERROR_LONG_TEMPO_NUM);
    }

    return(wReturn);
}


/**
* Function to get the number of long tempo running.
* @return : t_stmaNbTempoRunning number of long tempo running.
*/
t_stmaNbTempoRunning stmaLongTempoRunning(void)
{
    return (mLongTempoRunning);
}

#endif // COMP_LONG_TEMPO_USED

/******************************************************************************\
* END of FILE                                                                  *
\******************************************************************************/

