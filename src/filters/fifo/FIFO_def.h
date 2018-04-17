/*
 * FIFO_def.h
 *
 *  Created on: Feb 20, 2018
 *      Author: yuriy
 */

#ifndef FIFO_FIFO_DEF_H_
#define FIFO_FIFO_DEF_H_


#ifdef __cplusplus
extern "C" {
#endif

typedef enum tagFIFO_flags_t
{
  FIFO_NO_FLAGS = 0x00,
  FIFO_LOOP = 0x01
} FIFO_flags_t;


typedef enum tagFIFO_error_t
{
	FIFO_OK,
	FIFO_OVERFLOW,
	FIFO_UNDERFLOW,
	FIFO_NOT_INITIALIZED
}FIFO_error_t;


#ifdef __cplusplus
}
#endif

#endif /* FIFO_FIFO_DEF_H_ */
