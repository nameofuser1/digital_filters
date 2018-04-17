//============================================================================
// Name        : Filters.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <assert.h>
using namespace std;

#include "filters/filter.h"
#include "filters/rank_filter.h"
#include "filters/moving_average_filter.h"


#define FILTER_ASSERT(status) 	if(status != FilterOK) {cout << "Error at: " << __FILE__ << " " << __LINE__ << "\r\n";}
#define FIFO_ASSERT(status)		if(status != FIFO_OK) {cout << "Error at: " << __FILE__ << " " << __LINE__ << "\r\n";}


static void test_moving_average_simple_buffer(void)
{
	FilterStatus_t 	status;

	const uint16_t window_size = 3;
	const uint16_t buf_size = 16;
	const uint16_t output_length = 14;

	int16_t buffer[buf_size] = {5, 2, 1, 5, 10, 14, 32, 65, 13, 18, -10, -25, -30, 13, 1, 1};
	int16_t filtered[output_length] = {2, 2, 5, 9, 18, 37, 36, 32, 7, -5, -21, -14, -5, 5};

	uint16_t output_len;

	status = moving_avg_get_output_data_len(buf_size, window_size, &output_len);
	FILTER_ASSERT(status);

	//cout << "Expected output length: " << output_len << std::endl;

	int16_t out_data[output_len];
	status = moving_avg_filter_sequence(buffer, buf_size, window_size, out_data, &output_len);

	assert(output_len == output_length);

	//cout << "Exact output length: " << output_len << endl;

	for(unsigned int i=0; i<output_len; i++)
	{
	    //cout << out_data[i] << " " << filtered[i] << endl;
		assert(out_data[i] == filtered[i]);
	}
}




static void test_moving_average_ring_buffer_fifo(void)
{
	FilterStatus_t 	status;
	MovingAverageFilter_t average;

	const uint32_t window_size = 3;
	const uint32_t samples_buf_size = 16;
	const uint32_t fifo_buf_size = 3;
	const uint16_t output_length = 14;

	int16_t sample;

	int16_t fifo_buffer[fifo_buf_size];

	int16_t samples_buf[samples_buf_size] = {5, 2, 1, 5, 10, 14, 32, 65, 13, 18, -10, -25, -30, 13, 1, 1};
	int16_t filtered[output_length] = {2, 2, 5, 9, 18, 37, 36, 32, 7, -5, -21, -14, -5, 5};
	// int16_t samples_buf[samples_buf_size] = {15, 16, 14, 14, 15, 15, 16, 15, 17, 15, 15, 18, 13, 12, 18, 15};
	uint32_t sample_shift = 0;

	status = moving_avg_init(&average, FilterLowPass, fifo_buffer, window_size);
	FILTER_ASSERT(status);

	status = moving_avg_fill_buffer(&average, samples_buf, &sample);
	FILTER_ASSERT(status);

	sample_shift += average.window_size;

	uint16_t filtered_ptr = 0;
	//cout << "Filtered data: " << endl;
	assert(filtered[filtered_ptr++] == sample);

	for(uint16_t i=sample_shift; i<samples_buf_size; i++)
	{
		moving_avg_filter_sample(&average, samples_buf[i], &sample);
		assert(filtered[filtered_ptr++] == sample);
	}
}



static void test_rank_filter_simple_buffer(void)
{
	FilterStatus_t 	status;

	const uint32_t window_size = 3;
	const uint32_t rank = 2;
	const uint32_t buf_size = 16;

	const uint16_t expected_output_len = 14;
	const int16_t  expected_output[expected_output_len] = {44, 21, 21 ,14, 32, 65, 65, 65, 13, 11, 30, 30, 50, 50};
	int16_t buffer[buf_size] = {44, 2, 21, 5, 11, 14, 32, 65, 13, 11, -10, -25, 30, -40, 50, 1};

	uint16_t output_len;
	status = rank_filter_get_output_data_len(buf_size, window_size, &output_len);
	FILTER_ASSERT(status);

	assert(expected_output_len == output_len);

	int16_t out_data[output_len];
	status = rank_filter_filter_sequence(buffer, buf_size, window_size, rank, out_data, &output_len);

	for(unsigned int i=0; i<output_len; i++)
	{
		assert(out_data[i] == expected_output[i]);
	}
}


static void test_rank_filter_ring_buffer(void)
{
	FilterStatus_t 	status;
	RankFilter_t rank_filter;

	const uint32_t window_size = 3;
	const uint32_t rank = 2;
	const uint32_t buf_size = 16;

    const uint16_t expected_output_len = 14;
    const int16_t  expected_output[expected_output_len] = {44, 21, 21 ,14, 32, 65, 65, 65, 13, 11, 30, 30, 50, 50};

	int16_t buffer[buf_size] = {44, 2, 21, 5, 11, 14, 32, 65, 13, 11, -10, -25, 30, -40, 50, 1};

	status = rank_filter_init(&rank_filter, buffer, window_size, rank);
	FILTER_ASSERT(status);

	int16_t sample;

	status = rank_filter_fill_buffer(&rank_filter, buffer, &sample);
	FILTER_ASSERT(status);

	uint16_t expected_ptr = 0;
	assert(sample == expected_output[expected_ptr++]);

	for(unsigned int i=window_size; i<buf_size; i++)
	{
		status = rank_filter_filter_sample(&rank_filter, buffer[i], &sample);
		FILTER_ASSERT(status);

		assert(sample == expected_output[expected_ptr++]);
	}
}



int main() {
	cout << "Filters test" << endl; // prints !!!Hello World!!!

	cout << "\n***Testing moving average filter***" << endl;

	cout << "\nTesting moving average with simple buffer" << endl;
	test_moving_average_simple_buffer();
	cout << "Simple buffer successfully tested" << endl;

	cout << "\nTesting moving average with fifo" << endl;
	test_moving_average_ring_buffer_fifo();
	cout << "Ring buffer successfully tested" << endl;

	cout << "\n***Testing rank filter***" << endl;

	cout << "\nTesting rank filter with simple buffer" << endl;
	test_rank_filter_simple_buffer();
	cout << "Successfully tested simple buffer" << endl;

	cout << "\nTesting rank filter with ring buffer" << endl;
	test_rank_filter_ring_buffer();
	cout << "Successfully tested ring buffer" << endl;

	return 0;
}
