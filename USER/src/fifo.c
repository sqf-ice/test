#include <stdint.h>
#include <stddef.h>

#include "fifo.h"

fifo_result_t fifo_init(fifo_t * fifo, uint8_t * buffer, uint16_t size)
{
    if (NULL != fifo && NULL != buffer && 0 < size) {
        fifo->buffer = buffer;
        fifo->size = size;
        fifo->first = 0;
        fifo->elements_n = 0;

        return FIFO_SUCCESS;
    } else {
        return FIFO_ERROR;
    }
}

fifo_result_t fifo_push(fifo_t * fifo, uint8_t data)
{
    if (NULL != fifo && fifo->size != fifo->elements_n) {
        fifo->buffer[(fifo->first + fifo->elements_n) % fifo->size] = data;
        ++fifo->elements_n;
        return FIFO_SUCCESS;
    } else {
        return FIFO_ERROR;
    }
}

fifo_result_t fifo_push_multiple(fifo_t * fifo, uint8_t * data, uint16_t size)
{
    if (NULL != fifo && NULL != data && fifo->elements_n + size <= fifo->size) {
        while (size--) {
            fifo->buffer[(fifo->first + fifo->elements_n) % fifo->size] =
                *data++;
            ++fifo->elements_n;
        }
        return FIFO_SUCCESS;
    } else {
        return FIFO_ERROR;
    }
}

fifo_result_t fifo_pop(fifo_t * fifo, uint8_t * data)
{
    if (NULL != fifo && 0 != fifo->elements_n) {
        *data = fifo->buffer[fifo->first];
        fifo->first = (fifo->first + 1) % fifo->size;
        --fifo->elements_n;

        return FIFO_SUCCESS;
    } else {
        return FIFO_ERROR;
    }
}

fifo_result_t fifo_pop_multiple(fifo_t * fifo, uint8_t * data, uint16_t size)
{
    if (NULL != fifo && NULL != data && fifo->elements_n >= size) 
	{
        while (size--) 
		{
            *data++ = fifo->buffer[fifo->first];
            fifo->first = (fifo->first + 1) % fifo->size;
            --fifo->elements_n;
        }
        return FIFO_SUCCESS;
    } 
	else 
	{
        return FIFO_ERROR;
    }
}

fifo_result_t fifo_is_empty(fifo_t * fifo)
{
    if (NULL != fifo) {
        if (0 == fifo->elements_n) {
            return FIFO_TRUE;
        } else {
            return FIFO_FALSE;
        }
    } else {
        return FIFO_ERROR;
    }
}

fifo_result_t fifo_is_full(fifo_t * fifo)
{
    if (NULL != fifo) {
        if (fifo->size == fifo->elements_n) {
            return FIFO_TRUE;
        } else {
            return FIFO_FALSE;
        }
    } else {
        return FIFO_ERROR;
    }
}

__inline uint16_t fifo_count_elements(fifo_t * fifo)
{
    return fifo->elements_n;
}

fifo_result_t fifo_find(fifo_t * fifo, uint8_t data)
{
	uint16_t i ;
    if (NULL != fifo) 
	{
        for ( i = 0; fifo_count_elements(fifo) > i; ++i) 
		{
            if (data == fifo->buffer[(fifo->first + i) % fifo->size]) {
                return FIFO_TRUE;
            }
        }

        return FIFO_FALSE;
    } 
	else 
	{
        return FIFO_ERROR;
    }
}

fifo_result_t fifo_search(fifo_t * fifo, uint8_t * pattern,
                          uint16_t pattern_size, uint16_t * position)
{
    uint16_t i = 0;
    uint16_t j = 0;

    if (NULL != fifo && NULL != pattern && 0 < pattern_size
        && fifo->elements_n >= pattern_size) {
        while (i < fifo->elements_n && j < pattern_size) {
            if (*(pattern + j) == fifo->buffer[(fifo->first + i) % fifo->size]) {
                i++;
                j++;
            } else {
                i++;
                j = 0;
            }
        }
        if (pattern_size == j) {
            if (NULL != position) {
                *position = (fifo->first + i - j) % fifo->size;
            }
            return FIFO_TRUE;
        } else {
            return FIFO_FALSE;
        }
    } else {
        return FIFO_ERROR;
    }
}
