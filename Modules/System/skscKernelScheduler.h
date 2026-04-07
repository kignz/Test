/**
********************************************************************************
* @file skscKernelScheduler.h
* This is the interface file for the System Kernel Scheduler.
* This file declare all the interface needed to use the System Kernel Scheduler.
* Just include this file to have access to the System Kernel Scheduler.
********************************************************************************
*/
// Avoid multiple inclusion
#ifndef __SKSC_SCHEDULER_H
#define __SKSC_SCHEDULER_H

/******************************************************************************\
* PUBLIC SYMBOLIC CONSTANTS and MACROS                                         *
\******************************************************************************/
/**
* Maximum number of events per task.
* This constant value define the maximum number of events per task : 16 default.
*/
#ifdef COMP_SCHEDULER_8_BITS
    #define skscMAX_NB_EVENT (8)
#else
    #define skscMAX_NB_EVENT (16)
#endif
/**
* Define the generic events which has to be used in the different tasks
*/
/// @cond
    #define skscEVENT1      (0x0001)
    #define skscEVENT2      (0x0002)
    #define skscEVENT3      (0x0004)
    #define skscEVENT4      (0x0008)
    #define skscEVENT5      (0x0010)
    #define skscEVENT6      (0x0020)
    #define skscEVENT7      (0x0040)
    #define skscEVENT8      (0x0080)
#ifndef COMP_SCHEDULER_8_BITS
    #define skscEVENT9      (0x0100)
    #define skscEVENT10     (0x0200)
    #define skscEVENT11     (0x0400)
    #define skscEVENT12     (0x0800)
    #define skscEVENT13     (0x1000)
    #define skscEVENT14     (0x2000)
    #define skscEVENT15     (0x4000)
    #define skscEVENT16     (0x8000)
#endif
/// @endcond

/******************************************************************************\
* PUBLIC TYPES, STRUCTURES, UNIONS and ENUMS                                   *
\******************************************************************************/
/**
* Type for system events : 1 event per bit.
* - Each task could have its own 16 different events not the same for all the
* tasks.
* - A task should include the header of the other tasks to be able to post
* events to these tasks.
*
* A PRIORITY IS GIVEN TO THE BIT EVENTS. The priority order of the events for a
* task is the way these bits will be tested : from LSB to MSB (b0 to b15).
*/
#ifdef COMP_SCHEDULER_8_BITS
    typedef uint8_t t_skscTaskEvent;
#else
    typedef uint16_t t_skscTaskEvent;
#endif

/**
* Type for the state of the task process.
* The task may have different states for which a same event could have a
* different function link to
*/
typedef uint8_t t_skscProcessState;

/**
* Type for the primitives of the task process.
* Pointeur to the function called by the process when the appropriate event is
* set.
*/
typedef void (* t_skscPrimitivePointer)(void);

/**
* Enum for the different task number "Task Slot".
* In the scheduler, each task has a number define here. The number max of task
* is limited to 10.
*/
typedef enum
{
    skscTASK_0,   /**< Task n°0_A : highest priority */
    skscTASK_1,     /**< Task n°1  : medium priority */
    skscTASK_2,     /**< Task n°2  : low priority */
    skscTASK_3,     /**< Task n°3  : low priority */
    skscTASK_4,     /**< Task n°4  : low priority */
    skscTASK_5,     /**< Task n°5  : low priority */
    skscTASK_6,     /**< Task n°6  : low priority */
    skscTASK_7,     /**< Task n°7  : low priority */
    skscTASK_8,     /**< Task n°8  : low priority */
    skscTASK_9,     /**< Task n°9  : low priority */
//    skscMAX_NB_TASK /**< Nb Task Max : number of task */
} t_skscTaskSlot;

/**
* Type for the structure of the task process.
* This structure should be redefined for each task created depending on the
* number of the task state for optimisation.
*/
typedef struct
{
    const uint8_t NbMaxEvent;               /**< Nb max event used by this task process <= skscMAX_NB_EVENT */
    const t_skscProcessState NbMaxState;    /**< Nb max state of the task process */
    t_skscProcessState * State;             /**< pointeur to the state of the task process */
    void (*Initialisation) (void);          /**< init function of the task process */
#ifdef POLYSPACE
    t_skscPrimitivePointer  StateEventProcess[16*256]; /**< table of primitives (state,event) of the task process */
#else
    t_skscPrimitivePointer  *StateEventProcess; /**< table of primitives (state,event) of the task process */
#endif
}t_skscTaskProcess;

/**
* Type for the structure of the task.
* This is the structure of a task : a task number and a task process.
*/
typedef struct
{
    const t_skscTaskSlot TaskSlot;    /**< slot number of the task */
    t_skscTaskProcess * TaskProcess;  /**< pointeur to the process of the task */
}t_skscTask;

/**
* Type for the structure of a Scheduler Tempo Process.
* This is the structure of a Scheduler Tempo Process : an init function and a process function.
*/
typedef struct
{
    void (*Initialisation) (void);  /**< init function of the Scheduler Tempo Process */
    void (*Process) (void);         /**< process function of the Scheduler Tempo Process */
}t_skscTempoProcess;

/**
* Enum for the different Top for the sheduler tempo synchronisation.
* Each Top indicates the scheduler to compute the associated tempo.
*/
typedef enum
{
    skscTOP_SCHED_TEMPO_SHORT = 0x01,   /**< Top for short tempo */
    skscTOP_SCHED_TEMPO_LONG  = 0x02    /**< Top for long tempo */
}t_skscTopScheduler;

/**
* Type for the structure of the scheduler process.
* The scheduler compute the differents tasks and process defined in its process structure.
* Only one scheduler has to be defined.
*/
typedef struct
{
    void (*ProcessWatchDog)(void);      /**< process for the WatchDog manager */
#ifdef TRACE_UART
    void (*ProcessTrace)(void);         /**< process for the µC uart trace manager */
#endif // TRACE_UART
    void (*ProcessSleep)(void);         /**< process for the µC sleep low power manager */
#ifdef COMP_SHORT_TEMPO_USED
    const t_skscTempoProcess* ProcessTempoShort;    /**< process for short tempo management */
#endif
#ifdef COMP_LONG_TEMPO_USED    // used this define if you want long tempo managment
    const t_skscTempoProcess* ProcessTempoLong; /**< process for long tempo managment */
#endif
#ifdef COMP_TASK_LEVEL_0_USED
    const t_skscTask* TaskLevel0; /**< task level 0 : high priority */
#endif // COMP_TASK_LEVEL_0_USED
#ifdef COMP_TASK_LEVEL_1_USED
    const t_skscTask* TaskLevel1; /**< task level 1 : medium priority */
#endif
#ifdef COMP_TASK_LEVEL_2_USED
    const t_skscTask* TaskLevel2[MAX_NB_TASK_LEVEL_2];    /**< array of task level 2 : low priority */
#endif
}t_skscScheduler;

/******************************************************************************\
* PUBLIC VARIABLES                                                             *
\******************************************************************************/
/**
* Reference to the scheduler tasks table.
* The scheduler task table is a const struct of t_skscScheduler type which declare
* all the task of the application.
* this table has to be visible to the system. It is create in the main.
* @remarks
* ATTENTION !!
* @n This variable has to be created and declared once in the main as a
* @n const t_skscScheduler skscSchedulerTableTask
* @n The different tasks created for the specific application are referenced in this variable
* @n and the tempo short or/and long also
* @code
* // here is an exemple of the Scheduler task table creation in the main
* // this specific application has :
*   // - csrsSweetRadioTask for task level 0
*   // - aappProductAppliTask for task level 1
*   // - aihmProductIHMTask for task level 2
*   // - cipsTcpIpStack for task level 2
*   // - sstpProcessTempoShort for short tempo process
*   // - sltpProcessTempoLong for long tempo process
*   // - pwdmWatchDogManager for the watch dog manager process
*   // - plpmLowPowerManager for the low power manager process
* const t_skscScheduler skscSchedulerTableTask =
* {
*   pwdmWatchDogManager,    // process for the WatchDog manager
*   plpmLowPowerManager,    // process for the µC sleep low power manager
*   sstpProcessTempoShort,  // process for short tempo management
*   sltpProcessTempoLong,   // process for long tempo managment
*   &csrsSweetRadioTask,    // task level 0
*   &aappProductAppliTask,  // task level 1
*   {
*     &aihmProductIHMTask,  // task level 2
*     &cipsTcpIpStack,      // task level 2
*     NULL,                  // no task level 2
*     NULL,                  // no task level 2
*     NULL,                  // no task level 2
*     NULL,                  // no task level 2
*     NULL,                  // no task level 2
*     NULL,                  // no task level 2
*   },
*};
* @endcode
*/
extern const t_skscScheduler skscSchedulerTableTask;

/******************************************************************************\
* PUBLIC FUNCTION PROTOTYPES                                                   *
\******************************************************************************/
/* Prototype of the Scheduler Init all Tasks function */
/* This function is called after all the tasks have been created to initialise them
 * during the Boot */
void skscSchedulerInitAllTasks(void);

/* Prototype of the scheduler start function */
/* This function is called after all the tasks have been created and initialised
 * during the Boot */
void skscSchedule(void);

/* Prototype of function to set the synchronise top for the short tempo */
void skscSetTopShortTempo(void);
/* Prototype of function to set the synchronise top for the long tempo */
void skscSetTopLongTempo(void);

/* Prototype of the set event function to a task */
/* Use this function to post the parEvent Event to the parTaskId Task */
void skscEventSend(t_skscTaskEvent parEvent, t_skscTaskSlot parTaskId);
/* Prototype of the kill event function to a task */
/* Used this function to reset a posted event to a task */
void skscEventKill(t_skscTaskEvent parEvent, t_skscTaskSlot parTaskId);
/* Prototype of the function to test if at least one event is set and waiting for treatment */
bool_t skscEventPending(void);

/* Critical section manager */
/* Prototype of function enter in critical region */
void skscEnterCriticalRegion(void);
/* Prototype of function exit from critical region */
void skscLeaveCriticalRegion(void);

/* Prototype of function for No Treatment */
void skscFunctNull(void);

#endif /*__SKSC_SCHEDULER_H*/
/******************************************************************************\
* END of FILE                                                                  *
\******************************************************************************/

