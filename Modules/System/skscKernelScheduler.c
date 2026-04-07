/**
********************************************************************************
* @file skscKernelScheduler.c
* This file is the code for the System Kernel Scheduler.
* Here are coded all the functions used by the System Kernel Scheduler.
********************************************************************************
*/
/******************************************************************************\
* INCLUSIONS                                                                   *
\******************************************************************************/
// global types and constants declaration file
#include "Global.h"

// Definition of the tempos and tasks name
#include "ConfigDefine.h"

// Microcontroller definition
#include "pmcoMicroController.h"

// include of this code file
#include "slpmLowPowerManager.h"

// include of this code file
#include "skscKernelScheduler.h"

/******************************************************************************\
* PRIVATE SYMBOLIC CONSTANTS and MACROS                                        *
\******************************************************************************/
#define HIGH_PRIORITY_MAILBOX   ( 1 )


/** Medium priority tasks number*/
#define MEDIUM_PRIORITY_MAILBOX ( 1 )

/** Offset of the low priority tasks mailboxes */
#define LOW_PRIORITY_OFFSET     ( HIGH_PRIORITY_MAILBOX + MEDIUM_PRIORITY_MAILBOX )

/******************************************************************************\
* PRIVATE TYPES, STRUCTURES, UNIONS and ENUMS                                  *
\******************************************************************************/
// NOTHING HERE

/******************************************************************************\
* PRIVATE MEMBER VARIABLES                                                     *
\******************************************************************************/
/**
* Pseudo mailbox to store all the event for all the task.
* Mailbox of the event for the task : vector of Nb task max elements.
* Each element is a mailbox at 1 dimension which memorize all the event for one task.
*/
static t_skscTaskEvent mEventMailBox[LOW_PRIORITY_OFFSET + MAX_NB_TASK_LEVEL_2];

/**
* Top scheduler which memorize the top set for tempo management.
* This variable memorize the top set to the scheduler for tempo management.
*/
static t_skscTopScheduler mTopSchedulerMemo;

/**
* Critical section counter
*/
static uint8_t mNumberOfRegion;

/**
* Initial state of interrupt_enable (at the beginning of the first critical section)
*/
static t_EnableState mInitialInterruptState;

/******************************************************************************\
* PRIVATE FUNCTION PROTOTYPES                                                  *
\******************************************************************************/
// Prototype of the scheduler initialization function
static void SchedulerInit(void);

// Prototype of the main treatment function of the task
// This function process the posted event for the task
static void TaskProcess(const t_skscTask *parTask);

/* Prototype of function test if critical region counter ok */
static void skscTestCriticalRegion(void);

/******************************************************************************\
* PRIVATE FUNCTION CODE                                                        *
\******************************************************************************/
/**
* Function to init the scheduler.
* This function is used to init the scheduler of the task.
*/
static void SchedulerInit(void)
{
    // Initialisation du top du scheduleur : no top set
    mTopSchedulerMemo = (t_skscTopScheduler)(0x00);
    // Init members variable for critical region managment
    // counter enter / exit critical section
    mNumberOfRegion        = 0;
    // initial interrupt state flag
    mInitialInterruptState = pmcoGetInterruptState();
}

/**
* Function that process the set event of the task.
* This function process the task : it get the most priority event set to the task,
* clear it and process the treatment function associated depending of the task state
* as described in the StateEventProcess table of the task.
* This function is only called by the scheduler.
* The priority for the event is from lsb (high prio) to msb (low prio)
* @param [in] parTask : pointer to the task to process.
*/
static void TaskProcess(const t_skscTask *parTask)
{
    //***********************
    // declare local variable
    //***********************
    uint8_t                 wEventIndex;                                        // local index
    t_skscPrimitivePointer  wEventTreatfunction;                                // function associate
    t_skscTaskEvent         wEventMask = (t_skscTaskEvent)(0x0001);             // event mask
    t_skscProcessState      wTaskState = *(parTask->TaskProcess->State) ;       // Current task state
    t_skscProcessState      wMaxStates = parTask->TaskProcess->NbMaxState ;     // Max state for current task
    uint8_t                 wMaxEvents = parTask->TaskProcess->NbMaxEvent ;     // Max events used by the current task
    bool_t ExitLoop = FALSE;		// Flag to stop process the task, check if event set

    // NBl : Correction pour bug compilo Cypress
    const t_skscTask                *wTestTaskPtr;
    const t_skscTaskProcess         *wTestTaskProcessPtr;
    const t_skscPrimitivePointer    *wTestStateEventProcessPtr;
    t_skscProcessState              *wPtrState;

    wTestTaskProcessPtr = (const t_skscTaskProcess*)parTask->TaskProcess;
    wPtrState = wTestTaskProcessPtr->State;
    wTaskState = *wPtrState;
    // NBl : Fin correction pour bug compilo Cypress

/*
* @verbatim
Algo of the task process :

In the processed task mailbox, test if an event is set. This test is done starting
from the lsb event which is the highest priority to the msb event which is the lowest priority
For the first set event matched
    Clear the event in the mailbox for the task
    Get the associated treatment function of the event depending of the task state
    If the function is defined, call it
Stop here and give back the hand to the scheduler. Only one event process at a time.

@endverbatim
*/
    // Perform some checks on the task : is the current state correct for this task
    if (wTaskState >= wMaxStates)
    {
        // error : state of this task out of range
        MAC_ERROR(ERROR_EVENT_OR_STATE_UNDEFINED);
    }
    else
    {
        // test the event for the task from the high priority (lsb) to the low priority (msb)
        // limited only to the number of state used by the task
        for(wEventIndex = 0; (wEventIndex < wMaxEvents) && (ExitLoop == FALSE); wEventIndex++)
        {
            // test if event set
            if((mEventMailBox[parTask->TaskSlot] & wEventMask) != 0)
            {
                // this event is set : we process it
                // first clear this event
                mEventMailBox[parTask->TaskSlot] &= (uint16_t) ~wEventMask;
                // get the associated treatment function depending of the task state
                //wEventTreatfunction = parTask->TaskProcess->StateEventProcess[(wTaskState * wMaxEvents) + wEventIndex];

                // NBl : Correction bug compilo Cypress afin d eviter que "wEventTreatfunction = parTask->TaskProcess->StateEventProcess[(wTaskState * wMaxEvents) + wEventIndex]" ne soit pas nulle;
                wTestTaskPtr = (const t_skscTask*)parTask;
                wTestTaskProcessPtr = (const t_skscTaskProcess*)wTestTaskPtr->TaskProcess;
                wTestStateEventProcessPtr = (const t_skscPrimitivePointer*)&wTestTaskProcessPtr->StateEventProcess;
                // get the associated treatment function depending of the task state
                wEventTreatfunction = wTestStateEventProcessPtr[(wTaskState * wMaxEvents) + wEventIndex];
                // NBl : Fin correction bug compilo Cypress


                // if function defined, call it
                if(wEventTreatfunction != PTR_NULL)
                {
                    // here is the real treatment of the set event
#ifdef POLYSPACE
                    wEventTreatfunction();
#else
                    (*wEventTreatfunction)();
#endif //#ifdef POLYSPACE
                }
                else
                {
                    // error : null treatment of the event
                    MAC_ERROR(ERROR_EVENT_TREAT_PTR_NULL);
                }
                // give back the hand to the scheduler for next task
                // even if an other event is still set for the current task
                ExitLoop = TRUE ;

                // test if any not used event are set
                if((wEventIndex >= wMaxEvents) && (mEventMailBox[parTask->TaskSlot] != 0))
                {
                    // error : event not used set !
                    MAC_ERROR(ERROR_EVENT_OUT_OF_RANGE);
                }
            }
            // if this event is not set, test the next event
            wEventMask <<= 1 ;
        }
    }
}

/******************************************************************************\
* PUBLIC FUNCTION CODE                                                         *
\******************************************************************************/
/**
* Function to post an event to a task.
* This function is called to send one or more event to a task.
* @param [in] parEvent : Event to post.
* @param [in] parTaskId : Id Number of the task to which the event is sent.
* @return none.
* @code
*   skscEventSend(xyyyEVENT_TO_POST_1 | xyyyEVENT_TO_POST_2, zwwwIdTaskSelected);
* @endcode
*/
void skscEventSend(t_skscTaskEvent parEvent, t_skscTaskSlot parTaskId)
{
#ifdef COMP_ERROR_MANAGEMENT_USED
    #ifdef COMP_DEBUG_EVENT_CHECK
    uint8_t NbMaxEventTask;

    // check if this event is used by this task
    // how many event used by this task
#ifdef COMP_TASK_LEVEL_0_USED        
    // check if task level 0
    if(parTaskId == skscTASK_0)
    {
        // task level 0_B
        NbMaxEventTask = skscSchedulerTableTask.TaskLevel0->TaskProcess->NbMaxEvent;
    }
#endif // COMP_TASK_LEVEL_0_USED        
#ifdef COMP_TASK_LEVEL_1_USED        
    // check if task level 1
    else if(parTaskId == skscTASK_1)
    {
        // task level 1
        NbMaxEventTask = skscSchedulerTableTask.TaskLevel1->TaskProcess->NbMaxEvent;
    }
#endif // COMP_TASK_LEVEL_1_USED        
#ifdef COMP_TASK_LEVEL_2_USED
    // check if task level 2
    else
    {
        // task level 2
        NbMaxEventTask = skscSchedulerTableTask.TaskLevel2[parTaskId-LOW_PRIORITY_OFFSET]->TaskProcess->NbMaxEvent;
    }
#endif
    // is this event used by this task
    if(parEvent > (0x0001 << (NbMaxEventTask-1)))
    {
        // error : event not used by this task
        MAC_ERROR(ERROR_EVENT_OUT_OF_RANGE);
    }
    #endif
#endif
    if (parTaskId < (LOW_PRIORITY_OFFSET + MAX_NB_TASK_LEVEL_2))
    {
        // set the event of the specific task in the mailbox
        mEventMailBox[parTaskId] |= parEvent;
        // Disable low power when event pending
        slpmLowPowerDisable(LOWPOWER_SEMA_EVENT_PENDING);
    }
}

/**
* Function to kill an event already posted to a task.
* This function is called to reset one or more event to a task.
* @param [in] parEvent : Event to kill.
* @param [in] parTaskId : Id Number of the task to which the event is killed.
* @return none.
* @code
*   skscEventKill(xyyyEVENT_TO_POST_1, zwwwIdTaskSelected);
* @endcode
*/
void skscEventKill(t_skscTaskEvent parEvent, t_skscTaskSlot parTaskId)
{
#ifdef COMP_ERROR_MANAGEMENT_USED
    #ifdef COMP_DEBUG_EVENT_CHECK
    uint8_t NbMaxEventTask;

    // check if this event is used by this task
    // how many event used by this task
#ifdef COMP_TASK_LEVEL_0_USED
    // check if task level 0
    if(parTaskId == skscTASK_0)
    {
        // task level 0
        NbMaxEventTask = skscSchedulerTableTask.TaskLevel0->TaskProcess->NbMaxEvent;
    }
#endif // COMP_TASK_LEVEL_0_USED
#ifdef COMP_TASK_LEVEL_1_USED
    // check if task level 1
    else if(parTaskId == skscTASK_1)
    {
        // task level 1
        NbMaxEventTask = skscSchedulerTableTask.TaskLevel1->TaskProcess->NbMaxEvent;
    }
#endif // COMP_TASK_LEVEL_2_USED
#ifdef COMP_TASK_LEVEL_2_USED
    // check if task level 2
    else
    {
        // task level 2
        NbMaxEventTask = skscSchedulerTableTask.TaskLevel2[parTaskId-LOW_PRIORITY_OFFSET]->TaskProcess->NbMaxEvent;
    }
#endif
    // is this event used by this task
    if(parEvent > (0x0001 << (NbMaxEventTask-1)))
    {
        // error : event not used by this task
        MAC_ERROR(ERROR_EVENT_OUT_OF_RANGE);
    }
    #endif
#endif
    if (parTaskId < (LOW_PRIORITY_OFFSET + MAX_NB_TASK_LEVEL_2))
    {
        // reset the event of the specific task in the mailbox
        mEventMailBox[parTaskId] &= (uint16_t) ~parEvent;
    }
}

/**
* Function to test if at least one event is set for a task.
* This function is called to test if there is at least one event in the system
* mailbox waiting for treatment.
* @return TRUE : event waiting, FALSE : mailbox empty, no event set.
* @code
*   if(skscEventPending() == TRUE)
*       // event waiting for treatment, no sleep
*   else
*       // no event set, sleep ok
* @endcode
*/
bool_t skscEventPending(void)
{
    //***********************
    // declare local variable
    //***********************
    uint8_t wTaskIndex;
    bool_t  wReturnCode = FALSE ;
    bool_t  wExitLoop    = FALSE;

    // test for all the task used if event set
    for(wTaskIndex = 0; (wTaskIndex < (LOW_PRIORITY_OFFSET + MAX_NB_TASK_LEVEL_2)) && (wExitLoop == FALSE); wTaskIndex++)
    {
        // test if the mailbox event for the task is empty
        if (mEventMailBox[wTaskIndex] != 0)
        {
            // mailbox not empty, event set and waiting for treatment
            wReturnCode = TRUE;
            wExitLoop   = TRUE;
        }
    }
    return wReturnCode;
}

/**
* Function to init all the Tasks of the Scheduler during BOOT.
* This function is called after all the tasks have been created to initialise them
* during the Boot. This init is done without IT enable !
*/
void skscSchedulerInitAllTasks(void)
{
    void (*wFctPtr)(void);
    const t_skscTaskProcess* wTaskProcess;

    //***********************
    // declare local variable
    //***********************
#ifdef COMP_TASK_LEVEL_2_USED
    uint8_t wTaskIndex;     // local index
#endif

#ifdef COMP_SHORT_TEMPO_USED
    // init short tempo process
    skscSchedulerTableTask.ProcessTempoShort->Initialisation();
#endif

#ifdef COMP_LONG_TEMPO_USED    // used this define if you want long tempo managment
    // init long tempo process
    skscSchedulerTableTask.ProcessTempoLong->Initialisation();
#endif
#ifdef COMP_TASK_LEVEL_0_USED
      // init task level 0
    if(skscSchedulerTableTask.TaskLevel0 != PTR_NULL)
    {
        wTaskProcess = (const t_skscTaskProcess*)skscSchedulerTableTask.TaskLevel0->TaskProcess;
        wFctPtr = wTaskProcess->Initialisation;
        // call the task init function
        (*wFctPtr)();
    }
#endif // COMP_TASK_LEVEL_0_USED
#ifdef COMP_TASK_LEVEL_1_USED
    // init task level 1
    if(skscSchedulerTableTask.TaskLevel1 != PTR_NULL)
    {
        wTaskProcess = (const t_skscTaskProcess*)skscSchedulerTableTask.TaskLevel1->TaskProcess;
        wFctPtr = wTaskProcess->Initialisation;
        // call the task init function
        (*wFctPtr)();
    }
#endif // COMP_TASK_LEVEL_1_USED
#ifdef COMP_TASK_LEVEL_2_USED
    // for all the tasks level 2
    for(wTaskIndex = 0; wTaskIndex < MAX_NB_TASK_LEVEL_2; wTaskIndex++)
    {
        // init task level 2 i
        if(skscSchedulerTableTask.TaskLevel2[wTaskIndex] != FCT_NULL)
        {
            wTaskProcess = (const t_skscTaskProcess*)skscSchedulerTableTask.TaskLevel2[wTaskIndex]->TaskProcess;
            wFctPtr = wTaskProcess->Initialisation;
            // Task Level 2 i define not NULL : init task
            // call the task init function
            (*wFctPtr)();
        }
    }
#endif
}

/**
* Function to schedule all the tasks in the system.
* This function is called once in the end of the main after the init of all the
* task and process.
* @code
*   // in the main after the init of all the task and process
*   skscSchedule();
*   // nothing after : while(1) in the skscSchedule function
* @endcode
*/
void skscSchedule(void)
{
    //***********************
    // declare local variable
    //***********************
#ifdef COMP_TASK_LEVEL_2_USED
    uint8_t wTaskIndex;  // local index
#endif

/**
* @verbatim
Scheduler algorithm :

Init Scheduler (process and task already created and initiated in the boot)
DO LOOP WITHOUT ENDING
    if mailbox not empty for task level 0 (event set waiting for treatment)
        call process task level 0
    if mailbox not empty for task level 1 (event set waiting for treatment)
        call process task level 1
    else
        for each task i level 2
            if mailbox not empty for task i level 2 (event set waiting for treatment)
                call process task i level 2
    if top synchro scheduler short tempo set
        clear top short tempo under critical region
        call process short tempo
    if top synchro scheduler long tempo set
        clear top long tempo under critical region
        call process long tempo
    manage watchdog
    manage low power mode
@endverbatim
*/
    // Init Scheduler, reset Top synchro tempo
    SchedulerInit();

    // Loop without end : do while event waiting for treatment or sleep
    while(TRUE)
    {
        // Check that there is no critical region problem
        skscTestCriticalRegion();
#ifdef COMP_TASK_LEVEL_0_USED        
        // Is mailbox not empty for task level 0 : Event set waiting for treatment
        if(mEventMailBox[skscTASK_0] != 0)
        {
            // if task function is defined
            if ((skscSchedulerTableTask.TaskLevel0)!= PTR_NULL)
            {
                // process task level 0 to treat Event set
                TaskProcess(skscSchedulerTableTask.TaskLevel0);
            }
            else
            {
                // this is not possible : an event set for a NULL task
                // error sheduler
                MAC_ERROR(ERROR_EVENT_SET_TASK_NULL);
            }
        }
#endif // COMP_TASK_LEVEL_0_USED        
#ifdef COMP_TASK_LEVEL_1_USED        
        // Is mailbox not empty for task level 1 : Event set waiting for treatment
        if(mEventMailBox[skscTASK_1] != 0)
        {
            // if task function is defined
            if ((skscSchedulerTableTask.TaskLevel1)!= PTR_NULL)
            {
                // process task level 1 to treat Event set
                TaskProcess(skscSchedulerTableTask.TaskLevel1);
            }
            else
            {
                // this is not possible : an event set for a NULL task
                // error sheduler
                MAC_ERROR(ERROR_EVENT_SET_TASK_NULL);
            }
        }
#endif // COMP_TASK_LEVEL_1_USED        
#ifdef COMP_TASK_LEVEL_2_USED
        else
        {
            /* if no event set for task level 1, treat tasks level 2 */
            // for all the tasks level 2
            for(wTaskIndex = 0; wTaskIndex < MAX_NB_TASK_LEVEL_2; wTaskIndex++)
            {
                // Is mailbox not empty for each task i level 2 : Event set waiting for treatment
                // +2 is due to the task with priority 0 and 1 (take 2 indexes)
                if(mEventMailBox[wTaskIndex + LOW_PRIORITY_OFFSET] != 0)
                {
                    // if task function is defined
                    if ((skscSchedulerTableTask.TaskLevel2[wTaskIndex])!=FCT_NULL)
                    {
                        // Process pending event
                        TaskProcess(skscSchedulerTableTask.TaskLevel2[wTaskIndex]);
                    }
                    else
                    {
                        // this is not possible : an event set for a NULL task
                        // error sheduler
                        MAC_ERROR(ERROR_EVENT_SET_TASK_NULL);
                    }
                }
            }
        }
#endif

#ifdef COMP_SHORT_TEMPO_USED
        // manage short tempo
        // Is top synchro short tempo set ?
        if((mTopSchedulerMemo & skscTOP_SCHED_TEMPO_SHORT) == skscTOP_SCHED_TEMPO_SHORT)
        {
            // top synchro short tempo set : process tempo short
            // clear top synchro short tempo before treatment : critical section
            skscEnterCriticalRegion();
            mTopSchedulerMemo &= ~(skscTOP_SCHED_TEMPO_SHORT);
            skscLeaveCriticalRegion();
            // process short tempo
            skscSchedulerTableTask.ProcessTempoShort->Process();
        }
#endif // COMP_SHORT_TEMPO_USED

// compile option if long tempo used
#ifdef COMP_LONG_TEMPO_USED
        // Is top synchro long tempo set ?
        if((mTopSchedulerMemo & skscTOP_SCHED_TEMPO_LONG) == skscTOP_SCHED_TEMPO_LONG)
        {
            // top synchro long tempo set : process tempo long
            // clear top synchro long tempo before treatment : critical section
            skscEnterCriticalRegion();
            mTopSchedulerMemo &= ~(skscTOP_SCHED_TEMPO_LONG);
            skscLeaveCriticalRegion();
            // process short tempo
            skscSchedulerTableTask.ProcessTempoLong->Process();
        }
#endif // COMP_LONG_TEMPO_USED

        // Call the watchdog manager
        skscSchedulerTableTask.ProcessWatchDog();

#ifdef TRACE_UART
        // Call the trace manager
        skscSchedulerTableTask.ProcessTrace();
#endif //TRACE_UART

        // Call the low power manager
        skscSchedulerTableTask.ProcessSleep();
    }
}

/**
* Function to synchronise the scheduler for short tempo management.
* This function is called to set the top sheduler synchro for short tempo process.
* @code
*   // in the IT service where timer for short tempo top elapse
*   skscSetTopShortTempo();
* @endcode
*/
void skscSetTopShortTempo(void)
{
    // Set the flag for the top scheduler synchro for short tempo
    mTopSchedulerMemo |= skscTOP_SCHED_TEMPO_SHORT;
    // force the scheduler to perform a tempo treatment
    slpmLowPowerDisable(LOWPOWER_SEMA_SHORT_TEMPO_RUNNING);
}

/**
* Function to synchronise the scheduler for long tempo managment.
* This function is called to set the top sheduler synchro for long tempo process.
* @code
*   // in the IT service where timer for long tempo top elapse
*   skscSetTopLongTempo();
* @endcode
*/
void skscSetTopLongTempo(void)
{
    // Set the flag for the top shecduler synchro for long tempo
    mTopSchedulerMemo |= skscTOP_SCHED_TEMPO_LONG;
    // force the scheduler to perform a tempo treatment
    slpmLowPowerDisable(LOWPOWER_SEMA_LONG_TEMPO_RUNNING);
}

/**
* Routine to call to protect a region of code from interrupt
*/
void skscEnterCriticalRegion(void)
{
    t_EnableState wEnableState;

    // get the current interrupt state
    wEnableState = pmcoGetInterruptState();

    // disbale interrupt
    pmcoDisableInterrupt();

    // if there is no current region
    if (mNumberOfRegion==0)
    {
        // store the current interrupt state
        mInitialInterruptState = wEnableState;
    }
    mNumberOfRegion++;
}

/**
* Routine to stop the protection a region of code from interrupt
*/
void skscLeaveCriticalRegion(void)
{
    if (mNumberOfRegion>0)
    {
        mNumberOfRegion--;
        // if we are leaving the last region
        if (mNumberOfRegion==0)
        {
            // if the IT was activated before the first critical section
            if (mInitialInterruptState==ENABLED)
            {
                // restore it
                pmcoEnableInterrupt();
            }
        }
    }
    else
    {
        MAC_ERROR(ERROR_CRITICAL_REGION);
    }
}

/**
* Test the state of the critical region manager (has to be called in the main !)
*/
static void skscTestCriticalRegion(void)
{
    if (mNumberOfRegion!=0)
    {
        MAC_ERROR(ERROR_CRITICAL_REGION);
    }
}

/**
* Function for no treatment.
* This function do nothing : treatment null for evt and debug.
*/
void skscFunctNull(void)
{
    // error if debug
    MAC_ERROR(ERROR_EVENT_TREAT_NONE);
}

/******************************************************************************\
* END of FILE                                                                  *
\******************************************************************************/
