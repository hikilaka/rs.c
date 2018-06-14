#ifndef JAG_ARC_H
#define JAG_ARC_H

#pragma once

#include <stddef.h>
#include <stdint.h>

struct jag_arc_entry {
    uint8_t *data;
    size_t size;
    uint32_t hash;
};

struct jag_arc {
    size_t entry_cnt;
    struct jag_arc_entry *entries;
};

int jag_arc_load(struct jag_arc *arc, const char *file);

void jag_arc_unload(struct jag_arc *arc);

struct jag_arc_entry *jag_arc_get(struct jag_arc *arc, const char *file);

#endif // JAG_ARC_H
