/*
 * FIFO.h
 *
 *  Created on: Feb 19, 2018
 *      Author: yuriy
 */

#ifndef SRC_LIB_FIFO_FIFO_H_
#define SRC_LIB_FIFO_FIFO_H_

#include <stdint.h>
#include <stdbool.h>

#include "FIFO_def.h"
#include "FIFO8.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct _fifo {

    FIFO8_t fifo8;
    uint32_t item_size;
    uint32_t fifo_size;

} FIFO_t;


void FIFO_init(FIFO_t *fifo, uint8_t *buffer, uint16_t fifo_size,
        uint16_t item_size, uint8_t flags);
FIFO_error_t FIFO_read(FIFO_t *fifo, void *pData, uint16_t len, uint16_t *ir);
FIFO_error_t FIFO_write(FIFO_t *fifo, void *pData, uint16_t len, uint16_t *iw);
void FIFO_get_first_item(FIFO_t *fifo, void *item);
void FIFO_get_middle_item(FIFO_t *fifo, void *item);
void FIFO_get_last_item(FIFO_t *fifo, void *item);
uint16_t FIFO_get_data_count(FIFO_t *fifo);
uint16_t FIFO_get_free_space(FIFO_t *fifo);
bool FIFO_is_enough_free_space(FIFO_t *fifo, uint16_t len);
bool FIFO_not_empty(FIFO_t *fifo);
void FIFO_flush(FIFO_t *fifo);


#ifdef __cplusplus
}
#endif

#endif /* SRC_LIB_FIFO_FIFO_H_ */
