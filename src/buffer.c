#include <buffer.h>

void buffer_wrap(struct buffer *buf, uint8_t *data, size_t size) {
    buf->data = data;
    buf->size = size;
    buf->caret = 0;
}

int buffer_get(struct buffer *buf, uint8_t *out) {
    if (buf->caret >= buf->size) {
        return -1;
    }
    *out = buf->data[buf->caret];
    buf->caret += 1;
    return 0;
}

int buffer_get2(struct buffer *buf, uint16_t *out) {
    if (buf->caret + 1 >= buf->size) {
        return -1;
    }
    *out |= ((buf->data[buf->caret] & 0xff) << 8);
    *out |= (buf->data[buf->caret + 1] & 0xff);
    buf->caret += 2;
    return 0;
}

int buffer_get2le(struct buffer *buf, uint16_t *out) {
    if (buf->caret + 1 >= buf->size) {
        return -1;
    }
    *out |= ((buf->data[buf->caret + 1] & 0xff) << 8);
    *out |= (buf->data[buf->caret] & 0xff);
    buf->caret += 2;
    return 0;
}

int buffer_get3(struct buffer *buf, uint32_t *out) {
    if (buf->caret + 2 >= buf->size) {
        return -1;
    }
    *out |= ((buf->data[buf->caret] & 0xff) << 16);
    *out |= ((buf->data[buf->caret + 1] & 0xff) << 8);
    *out |= (buf->data[buf->caret + 2] & 0xff);
    buf->caret += 3;
    return 0;
}

int buffer_get4(struct buffer *buf, uint32_t *out) {
    if (buf->caret + 3 >= buf->size) {
        return -1;
    }
    *out |= ((buf->data[buf->caret] & 0xff) << 24);
    *out |= ((buf->data[buf->caret + 1] & 0xff) << 16);
    *out |= ((buf->data[buf->caret + 2] & 0xff) << 8);
    *out |= (buf->data[buf->caret + 3] & 0xff);
    buf->caret += 4;
    return 0;
}

int buffer_get8(struct buffer *buf, uint64_t *out) {
    int error = buffer_get4(buf, (uint32_t *)out);
    *out <<= 32;
    error |= buffer_get4(buf, (uint32_t *)out);
    return error;
}
