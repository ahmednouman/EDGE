#include "circular_buffer.h"


void fifoInsert(circ_buf* cbuf, uint16_t data) {
    cbuf->data[cbuf->head++] = data;
    cbuf->head &= (CIRCULAR_BUF_SIZE - 1);

    if (cbuf->head == cbuf->tail) {
        cbuf->tail++;
        cbuf->tail &= (CIRCULAR_BUF_SIZE - 1);
    }
}

uint16_t fifoLook(circ_buf* cbuf, uint16_t n) {
    uint16_t look_addr = cbuf->tail + n;
    look_addr &= (CIRCULAR_BUF_SIZE - 1);
    return cbuf->data[look_addr];
}

uint8_t fifoIn(circ_buf* cbuf) {
    return (cbuf->head - cbuf->tail) & (CIRCULAR_BUF_SIZE - 1);
}

void fifoInc(circ_buf* cbuf, uint16_t n) {
    int temp;

    if (n >= fifoIn(cbuf)) {
        temp = cbuf->tail + fifoIn(cbuf);
    } else {
        temp = cbuf->tail + n;
        temp &= (CIRCULAR_BUF_SIZE - 1);
    }
    cbuf->tail = temp;
}

void fifoPop(circ_buf* cbuf, uint16_t* dest_buf, uint16_t n) {
    uint16_t available_data = fifoIn(cbuf);
    if (n > available_data) {
        n = available_data;
    }

    for (uint16_t i = 0; i < n; i++) {
        dest_buf[i] = fifoLook(cbuf, i);
    }

    fifoInc(cbuf, n);
}

void lifoPush(circ_buf* cbuf, uint16_t data) {
    cbuf->data[cbuf->head++] = data;
    cbuf->head &= (CIRCULAR_BUF_SIZE - 1);

    if (cbuf->head == cbuf->tail) {
        cbuf->tail++;
        cbuf->tail &= (CIRCULAR_BUF_SIZE - 1);
    }
}

bool lifoPop(circ_buf* cbuf, uint16_t* dest_buf, uint16_t n) {
    uint16_t available_data = (cbuf->head - cbuf->tail) & (CIRCULAR_BUF_SIZE - 1);

    if (available_data == 0) {
        return false;
    }

    if (n > available_data) {
        n = available_data;
    }

    for (uint16_t i = 0; i < n; i++) {
        cbuf->head--;
        cbuf->head &= (CIRCULAR_BUF_SIZE - 1);
        dest_buf[i] = cbuf->data[cbuf->head];
    }

    return true;
}

uint16_t lifoPeek(circ_buf* cbuf, uint16_t n) {
    uint16_t peek_addr = (cbuf->head - 1 - n) & (CIRCULAR_BUF_SIZE - 1);
    return cbuf->data[peek_addr];
}


