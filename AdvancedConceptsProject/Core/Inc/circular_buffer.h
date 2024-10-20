#ifndef INC_CIRCULAR_BUFFER_H_
#define INC_CIRCULAR_BUFFER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define CIRCULAR_BUF_SIZE 16

typedef struct {
    volatile uint16_t data[CIRCULAR_BUF_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
} circ_buf;

void fifoInsert(circ_buf* cbuf, uint16_t data);
uint16_t fifoLook(circ_buf* cbuf, uint16_t n);
uint8_t fifoIn(circ_buf* cbuf);
void fifoInc(circ_buf* cbuf, uint16_t n);
void fifoPop(circ_buf* cbuf, uint16_t* dest_buf, uint16_t n);

void lifoPush(circ_buf* cbuf, uint16_t data);
bool lifoPop(circ_buf* cbuf, uint16_t* dest_buf, uint16_t n);
uint16_t lifoPeek(circ_buf* cbuf, uint16_t n);

#endif /* INC_CIRCULAR_BUFFER_H_ */
