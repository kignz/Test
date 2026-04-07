/**
********************************************************************************
* @file stmaTempoManager.h
* This is the interface file for the long tempo management process.
* This file reference all the interface required to create and start long tempo
* in the application. The long tempo are processed by the scheduler.
********************************************************************************
*/
// Avoid multiple inclusion
#ifndef __STMA_TEMPO_MANAGER_H
#define __STMA_TEMPO_MANAGER_H

/* global types and constants declaration file */
#include "Global.h"
/******************************************************************************\
* PUBLIC SYMBOLIC CONSTANTS and MACROS                                         *
\******************************************************************************/
// NOTHING HERE

/******************************************************************************\
* PUBLIC TYPES, STRUCTURES, UNIONS and ENUMS                                   *
\******************************************************************************/
/**
* Type of the short tempo duration.
*/
typedef uint16_t t_stmaShortTempoDuration;

/**
* Type of the long tempo duration.
*/
typedef uint16_t t_stmaLongTempoDuration;

/**
* Type of the number of tempo running
*/
typedef uint8_t t_stmaNbTempoRunning;

/**
* Type of enum for the long tempo state.
* This list all the state of the long tempo.
*/
typedef enum
{
    stmaSTA_TEMPO_RUNNING,    /**< tempo on duration */
    stmaSTA_TEMPO_STOPPED     /**< tempo stopped */
} t_stmaEnumTempoState;

/**
* Type of different long tempo.
*/
typedef enum
{
    stmaTEMPO_SECOND,
    stmaTEMPO_MINUTE
}t_stmaEnumTypeLong;

/******************************************************************************\
* PUBLIC FUNCTION PROTOTYPES                                                   *
\******************************************************************************/
// NOTHING HERE

/******************************************************************************\
* SHORT TEMPO MANAGEMENT                                                       *
\******************************************************************************/
#ifdef POLYSPACE
void stmaShortTempoProcess(void);
#endif 

// Prototype of the short tempo start function
void stmaShortTempoStart(t_EnumShortTempoId parTempoId, t_stmaShortTempoDuration parDuration);

// Prototype of the stop short tempo function
void stmaShortTempoStop(t_EnumShortTempoId parTempoId);

// Prototype of the read state short tempo function
t_stmaEnumTempoState stmaShortTempoStateGet(t_EnumShortTempoId parTempoId);

// Prototype of the read counter short tempo function
t_stmaShortTempoDuration stmaShortTempoCounterGet(t_EnumShortTempoId parTempoId);

// Prototype of the short tempo running function
t_stmaNbTempoRunning stmaShortTempoRunning(void);

/******************************************************************************\
* LONG TEMPO MANAGEMENT                                                        *
\******************************************************************************/
#ifdef POLYSPACE
void stmaLongTempoProcess(void);
#endif 

// Prototype of the short tempo start function
void stmaLongTempoStart(t_EnumLongTempoId parTempoId, t_stmaLongTempoDuration parDuration, t_stmaEnumTypeLong parType);

// Prototype of the stop short tempo function
void stmaLongTempoStop(t_EnumLongTempoId parTempoId);

// Prototype of the read state short tempo function
t_stmaEnumTempoState stmaLongTempoStateGet(t_EnumLongTempoId parTempoId);

// Prototype of the read counter short tempo function
t_stmaLongTempoDuration stmaLongTempoCounterGet(t_EnumLongTempoId parTempoId);

// Prototype of the short tempo running function
t_stmaNbTempoRunning stmaLongTempoRunning(void);

/******************************************************************************\
* PUBLIC VARIABLES                                                             *
\******************************************************************************/
/***************************************\
* SHORT TIMER PROCEDURES                *
\***************************************/
/**
* Array of the treatment function associated to the tempo.
* This constant array defined statically for each tempo the function to call as
* soon as the tempo is finished. The associated function should be of
* t_skscPrimitivePointer type.
*
* @remarks
* Do not forget for each new tempo defined in the declaration enum to add in
* this array the associated treatment function.
*/
extern const t_skscPrimitivePointer ArrayShortTempoTreatment[NB_MAX_SHORT_TIMER];

/**
* Variable of the Tempo Short Manager structure.
* This constant defined the tempo short manager used by the Scheduler.
* It is the init function and the process function of the tempo short manager.
*/
extern const t_skscTempoProcess stmaShortTempoManager;

/***************************************\
* LONG TIMER PROCEDURES                 *
\***************************************/
/**
* Array of the treatment function associated to the tempo.
* This constant array defined staticaly for each tempo the function to call as
* soon as the tempo is finished. The associated function should be of
* t_skscPrimitivePointer type.
*
* @remarks
* Do not forget for each new tempo defined in the declaration enum to add in
* this array the associated treatment function.
*/
extern const t_skscPrimitivePointer ArrayLongTempoTreatment[NB_MAX_LONG_TIMER];

/**
* Variable of the Tempo Long Manager structure.
* This constant defined the tempo long manager used by the Scheduler.
* It is the init function and the process function of the tempo long manager.
*/
extern const t_skscTempoProcess stmaLongTempoManager;

#endif /*__STMA_TEMPO_MANAGER_H*/
/******************************************************************************\
* END of FILE                                                                  *
\******************************************************************************/

