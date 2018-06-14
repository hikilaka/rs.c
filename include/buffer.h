#ifndef BUFFER_H
#define BUFFER_H

#pragma once

#include <stddef.h>
#include <stdint.h>

struct buffer {
    uint8_t *data;
    size_t size;
    size_t caret;
};

void buffer_wrap(struct buffer *buf, uint8_t *data, size_t size);

int buffer_get(struct buffer *buf, uint8_t *out);

int buffer_get2(struct buffer *buf, uint16_t *out);

int buffer_get2le(struct buffer *buf, uint16_t *out);

int buffer_get3(struct buffer *buf, uint32_t *out);

int buffer_get4(struct buffer *buf, uint32_t *out);

int buffer_get8(struct buffer *buf, uint64_t *out);

#endif // BUFFER_H
