/**
********************************************************************************
* @file sbmaBufferManager.c
* Module of buffers management.
* This module gives a standard interface for the buffer handling, like creation,
* read, write, delete ...
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

// Definition of the tempos and tasks tables
#include "ConfigTable.h"

// include of this code file
#include "sbmaBufferManager.h"

/******************************************************************************\
* PRIVATE SYMBOLIC CONSTANTS and MACROS                                        *
\******************************************************************************/
/// @cond
/** size of the buffer header */
#define HEADER_SIZE     (2)

#define MSK_VALIDATE    (0x80)
#define MSK_IDX         (0x7F)
/// @endcond

/******************************************************************************\
* PRIVATE TYPES, STRUCTURES, UNIONS ans ENUMS                                  *
\******************************************************************************/

/* NO PRIVATE TYPE */

/******************************************************************************\
* PRIVATE MEMBER VARIABLES                                                     *
\******************************************************************************/
/** Index of the first element of the buffer */
static t_sbmaBufferSize mWriteIndex[sbmaMAX_BUFFER];

/** Index list for each buffer */
static uint16_t mIndexList[sbmaMAX_BUFFER];

static const uint16_t mValToMsk[16] = { 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
                                        0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000 };

/******************************************************************************\
* PRIVATE FUNCTION PROTOTYPES                                                  *
\******************************************************************************/

static t_sbmaBufferMsgId GetFreeIndex (t_sbmaBufferIndex parBufferIndex);
static t_sbmaBufferSize  SearchMessage(t_sbmaBufferIndex parBufferIndex, t_sbmaBufferMsgId parMsgId);
/******************************************************************************/
/* Remarks about data format in the buffer                                    */
/******************************************************************************/
/*
   The messages in the buffer are writing with this format :
      ----------------------------------------------------------------------------
      | MSG Index (n) | MSG (n) Size (of Data) | Data of n | MSG Index (n+1) | ..
      --------------------------------------------------------------------------
*/



/******************************************************************************\
* PRIVATE FUNCTION CODE                                                        *
\******************************************************************************/

/**
* Returns a free index
* @param [in] parBufferIndex : Index of the buffer which will contain the new message
* @return The id that can be used, or sbmaBUFFER_FULL if no more place
*/
static t_sbmaBufferMsgId GetFreeIndex(t_sbmaBufferIndex parBufferIndex)
{
    uint16_t wIndex = 0;
    t_sbmaBufferMsgId wBuufferMsgId = sbmaBUFFER_FULL;
    uint8_t wStop=0;

    /* Search the first free index */
    while( (wIndex<16) && ( wStop == 0))
    {

        if ((mIndexList[parBufferIndex] & mValToMsk[wIndex]) == 0)
        {
            mIndexList[parBufferIndex] |= mValToMsk[wIndex];
            wBuufferMsgId = (t_sbmaBufferMsgId)wIndex;
            wStop = 1;
        }
        wIndex++;
    }

    /* No more free index */
    return (wBuufferMsgId);
}


/**
* Search a message with the corresponding id
* @param [in] parBufferIndex : Index of the buffer which will contain the new message
* @param [in] parMsgId : Id if the message that must be deleted
* @return The offset of the message, or sbmaBUFFER_EMPTY if the message wasn't found
*/
static t_sbmaBufferSize SearchMessage(t_sbmaBufferIndex parBufferIndex, t_sbmaBufferMsgId parMsgId)
{
    t_sbmaBufferSize wIndex = 0;
    uint8_t wStop=0;
    t_sbmaBufferSize wIndexRc = sbmaBUFFER_EMPTY;

    /* Searches the index */
    while ((mWriteIndex[parBufferIndex] > wIndex)&&(wStop==0))
    {
        if (((mBufTab[parBufferIndex].Ptr)[wIndex] & MSK_IDX) == parMsgId)
        {
            /* The index is found, we can return it */
            wStop = 1;
            wIndexRc = wIndex;
        }

        /* Jumps to the next message */
        wIndex += (mBufTab[parBufferIndex].Ptr)[(wIndex + 1)] + HEADER_SIZE;
    }

    /* This index is not in the buffer */
    return (wIndexRc);
}

/******************************************************************************\
* PUBLIC FUNCTION CODE                                                         *
\******************************************************************************/

/**
* Initialise all buffers
*/
void sbmaInit(void)
{
    t_sbmaBufferIndex wIndexBuffer;

    /* Makes data initialisation for each buffer */
    for (wIndexBuffer=0 ; wIndexBuffer<sbmaMAX_BUFFER ; wIndexBuffer++)
    {
        /* Resets all data for this buffer */
        sbmaFreeAll(wIndexBuffer);
    }
}

/**
* Allocate space in the buffer for a new message
* @param [in] parBufferIndex : Index of the buffer which will contain the new message
* @param [in] parSize : Size of the message
* @return The newly created message id, or sbmaBUFFER_FULL if no more place is available
*/
t_sbmaBufferMsgId sbmaAllocate(t_sbmaBufferIndex parBufferIndex, t_sbmaBufferSize parSize)
{
    t_sbmaBufferMsgId wNewIndex = sbmaBUFFER_FULL ;
    t_sbmaBufferMsgId wIndexRc  = sbmaBUFFER_FULL ;

    /* Check free space */
    if ((mBufTab[parBufferIndex].Size - mWriteIndex[parBufferIndex]) >= (parSize + HEADER_SIZE))
    {
        /* Allocate the new buffer*/
        wNewIndex = GetFreeIndex(parBufferIndex);
        if (wNewIndex != sbmaBUFFER_FULL)
        {
            /* A buffer is available */
            (mBufTab[parBufferIndex].Ptr)[mWriteIndex[parBufferIndex]] = wNewIndex;
            (mBufTab[parBufferIndex].Ptr)[mWriteIndex[parBufferIndex] + 1] = parSize;
            mWriteIndex[parBufferIndex] += parSize + HEADER_SIZE;
        }

        /* Returns the new index (or error if no index) */
        wIndexRc = wNewIndex;
    }

    /* Not enough space free */
    return (wIndexRc);
}


/**
* Free a message in the specified buffer that was allocated.
* @param [in] parBufferIndex : Index of the buffer which will contain the new message
* @param [in] parMsgId : Id if the message that must be deleted
* @return A boolean with value TRUE if the buffer was deleted, and FALSE if the index was not found
*/
bool_t sbmaFree(t_sbmaBufferIndex parBufferIndex, t_sbmaBufferMsgId parMsgId)
{
    t_sbmaBufferSize wMsgIndex;
    t_sbmaBufferSize wSize;
    bool_t wRc = TRUE;

    /* Search the message */
    wMsgIndex = SearchMessage(parBufferIndex, parMsgId);
    if (wMsgIndex == sbmaBUFFER_EMPTY)
    {
        /* This message was not found in the buffer */
        wRc = FALSE;
    }
    else
    {

        /* Displace all data from the next messages */
        wSize = ((mBufTab[parBufferIndex].Ptr)[wMsgIndex + 1] + HEADER_SIZE);
        mWriteIndex[parBufferIndex] -= wSize;
        do
        {
            (mBufTab[parBufferIndex].Ptr)[wMsgIndex] = (mBufTab[parBufferIndex].Ptr)[wMsgIndex + wSize];
        }
        while (++wMsgIndex < mWriteIndex[parBufferIndex]);

        mIndexList[parBufferIndex] &= (uint16_t) ~mValToMsk[parMsgId];
    }
    /* Operation successfull */
    return (wRc );
}

/**
* Free all messages in the specified buffer that was allocated.
* @param [in] parBufferIndex : Index of the buffer which will contain the new message
* @return A boolean with value TRUE if the buffer was cleared, and FALSE if the index was not found
*/
bool_t sbmaFreeAll(t_sbmaBufferIndex parBufferIndex)
{

    uint16_t wIndexByte;
    bool_t wRc = TRUE;

    // test if good Buffer index
    if(parBufferIndex < sbmaMAX_BUFFER )
    {
        /* Resets all data for this buffer parBufferIndex */
        for (wIndexByte = 0 ; wIndexByte < mBufTab[parBufferIndex].Size ; wIndexByte++)
        {
            mBufTab[parBufferIndex].Ptr[wIndexByte] = sbmaIDX_MSG_FREE;
        }
        /* Initialises the index of the buffer */
        mWriteIndex[parBufferIndex] = 0;
        mIndexList[parBufferIndex] = 0;
    }
    else
    {
       // wrong buffer index
        wRc = FALSE;
    }
    return wRc;
}

/**
* Writes data to a previously allocated message
* @param [in] parBufferIndex : Index of the buffer which will receive the data
* @param [in] parMsgId : Id of the message that must be written
* @param [in] parByteOffset : Offset to start writting in the specified message
* @param [in] parSize : Size of data that must be written
* @param [in] parPtr : Pointer on the first element of the source data
* @return A boolean with value TRUE if all data are written correctly, and FALSE if there was an error
*/
bool_t sbmaWrite(t_sbmaBufferIndex parBufferIndex, t_sbmaBufferMsgId parMsgId,
                 t_sbmaByteIndex parByteOffset, t_sbmaBufferSize parSize, uint8_t* parPtr)
{
    t_sbmaBufferSize wMessageIndex;
    bool_t wRc;

    /* Search the specified message */
    wMessageIndex = SearchMessage(parBufferIndex, parMsgId);
    if (wMessageIndex == sbmaBUFFER_EMPTY)
    {
        /* This message does not exist */
        wRc = FALSE;
    }

    /* Make verification of nb data that are written */
    else if (parSize + parByteOffset > (mBufTab[parBufferIndex].Ptr)[wMessageIndex + 1])
    {
        /* To much data must be written, error */
        wRc =  (FALSE);
    }
    else
    {
        /* All verifications are OK, we can start the write */
        wMessageIndex += parByteOffset + HEADER_SIZE;
        do
        {
            (mBufTab[parBufferIndex].Ptr)[wMessageIndex++] = *(parPtr++);
        }
        while (--parSize != 0);
        wRc = TRUE;
    }

    return (wRc);
}


/**
* Returns the next message of a buffer, refering to the message specified in parameter
* @param [in] parBufferIndex : Index of the buffer
* @param [in] parMsgId : Id of the message. If ID is sbmaIDX_MSG_FREE, returns the first message
* @return : ID of the message, or sbmaBUFFER_EMPTY if no more message
*/
t_sbmaBufferMsgId sbmaGetNextMessage(t_sbmaBufferIndex parBufferIndex, t_sbmaBufferMsgId parMsgId)
{
    t_sbmaBufferSize wIndex = 0;
    t_sbmaBufferMsgId wMsgIdRc = 0;
    uint8_t wStop = 0;

    /* Check if the first message is wanted */
    if (parMsgId != sbmaIDX_MSG_FREE)
    {
        /* We search the index of the wanted message */
        wIndex = SearchMessage(parBufferIndex, parMsgId);
        if (wIndex == sbmaBUFFER_EMPTY)
        {
            /* Ce message n'est pas dans le buffer */
            wMsgIdRc = sbmaBUFFER_EMPTY;
        }
        else
        {
        /* Jump to the next message */
        wIndex += (mBufTab[parBufferIndex].Ptr)[wIndex + 1] + HEADER_SIZE;
        }
    }

    if(wMsgIdRc!=sbmaBUFFER_EMPTY)
    {
         wMsgIdRc = sbmaBUFFER_EMPTY;
        /* Now we search the first message that is validated (including the one at the current position) */
        while( (wIndex < mWriteIndex[parBufferIndex])&& (wStop ==0))
        {
            /* Is the message validated */
            if ((mBufTab[parBufferIndex].Ptr)[wIndex] > MSK_IDX)
            {
                /* Ok, we can return this index */
                wMsgIdRc =  (t_sbmaBufferMsgId)((mBufTab[parBufferIndex].Ptr)[wIndex] & (uint8_t)MSK_IDX);
                wStop =1;
            }

            /* Increments the index */
            wIndex += (mBufTab[parBufferIndex].Ptr)[wIndex + 1] + HEADER_SIZE;
         }

    }

    /* No message found */
    return (wMsgIdRc);
}


/**
* Returns the size of an existing message
* @param [in] parBufferIndex : Index of the buffer which will receive the data
* @param [in] parMsgId : Id if the message that must be written
* @return : Size of the message, or sbmaBUFFER_EMPTY if no message
*/
t_sbmaBufferSize sbmaGetMessageSize(t_sbmaBufferIndex parBufferIndex, t_sbmaBufferMsgId parMsgId)
{
    t_sbmaBufferSize wIndex;
    t_sbmaBufferSize wBuffersize;

    /* Searches the specified message */
    wIndex = SearchMessage(parBufferIndex, parMsgId);
    if (wIndex == sbmaBUFFER_EMPTY)
    {
        /* Ce message n'est pas dans le buffer */
        wBuffersize = (sbmaBUFFER_EMPTY);
    }
    else
    {
        wBuffersize = (mBufTab[parBufferIndex].Ptr)[wIndex + 1];
    }

    /* Message was found, now we return the size */
    return (wBuffersize);
}


/**
* Reads data from a previously allocated message
* @param [in] parBufferIndex : Index of the buffer which will be red
* @param [in] parMsgId : Id if the message that must be red
* @param [in] parByteOffset : Offset to start reading in the specified message
* @param [in] parSize : Size of data that must be red
* @param [in,out] parPtr : Pointer on the first element of the target buffer
* @return A boolean with value TRUE if all data are red correctly, and FALSE if there was an error
*/
bool_t sbmaRead(t_sbmaBufferIndex parBufferIndex, t_sbmaBufferMsgId parMsgId,
                t_sbmaByteIndex parByteOffset, t_sbmaBufferSize parSize, uint8_t* parPtr)
{
    t_sbmaBufferSize wIndex;
    bool_t wRc;

    /* Searches the specified message */
    wIndex = SearchMessage(parBufferIndex, parMsgId);
    if (wIndex == sbmaBUFFER_EMPTY)
    {
        /* This message does not exist */
        wRc = FALSE;
    }

    /* Make verification of nb data that must be red */
    else if (parSize + parByteOffset > (mBufTab[parBufferIndex].Ptr)[wIndex + 1])
    {
        /* To much data must be written, error */
        wRc = FALSE;
    }
    else
    {
        /* All verifications are OK, we can start to read */
        wIndex += parByteOffset + HEADER_SIZE;
        do
        {
            *(parPtr++) = (mBufTab[parBufferIndex].Ptr)[wIndex++];
        }
        while (--parSize != 0);
        wRc = TRUE;
    }

    return (wRc);
}


/**
* Validates a message in a buffer, so it can be used
* @param [in] parBufferIndex : Index of the buffer which will be read
* @param [in] parMsgId : Id if the message that must be red
* @return TRUE if the message is validated, FALSE otherwise
*/
bool_t sbmaValidate(t_sbmaBufferIndex parBufferIndex, t_sbmaBufferMsgId parMsgId)
{
    t_sbmaBufferSize wIndex;
    bool_t wRc;

    /* Searches the specified message */
    wIndex = SearchMessage(parBufferIndex, parMsgId);
    if (wIndex == sbmaBUFFER_EMPTY)
    {
        /* This message does not exist */
        wRc = FALSE;
    }
    else
    {

        /* The message exist, now we can set the validate bit */
        (mBufTab[parBufferIndex].Ptr)[wIndex++] |= MSK_VALIDATE;
        wRc = TRUE;
    }

    /* It's ok */
    return (wRc);
}

/******************************************************************************\
* END OF FILE                                                                  *
\******************************************************************************/


