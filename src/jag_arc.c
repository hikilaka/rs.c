#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <bzlib.h>

#include <buffer.h>
#include <jag_arc.h>
#include <log.h>

static uint8_t *decompress(uint8_t *buffer, size_t c_size, size_t d_size);

static int read_entries(struct jag_arc *arc, uint8_t *buffer, size_t size);

int jag_arc_load(struct jag_arc *arc, const char *file) {
    arc->entry_cnt = 0;
    arc->entries = NULL;

    FILE *fp = fopen(file, "rb");

    if (fp == NULL) {
        return -1;
    }

    uint8_t header_buf[6];
    struct buffer header;

    fread(header_buf, 6, 1, fp);
    buffer_wrap(&header, header_buf, 6);

    uint32_t d_size, c_size;

    buffer_get3(&header, &d_size);
    buffer_get3(&header, &c_size);

    log_debug("%s d_size=%zu, c_size=%zu", file, d_size, c_size);

    uint8_t *arc_data = malloc(sizeof(uint8_t) * c_size);
    fread(arc_data, c_size, 1, fp);
    fclose(fp);

    if (d_size != c_size) {
        uint8_t *d_arc_data = decompress(arc_data, c_size, d_size);

        if (d_arc_data == NULL) {
            return -2;
        }

        free(arc_data);
        arc_data = d_arc_data;
    }

    int error = read_entries(arc, arc_data, d_size);

    free(arc_data);
    return error;
}

void jag_arc_unload(struct jag_arc *arc) {
    if (arc == NULL) {
        return;
    }

    for (size_t i = 0; i < arc->entry_cnt; i++) {
        free(arc->entries[i].data);
    }

    free(arc->entries);
}

struct jag_arc_entry *jag_arc_get(struct jag_arc *arc, const char *file) {
    uint32_t hash = 0;
    size_t file_len = strlen(file);

    for (size_t i = 0; i < file_len; i++) {
        hash = (hash * 61) + (toupper(file[i]) - 32);
    }

    for (size_t i = 0; i < arc->entry_cnt; i++) {
        if (arc->entries[i].hash == hash) {
            return &arc->entries[i];
        }
    }

    return NULL;
}

static int read_entries(struct jag_arc *arc, uint8_t *buffer, size_t size) {
    struct buffer arc_data;
    buffer_wrap(&arc_data, buffer, size);

    int error = buffer_get2(&arc_data, (uint16_t *)&arc->entry_cnt);

    arc->entries = malloc(sizeof(struct jag_arc_entry) * arc->entry_cnt);

    size_t data_ptr = (arc->entry_cnt * 10) + arc_data.caret;

    for (size_t i = 0; i < arc->entry_cnt; i++) {
        size_t hash = 0;
        size_t d_size = 0, c_size = 0;

        error |= buffer_get4(&arc_data, (uint32_t *)&hash);
        error |= buffer_get3(&arc_data, (uint32_t *)&d_size);
        error |= buffer_get3(&arc_data, (uint32_t *)&c_size);

        uint8_t *data = malloc(sizeof(uint8_t) * c_size);
        memcpy(data, &buffer[data_ptr], c_size);

        if (d_size != c_size) {
            uint8_t *d_data = decompress(data, c_size, d_size);

            if (d_data == NULL) {
                error = -2;
            }

            free(data);
            data = d_data;
        }

        arc->entries[i].data = data;
        arc->entries[i].size = d_size;
        arc->entries[i].hash = hash;

        log_debug("\t%zu - %zu bytes", hash, d_size);

        data_ptr += c_size;
    }
    return error;
}

static uint8_t *decompress(uint8_t *buffer, size_t c_size, size_t d_size) {
    uint8_t *d_buffer = malloc(sizeof(uint8_t) * d_size);
    size_t d_buffer_size = d_size;

    // requires 4 byte header "BZh1"
    uint8_t *buffer_cpy = malloc(sizeof(uint8_t) * (c_size + 4));
    memcpy(buffer_cpy, "BZh1", 4);
    memcpy(&buffer_cpy[4], buffer, c_size);

    int result =
        BZ2_bzBuffToBuffDecompress((char *)d_buffer, (unsigned *)&d_buffer_size,
                                   (char *)buffer_cpy, c_size + 4, 0, 0);

    free(buffer_cpy);

    if (result != BZ_OK) {
        log_warn("error decompressing bzip buffer: %d", result);
        free(d_buffer);
        return NULL;
    }

    return d_buffer;
}
