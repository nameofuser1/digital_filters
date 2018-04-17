/*
 *  FIFO.c
 *
 *  Created on: Feb 19, 2018
 *  Author: yuriy
 *
 *
 *  @file 	 FIFO.c
 *  @author  Kamnev Yuriy
 *  @date    19-Feb-2018
 *
 *  @brief   Wrapper around FIFO8 which can store any data type.
 */

#include <stddef.h>
#include <string.h>

#include "FIFO.h"

/*!
 @brief  Initializes underlying bytes FIFO.

 @param  fifo - pointer to the FIFO structure.
 @param  buffer - pointer to buffer array.
 @param  fifo_size - size of FIFO expressed in ITEMS.
 @param  item_size - FIFO's item size.
 @param  flags - are may be any combination of FIFO_flags_t.

 @retval Nothing.
 */
void FIFO_init(FIFO_t *fifo, uint8_t *buffer, uint16_t fifo_size,
        uint16_t item_size, uint8_t flags)
{
    uint16_t buffer_size = fifo_size * item_size;
    fifo->fifo_size = fifo_size;
    fifo->item_size = item_size;

    FIFO8_init(&fifo->fifo8, buffer, buffer_size, flags);
}

/*!
 @brief  Reads data from the FIFO.

 @param  fifo - pointer to the FIFO structure.
 @param  pData - pointer to copy data to.
 @param  len - number of items to be read.
 @param  ir - number of read items. Can be NULL.

 @retval FIFO_OK - on success.
 @retval FIFO_NOT_INITIALIZED - fifo buffer not initialized.
 @retval FIFO_UNDERFLOW - no data to read.

 */
FIFO_error_t FIFO_read(FIFO_t *fifo, void *pData, uint16_t len, uint16_t *ir)
{
    FIFO_error_t status;
    uint16_t br;

    status = FIFO8_read(&fifo->fifo8, pData, len * fifo->item_size, &br);

    if(ir != NULL)
    {
        *ir = br / fifo->item_size;
    }

    return status;
}

/*!
 @brief  Writes data into FIFO buffer.

 @param  fifo - pointer to the FIFO structure.
 @param  pData - pointer to data to be written.
 @param  len - number of items to write.
 @param  iw - written items count, can be NULL.

 @retval FIFO_OK - on success.
 @retval FIFO_NOT_INITIALIZED - fifo buffer not initialized.
 @retval FIFO_OVERFLOW - no free space available.
 */
FIFO_error_t FIFO_write(FIFO_t *fifo, void *pData, uint16_t len, uint16_t *iw)
{
    FIFO_error_t status;
    uint16_t bw;

    status = FIFO8_write(&fifo->fifo8, pData, len * fifo->item_size, &bw);

    if(iw != NULL)
    {
        *iw = bw / fifo->item_size;
    }

    return status;
}


/**
 * @brief	Return first item id in the buffer. Useful when using LOOP mode and have to process new samples on fly.
 * @note	Return the first sample in the STRAIGHTENED buffer. Not the next sample's id which will be read from FIFO.
 *
 * @retval  Id of the element in the CASTED to the original type buffer.
 */
void FIFO_get_first_item(FIFO_t *fifo, void *item)
{
    FIFO8_t *fifo8 = &fifo->fifo8;
    uint16_t bytes_buffer_ptr;

    if(fifo8->w_index == 0)
    {
        bytes_buffer_ptr = fifo8->FIFO_size - fifo->item_size;
    }
    else
    {
        bytes_buffer_ptr = fifo8->w_index - fifo->item_size;
    }

    memcpy(item, fifo8->pBuffer+bytes_buffer_ptr, fifo->item_size);
}


/**
 * @brief       Return middle item from FIFO.
 */
void FIFO_get_middle_item(FIFO_t *fifo, void *item)
{
    FIFO8_t *fifo8 = &fifo->fifo8;

    uint16_t fifo8_size = fifo8->FIFO_size;
    uint16_t read_ptr = fifo8->r_index;
    uint16_t wr_ptr = fifo8->w_index;

    uint16_t fifo8_middle_ptr;

    if(wr_ptr > read_ptr)
    {
        fifo8_middle_ptr = (wr_ptr + read_ptr) / 2;
    }
    else
    {
        /**
         * Middle item index in FIFO8 straightened buffer
         */
        uint16_t mis = (wr_ptr + (fifo8_size - read_ptr)) / 2;

        if(read_ptr + mis < fifo8_size)
        {
            fifo8_middle_ptr = mis + read_ptr;
        }
        else
        {
            fifo8_middle_ptr = mis - (fifo8_size - read_ptr);
        }
    }

    /**
     * Align index to item size
     */
    fifo8_middle_ptr -= (fifo8_middle_ptr % fifo->item_size);
    memcpy(item, fifo8->pBuffer+fifo8_middle_ptr, fifo->item_size);
}

/**
 * @brief	Return last item id in the buffer. Useful when using LOOP mode and have to process new samples on fly.
 * @note	Return the last sample in the STRAIGHTENED buffer. Not the last written sample which will be read from FIFO.
 *
 * @retval  Id of the element in the CASTED to the original type buffer.
 */
void FIFO_get_last_item(FIFO_t *fifo, void *item)
{
    FIFO8_t *fifo8 = &fifo->fifo8;
    memcpy(item, fifo8->pBuffer+fifo8->r_index, fifo->item_size);
}

/*!
 @brief    Gets items count stored in the FIFO.
 @note
 @param    fifo - pointer to the FIFO buffer.

 @retval   Items count 0 - 65535.
 */
uint16_t FIFO_get_data_count(FIFO_t *fifo)
{
    return FIFO8_get_data_count(&fifo->fifo8) / fifo->item_size;
}

/*!
 @brief    Gets free space in the FIFO.
 @note
 @param    fifo - pointer to the FIFO buffer.
 @retval   Items count 0 - 65535.
 */
uint16_t FIFO_get_free_space(FIFO_t *fifo)
{
    return FIFO8_get_free_space(&fifo->fifo8) / fifo->item_size;
}

/*!
 @brief    Check free space
 @note
 @param    fifo - pointer to the FIFO buffer.
 @param    len - number of items which is to be written

 @retval   true  if free space is enough
 */
bool FIFO_is_enough_free_space(FIFO_t *fifo, uint16_t len)
{
    return FIFO8_is_enough_free_space(&fifo->fifo8, len * fifo->item_size);
}

/*!
 @brief    Return true if FIFO is not empty
 @note
 @param    fifo - pointer to the FIFO buffer.
 @retval   true if FIFO is not empty
 */
bool FIFO_not_empty(FIFO_t *fifo)
{
    return FIFO8_not_empty(&fifo->fifo8);
}

/*!
 @brief  	Flush FIFO
 @note
 @param    fifo - pointer to the FIFO buffer.
 */
void FIFO_flush(FIFO_t *fifo)
{
    FIFO8_flush(&fifo->fifo8);
}
