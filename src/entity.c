#include <buffer.h>
#include <entity.h>

#include <log.h>

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
