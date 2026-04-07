/**
********************************************************************************
* @file slpmLowPowerManager.h
* This is the interface of the Low Power Manager.
* The Low Power Manager is a semaphore which is used by the scheduler to check
* if entering in Low Power is enabled. This semaphore is reserved when a ressource
* in use not allowed entering in Low power Mode.
********************************************************************************
*/
// Avoid multiple inclusion
#ifndef __SLMP_LOW_POWER_MANAGER_H
#define __SLMP_LOW_POWER_MANAGER_H

/******************************************************************************\
* PUBLIC SYMBOLIC CONSTANTS and MACROS                                         *
\******************************************************************************/
// NO DEFINE

/******************************************************************************\
* PUBLIC TYPES, STRUCTURES, UNIONS ans ENUMS                                   *
\******************************************************************************/
// NO PUBLIC STRUCTURE

/******************************************************************************\
* PUBLIC VARIABLES                                                             *
\******************************************************************************/
// NO PUBLIC VARIABLE

/******************************************************************************\
* PUBLIC FUNCTION PROTOTYPES                                                   *
\******************************************************************************/
/* Prototype of function to enable for a specific ressource entering in Low Power Mode */
void slpmLowPowerEnable(t_EnumLowPowerSema parSemaElt);
/* Prototype of function to disable for a specific ressource entering in Low Power Mode */
void slpmLowPowerDisable(t_EnumLowPowerSema parSemaElt);
/* Prototype of function to manage entering in Low Power Mode : test if sema is
 * free, and if so entering in low power mode */
void slpmLowPowerManager(void);

#endif /*__SLMP_LOW_POWER_MANAGER_H*/
/******************************************************************************\
* END of FILE                                                                  *
\******************************************************************************/

