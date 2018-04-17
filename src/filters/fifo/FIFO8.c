/*!

 @file    FIFO8.c
 @author  Zinoviev Sergey, Woodenshark LLC, 2015.
 @date    24-may-2017
 @brief   FIFO module.

 ______ History ________
 1. | -> || fix.

 */

/*! @addtogroup Modules
 @{
 */

/*!
 @defgroup FIFO8
 @brief    FIFO8 like buffers. Without any protection from concurrent access.
 @{
 */

#include "FIFO8.h"

#include <string.h>

/*!
 @brief  Initializes FIFO buffer by copying pointer to buffer and setting size of the buffer.

 @param  pFIFO - pointer to the FIFO structure.
 @param  pBuffer - pointer to buffer array.
 @param  size - size of the buffer array.
 @param  flags - are may be any combination of FIFO8_flags_t.

 @retval Nothing.
 */
void FIFO8_init(FIFO8_t *pFIFO, FIFO_TYPE *pBuffer, uint16_t size,
        uint8_t flags)
{
    pFIFO->pBuffer = pBuffer;
    pFIFO->FIFO_size = size;
    pFIFO->flags = flags;
    pFIFO->w_index = 0;
    pFIFO->r_index = 0;
    pFIFO->counter = 0;
}

/*!
 @brief  Reads data from the FIFO.
 
 @param  pFIFO - pointer to the FIFO structure.
 @param  len - length of data to read.
 @param  pData - pointer to copy data to.
 @param  br - bytes read count.

 @retval FIFO_OK - on success.
 @retval FIFO_NOT_INITIALIZED - fifo buffer not initialized.
 @retval FIFO_UNDERFLOW - no data to read.
 
 */
FIFO_error_t FIFO8_read(FIFO8_t *pFIFO, FIFO_TYPE *pData, uint16_t len,
        uint16_t *br)
{
    FIFO_error_t RetVal = FIFO_OK;
    uint16_t bytes_read = 0;

    if(pFIFO == NULL || pFIFO->pBuffer == NULL)
    {
        RetVal = FIFO_NOT_INITIALIZED;
    }

    if(RetVal == FIFO_OK)
    {
        while (len != 0)
        {
            if(pFIFO->counter != 0)
            {
                if(pData != NULL)
                {
                    *pData = pFIFO->pBuffer[pFIFO->r_index];
                    pData++;
                }

                if(pFIFO->r_index == (pFIFO->FIFO_size - 1))
                {
                    pFIFO->r_index = 0;
                }
                else
                {
                    pFIFO->r_index++;
                }

                pFIFO->counter--;
                len--;

                bytes_read++;
            }
            else
            {
                RetVal = FIFO_UNDERFLOW;
                break;
            }
        }
    }

    if(br != NULL)
    {
        *br = bytes_read;
    }

    return RetVal;
}

/*!
 @brief  Writes data into FIFO buffer.
 
 @param  pFIFO - pointer to the FIFO structure.
 @param  len - length of data to write.
 @param  pData - pointer to data to write.
 @param  bw - written bytes count, can be NULL.
 
 @retval FIFO_OK - on success.
 @retval FIFO_NOT_INITIALIZED - fifo buffer not initialized.
 @retval FIFO_OVERFLOW - no free space available.
 */
FIFO_error_t FIFO8_write(FIFO8_t *pFIFO, FIFO_TYPE *pData, uint16_t len,
        uint16_t *bw)
{
    FIFO_error_t RetVal = FIFO_OK;
    uint16_t bytes_written = 0;

    if(pFIFO == NULL || pFIFO->pBuffer == NULL)
    {
        RetVal = FIFO_NOT_INITIALIZED;
    }

    if(RetVal == FIFO_OK)
    {
        while (len != 0)
        {
            if(pFIFO->counter != pFIFO->FIFO_size
                    || (pFIFO->flags && FIFO_LOOP))
            {
                // copy data to the buffer
                pFIFO->pBuffer[pFIFO->w_index] = *pData++;

                if(pFIFO->w_index == (pFIFO->FIFO_size - 1))
                {
                    pFIFO->w_index = 0;
                }
                else
                {
                    pFIFO->w_index++;
                }

                if(pFIFO->flags && FIFO_LOOP)
                {
                    if(pFIFO->counter == pFIFO->FIFO_size)
                    {
                        pFIFO->r_index = pFIFO->w_index;
                    }
                }

                if(pFIFO->counter < pFIFO->FIFO_size)
                {
                    pFIFO->counter++;
                }

                bytes_written++;
                len--;
            }
            else
            {
                RetVal = FIFO_OVERFLOW;
                break;
            }
        }
    }

    if(bw != NULL)
    {
        *bw = bytes_written;
    }

    return RetVal;
}

/*!
 @brief    Gets bytes count stored in the FIFO.
 @note     
 @param    pFIFO - pointer to the FIFO buffer.
 @retval   Bytes count 0 - 65535.
 */
uint16_t FIFO8_get_data_count(FIFO8_t *pFIFO)
{
    return pFIFO->counter;
}

/*!
 @brief    Gets free space in the FIFO.
 @note     
 @param    pFIFO - pointer to the FIFO buffer.
 @retval   Bytes count 0 - 65535.
 */
uint16_t FIFO8_get_free_space(FIFO8_t *pFIFO)
{
    return (pFIFO->FIFO_size - pFIFO->counter);
}

/*!
 @brief    Check free space
 @note     
 @param    pFIFO - pointer to the FIFO buffer.
 @param    len - count of bytes to planning write
 @retval   true  if free space is enough
 */
bool FIFO8_is_enough_free_space(FIFO8_t *pFIFO, uint16_t len)
{
    return ((pFIFO->FIFO_size - pFIFO->counter) >= len);
}

/*!
 @brief    Return true if FIFO is not empty
 @note     
 @param    pFIFO - pointer to the FIFO buffer.
 @retval   true if FIFO is not empty
 */
bool FIFO8_not_empty(FIFO8_t *pFIFO)
{
    return pFIFO->counter;
}

/*!
 @brief  Flush FIFO
 @note     
 @param    pFIFO - pointer to the FIFO buffer.
 */
void FIFO8_flush(FIFO8_t *pFIFO)
{
    pFIFO->counter = 0;
    pFIFO->r_index = 0;
    pFIFO->w_index = 0;
}

/*!
 @}
 */

/*!
 @}
 */

