#ifndef ENTITY_H
#define ENTITY_H

#pragma once

#include <jag_arc.h>
#include <surface.h>

struct entity_bubble {
    size_t x;
    size_t y;
    size_t type;
    size_t alive;
};

#define ENTITY_MAX_BUBBLE_CNT 50

extern size_t entity_bubble_cnt;
extern struct entity_bubble entity_bubbles[ENTITY_MAX_BUBBLE_CNT];

size_t jagex_logo_decode(struct surface *surface, struct jag_arc_entry *entry);

void entity_bubble_add(size_t x, size_t y, size_t type);

void entity_bubble_render(void);

#endif // ENTITY_H
