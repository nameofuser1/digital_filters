#ifndef FIFO_H_
#define FIFO_H_

#include <stdint.h>
#include <stdbool.h>

#include "FIFO_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define FIFO_TYPE	uint8_t

typedef struct tagFIFO8_t {
    FIFO_TYPE *pBuffer;

    uint16_t FIFO_size;

    uint8_t flags;

    uint16_t counter;
    uint16_t r_index;
    uint16_t w_index;
} FIFO8_t;

void FIFO8_init(FIFO8_t *pFIFO, FIFO_TYPE *pBuffer, uint16_t size,
        uint8_t flags);

FIFO_error_t FIFO8_read(FIFO8_t *pFIFO, FIFO_TYPE *pData, uint16_t len,
        uint16_t *br);
FIFO_error_t FIFO8_write(FIFO8_t *pFIFO, FIFO_TYPE *pData, uint16_t len,
        uint16_t *bw);

uint16_t FIFO8_get_data_count(FIFO8_t *pFIFO);
uint16_t FIFO8_get_free_space(FIFO8_t *pFIFO);
bool FIFO8_is_enough_free_space(FIFO8_t *pFIFO, uint16_t len);
bool FIFO8_not_empty(FIFO8_t *pFIFO);
void FIFO8_flush(FIFO8_t *pFIFO);

#ifdef __cplusplus
}
#endif

#endif
