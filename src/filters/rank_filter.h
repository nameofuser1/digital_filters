/*
 * rank_filter.h
 *
 *  Created on: Feb 9, 2018
 *      Author: yuriy
 */

#ifndef SRC_MOD_FILTERS_RANK_FILTER_H_
#define SRC_MOD_FILTERS_RANK_FILTER_H_

#include "filter.h"
#include "fifo/FIFO.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct rank_filter {
	int16_t 	*sorted_window;
	uint16_t 	window_size;
	uint16_t	rank;

	uint8_t 	initialized;

	FIFO_t      fifo;
} RankFilter_t;


FilterStatus_t  rank_filter_init(RankFilter_t *rank_filter, int16_t *buffer, uint16_t window_size, uint16_t rank);
FilterStatus_t  rank_filter_fill_buffer(RankFilter_t *rf, int16_t *samples, int16_t *y);
FilterStatus_t  rank_filter_filter_sample(RankFilter_t *rank_filter, int16_t new_sample, int16_t *y);
FilterStatus_t  rank_filter_filter_sequence(int16_t *data, int16_t data_size, uint16_t window_size,
        uint16_t rank, int16_t *y, uint16_t *y_len);
FilterStatus_t  rank_filter_get_output_data_len(uint16_t data_size, uint16_t window_size, uint16_t *y_len);
void            rank_filter_flush(RankFilter_t *rank_filter);


#ifdef __cplusplus
}
#endif

#endif /* SRC_MOD_FILTERS_RANK_FILTER_H_ */
