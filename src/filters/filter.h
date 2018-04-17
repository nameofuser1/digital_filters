/*
 * filter.h
 *
 *  Created on: Feb 9, 2018
 *      Author: yuriy
 */

#ifndef SRC_MOD_FILTERS_FILTER_H_
#define SRC_MOD_FILTERS_FILTER_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Redefine malloc if needed
 */
#define	_malloc	malloc

typedef enum {FilterLowPass, FilterHighPass} FilterType_t;

typedef enum {FilterOK=0, FilterError} FilterStatus_t;

typedef enum {FilterRingBuffer=0, FilterSimpleBuffer} FilterBufferType_t;


typedef struct _filter_buffer_config {
	uint32_t last_x_rd_ptr;
	uint32_t new_x_rd_ptr;
	uint32_t middle_x_rd_ptr;
	uint32_t buf_size;
} FilterBufferConfig_t;


// void update_buffer_ptr(uint32_t *ptr, uint32_t buf_size);
void filter_update_buffer_ptrs(FilterBufferConfig_t *filter);
uint32_t filter_windowed_get_expected_output_len(uint32_t data_len, uint32_t window_size);


#ifdef __cplusplus
}
#endif

#endif /* SRC_MOD_FILTERS_FILTER_H_ */
