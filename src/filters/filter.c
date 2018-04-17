/*
 * filter.cpp
 *
 *  Created on: Feb 9, 2018
 *      Author: yuriy
 */


#include <stdint.h>
#include <stdlib.h>

#include "filter.h"


/**
 * @brief 	Updates ring buffer pointer
 */
static inline void update_buffer_ptr(uint32_t *ptr, uint32_t buf_size)
{
	*ptr = (*ptr >= buf_size - 1) ? 0 : ++(*ptr);
}


/**
 * @brief	Updates last_x and new_x read pointers of filter's buffer.
 */
void filter_update_buffer_ptrs(FilterBufferConfig_t *buffer_config)
{

	uint32_t *last_x_rd_ptr = &buffer_config->last_x_rd_ptr;
	uint32_t *new_x_rd_ptr = &buffer_config->new_x_rd_ptr;
	uint32_t *middle_x_rd_ptr = &buffer_config->middle_x_rd_ptr;
	uint32_t buf_size = buffer_config->buf_size;

	update_buffer_ptr(last_x_rd_ptr, buf_size);
	update_buffer_ptr(new_x_rd_ptr, buf_size);
	update_buffer_ptr(middle_x_rd_ptr, buf_size);
}


/**
 * @brief	Returns sequence length filtered with window method.
 */
uint32_t filter_windowed_get_expected_output_len(uint32_t data_len, uint32_t window_size)
{
	return data_len - window_size + 1;
}










