/*
 * moving_average.c
 *
 *  Created on: Feb 9, 2018
 *      Author: yuriy
 *
 *
 *  USAGE:
 *      1. Call moving_avg_init(...) on your filter handle
 *      2. Call moving_avg_fill_buffer(...) when you collected enough samples(equal to window size) to compute the first sample.
 *      3. Call moving_avg_filter_sample(...) on each new sample.
 *
 *      If you need to reset filter i.e. pause:
 *          4.1 Call moving_avg_flush(...)
 *
 *      After that you have to fill buffer again with:
 *          4.2 moving_avg_fill_buffer(...) before sampling.
 *
 *          No moving_avg_init(...) required.
 *
 *  You can also filter prepared sequence with:
 *      1. moving_avg_filter_sequence(...)
 *
 *   Algorithm:
 *      1. Keeps accumulative sum of the window.
 *      2. On each sample it subtracts the last sample and adds the new one.
 *      3. Then returns accumulative sum divided by window size. In case of HighPass filter returns middle element of the window
 *              minus low pass moving average sample.
 */


#include <stdlib.h>
#include <string.h>

#include "moving_average_filter.h"


/****** STATIC FUNCTION PROTOTYPES ********/
static int16_t moving_avg_compute_first_output(MovingAverageFilter_t *filter);
static int16_t moving_avg_filter_initalize(MovingAverageFilter_t *filter);
static int16_t produce_output(int16_t current_sample, int32_t acc, uint32_t window_size, FilterType_t ftype);


/**************************** PUBLIC API ****************************/

/**
 * @brief 	Initializes moving average filter
 * @param	filter		-	filter handle
 * @param	buffer		-	buffer which contains data.
 * @param   window_size -   moving average window size. Buffer length must match window size
 * @param	buf_type	-	buffer type. Either Ring or simple.
 *
 *
 * @return  Filter error status
 */
FilterStatus_t moving_avg_init(MovingAverageFilter_t *filter, FilterType_t ftype,
		int16_t *buffer, uint16_t window_size)
{
	filter->type = ftype;
	filter->window_size = window_size;

	filter->prev_acc = 0;
	filter->initialized = 0;

	FIFO_init(&filter->fifo, (uint8_t*)buffer, window_size, sizeof(*buffer), FIFO_LOOP);

	return FilterOK;
}


/**
 * @brief       Fill buffer with initial samples.
 *
 * @param[in]   filter  -   pointer to filter handle
 * @param[in]   data    -   pointer to data to copy. Length of data must be the same as filter window size.
 * @param[out]  y       -   pointer where computed sample will be stored.
 *
 * @return      Filter  error status
 */
FilterStatus_t moving_avg_fill_buffer(MovingAverageFilter_t *filter, int16_t *data, int16_t *y)
{
    if(filter->initialized)
    {
        return FilterError;
    }

    FIFO_t *fifo_ptr = &filter->fifo;
    uint32_t window_size = filter->window_size;

    if(FIFO_write(fifo_ptr, data, window_size, NULL) != FIFO_OK)
    {
        return FilterError;
    }

    *y = moving_avg_filter_initalize(filter);

    return FilterOK;
}


/**
 * @brief	Produces one output sample from ring buffer.
 * @note	Does not work with Simple buffer. Since it keeps track of internal
 * 				read pointer of ring buffer.
 *
 * @param[in]	    filter	-	filter handle. Must be configured with ring buffer
 * @param[in]       new_sample  -   new raw sample.
 * @param[out]  	y	-	variable where sample will be saved.
 *
 * @return  Filter error status
 */
FilterStatus_t moving_avg_filter_sample(MovingAverageFilter_t *filter, int16_t new_sample, int16_t *y)
{
    if(!filter->initialized)
    {
        return FilterError;
    }

    FIFO_t *fifo_ptr = &filter->fifo;
    FilterType_t type = filter->type;
    uint32_t window_size = filter->window_size;

    int32_t acc;
    int16_t middle, new_x, last_x;

    if(FIFO_read(fifo_ptr, &last_x, 1, NULL) != FIFO_OK)
    {
        return FilterError;
    }

    /**
     * Replace old sample with a new one
     */
    if(FIFO_write(fifo_ptr, &new_sample, 1, NULL) != FIFO_OK)
    {
        return FilterError;
    }

    acc = filter->prev_acc;
    new_x = new_sample;
    FIFO_get_middle_item(fifo_ptr, &middle);

    /* Cast in order to avoid overflow */
    acc += (int32_t)new_x - (int32_t)last_x;
    *y = produce_output(middle, acc, window_size, type);

    filter->prev_acc = acc;
	return FilterOK;
}


/**
 * @brief	Produces filtered sequence from simple buffer
 * @note	Does not work with ring buffer.
 * @note 	Assumes that internal buffer is fully filled with samples
 *
 * @param[in]	data	    -	data to be filtered
 * @param[in]   data_size   -   data length
 * @param[in]   window_size -   moving average window size
 * @param[out]	y	        -	buffer to save filtered data into.
 * @param[out]	y_data_len	- 	output sequence length. You can predict it with moving_avg_get_output_data_len.
 *
 * @return      Filter error status
 */
FilterStatus_t	moving_avg_filter_sequence(int16_t *data, uint16_t data_size,
        uint16_t window_size, int16_t *y, uint16_t *y_data_len)
{
    if(window_size > data_size)
    {
        return FilterError;
    }

	uint32_t filtered_len = filter_windowed_get_expected_output_len(data_size, window_size); // Always > 0
	uint32_t recursive_items_num = 	filtered_len - 1; 			// Always >= 0

	/* Contains accumulative sum */
	int32_t acc = 0;

	/* Computing the first output item */
	for(uint32_t i=0; i<window_size; i++)
	{
		acc += (int32_t)data[i];
	}

	uint32_t item_shift = 0;
	y[item_shift++] = acc / window_size;

	/* Recursive part */
	uint32_t p = (window_size-1) / 2;

	for(uint32_t i=0; i<recursive_items_num; i++)
	{
		uint32_t prev_sample_id = i;
		uint32_t next_sample_id = i+2*p+item_shift;

		acc += data[next_sample_id] - data[prev_sample_id];
		y[i+item_shift] = acc / (int32_t)window_size;
	}

	*y_data_len = filtered_len;

	return FilterOK;
}


/**
 * @brief	Returns expected filtered sequence length.
 * @note	Works only with Simple buffer.
 * @param	filter	-	filter handle
 * @param	y_len	-	expected output length
 * @return	Filter error status.
 */
FilterStatus_t moving_avg_get_output_data_len(uint16_t data_size, uint16_t window_size, uint16_t *y_len)
{
    if(window_size > data_size)
    {
        return FilterError;
    }

	*y_len = filter_windowed_get_expected_output_len(data_size, window_size);

	return FilterOK;
}


void moving_avg_flush(MovingAverageFilter_t *filter)
{
    FIFO_t *fifo_ptr = &filter->fifo;
    FIFO_flush(fifo_ptr);

    filter->initialized = 0;
}




/**************************** PRIVATE API ****************************/

/**
 * @brief	Internal initialization of filter. Computes first output sample.
 * 				After that recursive implementation will be used.
 *
 * @param	filter	-	filter handle.
 * @return	Filtered sample
 */
static int16_t moving_avg_filter_initalize(MovingAverageFilter_t *filter)
{
	filter->initialized = 1;
	return moving_avg_compute_first_output(filter);
}



/**
 * @brief	Computes the first output sample of a filter allowing recursive computations later.
 * @param	filter	-	filter handle
 * @return	Filtered sample
 */
static int16_t moving_avg_compute_first_output(MovingAverageFilter_t *filter)
{
	int16_t sample;
	int32_t acc = 0;
	uint32_t window_size = filter->window_size;
	FilterType_t ftype = filter->type;
	FIFO_t *fifo_ptr = &filter->fifo;

	int16_t buf[filter->window_size];
	if(FIFO_read(fifo_ptr, buf, window_size, NULL) != FIFO_OK)
	{
	    return FilterError;
	}

	/**
	 * Write the same data again for future usage
	 */
	if(FIFO_write(fifo_ptr, buf, window_size, NULL) != FIFO_OK)
	{
	    return FilterError;
	}

	for(uint32_t i=0; i<filter->window_size; i++)
	{
		acc += (int32_t)buf[i];
	}

	int16_t middle;
	FIFO_get_middle_item(fifo_ptr, &middle);

	filter->prev_acc = acc;
	sample = produce_output(middle, acc, window_size, ftype);

	return sample;
}



static int16_t produce_output(int16_t current_sample, int32_t acc, uint32_t window_size, FilterType_t ftype)
{
	int16_t sample;

	if(ftype == FilterHighPass)
	{
		sample = (int32_t)current_sample - (acc / (int32_t)window_size);
	}
	else
	{
		sample = acc / (int32_t)window_size;
	}

	return sample;
}
