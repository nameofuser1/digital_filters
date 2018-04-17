/*
 * moving_average.h
 *
 *  Created on: Feb 9, 2018
 *      Author: yuriy
 */

#ifndef SRC_MOD_FILTERS_MOVING_AVERAGE_FILTER_H_
#define SRC_MOD_FILTERS_MOVING_AVERAGE_FILTER_H_

#include "filter.h"
#include "fifo/FIFO.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct moving_average_filter {

    uint16_t            buffer_size;
	uint16_t 			window_size;
	int32_t				prev_acc;
	uint8_t				initialized;

	FilterType_t		type;
	FIFO_t              fifo;

} MovingAverageFilter_t;


FilterStatus_t  moving_avg_init(MovingAverageFilter_t *filter, FilterType_t ftype,
        int16_t *buffer, uint16_t window_size);
FilterStatus_t  moving_avg_fill_buffer(MovingAverageFilter_t *filter, int16_t *data, int16_t *y);
FilterStatus_t  moving_avg_filter_sample(MovingAverageFilter_t *filter, int16_t new_sample, int16_t *y);
FilterStatus_t  moving_avg_filter_sequence(int16_t *data, uint16_t data_size,
        uint16_t window_size, int16_t *y, uint16_t *y_data_len);
FilterStatus_t  moving_avg_get_output_data_len(uint16_t data_size, uint16_t window_size, uint16_t *y_len);
void            moving_avg_flush(MovingAverageFilter_t *filter);



#ifdef __cplusplus
}
#endif

#endif /* SRC_MOD_FILTERS_MOVING_AVERAGE_FILTER_H_ */
