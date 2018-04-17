/*
 * rank_fitler.c
 *
 *  Created on: Feb 9, 2018
 *      Author: yuriy
 */

#include <stdlib.h>
#include <string.h>

#include "rank_filter.h"

/**
 *  USAGE:
 *      1. Call rank_filter_init(...) on your filter handle
 *      2. Call rank_filter_fill_buffer(...) when you collected enough samples(equal to window size) to compute the first sample.
 *      3. Call rank_filter_filter_sample(...) on each new sample.
 *
 *      If you need to reset filter i.e. pause:
 *          4.1 Call rank_filter_flush(...)
 *
 *      After that you have to fill buffer again with:
 *          4.2 rank_filter_fill_buffer(...) before sampling.
 *
 *          No rank_filter_init(...) required.
 *
 *  You can also filter prepared sequence with:
 *      1. rank_filter_filter_sequence(...)
 *
 *   Algorithm:
 *      1. When buffer is filled for the first time it sorts window with qsort and return element with given rank.
 *      2. On each new sample it removes last sample from sorted window and inserts new sample into it.
 */


/* Private functions prototypes */
static int sort_cmp_func(const void *pdata1, const void *pdata2);
static inline FilterStatus_t rank_filter_compute_first_output(RankFilter_t *filter, int16_t *y);
static inline FilterStatus_t rank_filter_compute_next_sample(RankFilter_t *filter, int16_t new_sample, int16_t *y);



/*********************************************************************************/
/*****							PUBLIC API									*****/


/**
 * @brief 	Performs initialization of rank filter
 * @param	rank_filter	- rank filter handle
 * @param 	buffer		-	buffer with incoming data
 * @param	window_size	-	filter window size. Length of buffer must match window size.
 * @param	rank		-	filter rank
 *
 * @return	Filter status
 */
FilterStatus_t	rank_filter_init(RankFilter_t *rank_filter, int16_t *buffer, uint16_t window_size, uint16_t rank)
{
	if(rank > window_size - 1)
	{
		return FilterError;
	}

	rank_filter->window_size = window_size;
	rank_filter->rank = rank;
	rank_filter->initialized = 0;

    rank_filter->sorted_window = _malloc((sizeof rank_filter->sorted_window) * rank_filter->window_size);
    if(rank_filter->sorted_window == NULL)
    {
        return FilterError;
    }

    FIFO_init(&rank_filter->fifo, (uint8_t*)buffer, window_size, sizeof(*buffer), FIFO_NO_FLAGS);

	return FilterOK;
}


/**
 * @brief       Fill rank filter buffer for the first time
 *
 * @param[in]   rf  -   pointer to rank filter handle
 * @param[in]   sample  -   sample to be written. Length must match filter window size
 * @param[out]  y   -   pointer where sample will be stored. Can be NULL.
 *
 * @return      Filter error status
 */
FilterStatus_t rank_filter_fill_buffer(RankFilter_t *rf, int16_t *samples, int16_t *y)
{
    if(rf->initialized)
    {
        return FilterError;
    }

    FIFO_t *fifo_ptr = &rf->fifo;
    uint16_t window_size = rf->window_size;

    if(FIFO_write(fifo_ptr, samples, window_size, NULL) != FIFO_OK)
    {
        return FilterError;
    }

    rf->initialized = 1;
    return rank_filter_compute_first_output(rf, y);
}


/**
 * @brief	    Computes the next filtered sample.

 * @note	    Time complexity is O(window_size).
 * @note	    Memory complexity is O(window_size)
 *
 * @param[in]	rank_filter	- rank filter handle
 * @param[in]   new_sample  -   new sample to be written
 * @param[out]	y	-	pointer to where filtered sample will be written. Can be NULL.
 *
 * @return	    Filter error status
 */
FilterStatus_t rank_filter_filter_sample(RankFilter_t *rank_filter, int16_t new_sample, int16_t *y)
{
	if(!rank_filter->initialized)
	{
		return FilterError;
	}

	return rank_filter_compute_next_sample(rank_filter, new_sample, y);
}


/**
 * @brief 	    Performs rank filtering on a simple buffer.
 * @note	    NOT an optimal implementation. Time complexity is O(n * window_size * log(window_size))
 *
 * @param[in]   data        -   data to be filtered
 * @param[in]   data_size   -   data length
 * @param[in]   window_size -   rank filter window size
 * @param[in]   rank        -   rank filter rank
 * @param[out]  y           -   pointer where output data will be stored
 * @param[out]  y_len       -   output data length. You can predict it using rank_filter_get_output_data_len.
 *
 * @return      FilterStatus_t
 */
FilterStatus_t rank_filter_filter_sequence(int16_t *data, int16_t data_size, uint16_t window_size,
        uint16_t rank, int16_t *y, uint16_t *y_len)
{
    if(rank > window_size)
    {
        return FilterError;
    }

	uint32_t item_size = sizeof *data;

	int16_t 	window[window_size];
	uint16_t	filtered_len = filter_windowed_get_expected_output_len(data_size, window_size);

	for(uint16_t i=0; i<filtered_len; i++)
	{
		memcpy(window, data+i, window_size*item_size);

		qsort(window, window_size, 2, sort_cmp_func);
		y[i] = window[rank];
	}

	*y_len = filtered_len;
	return FilterOK;
}


/**
 * @brief       Returns expected filtered sequence length.
 *
 * @param[in]   data_size   -   data size
 * @param[in]   window_size -   filter window size
 * @param[out]  y_len   -   pointer to where expected length will be written.
 *
 * @return      Filter error status
 */
FilterStatus_t rank_filter_get_output_data_len(uint16_t data_size, uint16_t window_size, uint16_t *y_len)
{
    if(window_size > data_size)
    {
        return FilterError;
    }

    *y_len = filter_windowed_get_expected_output_len(data_size, window_size);
    return FilterOK;
}


/**
 *  @brief      Reset filter to unintialized state. You have to call rank_filter_fill_buffer again in order to use\
 *              rank_filter_sample.
 *
 *  @param[in]  rank_filter     -   pointer to rank filter
 */
void rank_filter_flush(RankFilter_t *rank_filter)
{
    FIFO_t *fifo_ptr = &rank_filter->fifo;

    FIFO_flush(fifo_ptr);
    rank_filter->initialized = 0;
}



/*********************************************************************************/
/*****							PRIVATE API									*****/

/**
 * @brief 	Comparasion function for qsort
 */
static int sort_cmp_func(const void *pdata1, const void *pdata2)
{
	return (*(int16_t*)pdata1 - *(int16_t*)pdata2);
}


/**
 * @brief	    Computes the first output of rank filter.
 * @note	    Time complexity is O(window_size*log(window_size)) since requires sorting.
 *
 * @param[in]	filter	-	rank filter handle
 * @param[out]  y   -   pointer to where filtered sample will be written
 *
 * @return 	    Filtered error status
 */
static inline FilterStatus_t rank_filter_compute_first_output(RankFilter_t *filter, int16_t *y)
{
	int16_t *sorted_window = filter->sorted_window;
	uint8_t item_size = sizeof *filter->sorted_window;
	uint16_t window_size = filter->window_size;
	uint16_t rank = filter->rank;

	FIFO_t *fifo_ptr = &filter->fifo;
	int16_t samples[window_size];

	/**
	 * Read samples
	 */
	if(FIFO_read(fifo_ptr, samples, window_size, NULL) != FIFO_OK)
	{
	    return FilterError;
	}

	/**
	 * Write them back
	 */
	if(FIFO_write(fifo_ptr, samples, window_size, NULL) != FIFO_OK)
	{
	    return FilterError;
	}

	/**
	 * Sort window
	 */
	memcpy(sorted_window, samples, window_size*item_size);
	qsort(sorted_window, window_size, item_size, sort_cmp_func);

	/**
	 * Output
	 */
	if(y != NULL)
	{
	    *y = sorted_window[rank];
	}

	return FilterOK;
}


/**
 * @brief 	Computes next filtered sample from a ring buffer.
 * @note	Time complexity is O(window_size).
 *
 * @param[in]	filter	-	rank filter handle
 * @param[in]   new_sample  -   new raw sample
 * @param[out]  y   -   pointer to where filtered sample will be written
 *
 * @return	    Filter error status
 */
static inline FilterStatus_t rank_filter_compute_next_sample(RankFilter_t *filter, int16_t new_sample, int16_t *y)
{
	int16_t *sorted_window = filter->sorted_window;
	uint16_t rank = filter->rank;
	uint16_t window_size = filter->window_size;
	uint16_t item_size = sizeof(new_sample);

	FIFO_t *fifo_ptr = &filter->fifo;
	int16_t last_sample;

	if(FIFO_read(fifo_ptr, &last_sample, 1, NULL) != FIFO_OK)
	{
	    return FilterError;
	}

	if(FIFO_write(fifo_ptr, &new_sample, 1, NULL) != FIFO_OK)
	{
	    return FilterError;
	}

	int32_t last_sample_rank = -1;
	int32_t new_sample_rank = -1;
	int32_t new_sample_rank_shift = 1;

	/* Determine last and new sample ranks */
	for(uint32_t i=0; i<window_size; i++)
	{
		if(sorted_window[i] == last_sample && last_sample_rank == -1)
		{
			last_sample_rank = i;
		}

		if(sorted_window[i] >= new_sample && new_sample_rank == -1)
		{
			new_sample_rank = i;
		}

		if(last_sample_rank != -1 && new_sample_rank != -1)
		{
			break;
		}
	}

	if(new_sample_rank == -1)
	{
		new_sample_rank = window_size - 1;
		new_sample_rank_shift = 0;
	}

	/* We remove last sample and insert new sample at its' position */
	uint32_t bytes_to_move;
	if(last_sample_rank < new_sample_rank)
	{
		bytes_to_move = (new_sample_rank - last_sample_rank)*item_size;
		memmove(sorted_window+last_sample_rank, sorted_window+last_sample_rank+1, bytes_to_move);

		sorted_window[new_sample_rank - new_sample_rank_shift] = new_sample;

	}
	else if(last_sample_rank > new_sample_rank)
	{
		bytes_to_move = (last_sample_rank - new_sample_rank)*item_size;
		memmove(sorted_window+new_sample_rank+1, sorted_window+new_sample_rank, bytes_to_move);

		sorted_window[new_sample_rank] = new_sample;
	}
	else // equal
	{
		sorted_window[last_sample_rank] = new_sample;
	}

	if(y != NULL)
	{
	    *y = sorted_window[rank];
	}

	return FilterOK;
}
