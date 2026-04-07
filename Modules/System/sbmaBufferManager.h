/**
********************************************************************************
* @file sbmaBufferManager.h
* Module of buffers management.
* This module gives a standard interface for the buffer handling, like creation,
* read, write, delete ...
********************************************************************************
*/
#ifndef __SBMA_BUFFER_MANAGER_H
#define __SBMA_BUFFER_MANAGER_H

/******************************************************************************\
* PUBLIC SYMBOLIC CONSTANTS and MACROS                                         *
\******************************************************************************/

/**
* possibles return values of the handling procedures.
*/
#define sbmaBUFFER_FULL    ((t_sbmaBufferMsgId) 0xFFFF) /**< Buffer Full Status */
#define sbmaBUFFER_EMPTY   ((t_sbmaBufferMsgId) 0xFFFF) /**< Buffer Empty Status */
#define sbmaIDX_MSG_FREE   (sbmaBUFFER_FULL)            /**< Message free Status */

/******************************************************************************\
* PUBLIC TYPES, STRUCTURES, UNIONS and ENUMS                                   *
\******************************************************************************/
/** Defines the maximum size of a buffer */
typedef uint8_t t_sbmaBufferSize;

/** Used to specify the buffer on which the operation must take place */
typedef uint8_t t_sbmaBufferIndex;

/** Defines the number of messages that can be created in a buffer */
typedef uint8_t t_sbmaBufferMsgId;

/** Contains the size and memory pointer to a buffer */
typedef struct
{
    uint8_t*            Ptr;   /**< Pointer on the first byte of the buffer */
    t_sbmaBufferSize    Size;  /**< Size of the buffer (in bytes) */
} t_sbmaBufStruct;

/** Defines the type of an index */
typedef uint8_t t_sbmaByteIndex;

/******************************************************************************\
* PUBLIC VARIABLES                                                             *
\******************************************************************************/
/** Buffer table declaration (in static mode) */
extern const t_sbmaBufStruct mBufTab[sbmaMAX_BUFFER];

/******************************************************************************\
* PUBLIC FUNCTION PROTOTYPES                                                   *
\******************************************************************************/

void                sbmaInit(void);

bool_t              sbmaFreeAll (t_sbmaBufferIndex parBufferIndex );

t_sbmaBufferMsgId   sbmaAllocate        (t_sbmaBufferIndex parBufferIndex,
                                         t_sbmaBufferSize parSize );

bool_t              sbmaWrite           (t_sbmaBufferIndex parBufferIndex,
                                         t_sbmaBufferMsgId parMsgId,
                                         t_sbmaByteIndex parByteOffset,
                                         t_sbmaBufferSize parSize,
                                         uint8_t* parPtr);

bool_t              sbmaValidate        (t_sbmaBufferIndex parBufferIndex,
                                         t_sbmaBufferMsgId parMsgId);

t_sbmaBufferMsgId   sbmaGetNextMessage  (t_sbmaBufferIndex parBufferIndex,
                                         t_sbmaBufferMsgId parMsgId);

t_sbmaBufferSize    sbmaGetMessageSize  (t_sbmaBufferIndex parBufferIndex,
                                         t_sbmaBufferMsgId parMsgId);

bool_t              sbmaRead            (t_sbmaBufferIndex parBufferIndex,
                                         t_sbmaBufferMsgId parMsgId,
                                         t_sbmaByteIndex parByteOffset,
                                         t_sbmaBufferSize parSize,
                                         uint8_t* parPtr);

bool_t              sbmaFree            (t_sbmaBufferIndex parBufferIndex,
                                         t_sbmaBufferMsgId parMsgId);

#endif /*__SBMA_BUFFER_MANAGER_H*/
/******************************************************************************\
* END OF FILE                                                                  *
\******************************************************************************/

