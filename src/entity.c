#include <buffer.h>
#include <entity.h>

#include <globals.h>
#include <log.h>

size_t entity_bubble_cnt = 0;
struct entity_bubble entity_bubbles[ENTITY_MAX_BUBBLE_CNT];

void entity_bubble_add(size_t x, size_t y, size_t type) {
    if (entity_bubble_cnt >= ENTITY_MAX_BUBBLE_CNT) {
        return;
    }

    struct entity_bubble *b = &entity_bubbles[entity_bubble_cnt];
    b->x = x;
    b->y = y;
    b->type = type;
    b->alive = 0;

    entity_bubble_cnt += 1;
}

void entity_bubble_render(void) {
    for (size_t i = 0; i < entity_bubble_cnt; i++) {
        struct entity_bubble *b = &entity_bubbles[i];

        if (b->type == 0) {
            surface_circle_fill(g_screen, b->x, b->y, 20 + (b->alive * 2),
                                255 + (b->alive * 5 * 256),
                                255 - (b->alive * 5));
        } else {
            surface_circle_fill(g_screen, b->x, b->y, 10 + b->alive,
                                0xFF0000 + (b->alive * 5 * 256),
                                255 - (b->alive * 5));
        }
    }
}

size_t jagex_logo_decode(struct surface *surface, struct jag_arc_entry *entry) {
    struct buffer buf;
    buffer_wrap(&buf, entry->data, entry->size);

    // skip the first 12 bytes
    buf.caret = 11;

    uint16_t width = 0, height = 0;

    buffer_get2(&buf, &width);
    buffer_get2(&buf, &height);

    uint8_t palette_r[256];
    uint8_t palette_g[256];
    uint8_t palette_b[256];

    for (size_t i = 0; i < 256; i++) {
        palette_r[i] = buf.data[(i * 3) + 20];
        palette_g[i] = buf.data[(i * 3) + 19];
        palette_b[i] = buf.data[(i * 3) + 18];
    }

    uint8_t *raster = malloc(sizeof(uint8_t) * (width * height));

    for (size_t y = height - 1; y-- > 0;) {
        for (size_t x = 0; x < width; x++) {
            raster[x + (y * width)] = buf.data[768 + x + (y * width)];
        }
    }

    size_t id = surface_sprite_define_rgb_raster(
        surface, raster, palette_r, palette_g, palette_b, width, height);

    free(raster);
    return id;
}
