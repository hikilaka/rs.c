#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <log.h>
#include <surface.h>

const size_t glyph_tbl[256] = {
    666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666,
    666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666,
    666, 666, 846, 558, 567, 756, 585, 594, 612, 738, 630, 639, 621, 675, 774,
    648, 792, 810, 468, 477, 486, 495, 504, 513, 522, 531, 540, 549, 729, 720,
    783, 666, 801, 819, 747, 0,   9,   18,  27,  36,  45,  54,  63,  72,  81,
    90,  99,  108, 117, 126, 135, 144, 153, 162, 171, 180, 189, 198, 207, 216,
    225, 684, 828, 702, 603, 657, 666, 234, 243, 252, 261, 270, 279, 288, 297,
    306, 315, 324, 333, 342, 351, 360, 369, 378, 387, 396, 405, 414, 423, 432,
    441, 450, 459, 693, 837, 711, 765, 666, 666, 666, 666, 666, 666, 666, 666,
    666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666,
    666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 576, 666,
    666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666,
    666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666,
    666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666,
    666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666,
    666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666,
    666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666, 666,
    666};

static void sprite_plot(uint32_t *out, uint32_t *in, size_t out_ptr,
                        size_t in_ptr, size_t out_ptr_step, size_t in_ptr_step,
                        int32_t w, int32_t h, uint8_t step);

static void char_plot(struct surface *surface, int32_t idx, size_t x, size_t y,
                      uint32_t color, uint8_t *glyphs);

void surface_init(struct surface *surface, struct game *game,
                  size_t sprite_cnt) {
    surface->game = game;
    surface->renderer =
        SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);

    if (surface->renderer == NULL) {
        log_warn("error creating renderer: %s", SDL_GetError());
    }

    surface->texture = SDL_CreateTexture(
        surface->renderer, SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, game->width, game->height);

    if (surface->texture == NULL) {
        log_warn("error creating texture: %s", SDL_GetError());
    }

    surface->pitch = game->width * 4;

    surface->max_sprite_cnt = sprite_cnt;
    surface->sprite_cnt = 0;
    surface->sprites = malloc(sizeof(struct sprite) * sprite_cnt);

    surface->max_font_cnt = 50;
    surface->font_cnt = 0;
    surface->fonts = malloc(sizeof(uint8_t *) * 50);
    surface->text_shadows = true;

    surface_set_rect(surface, 0, 0, game->width, game->height);
    surface->max_w = game->width;
    surface->max_h = game->height;

    SDL_SetRenderDrawColor(surface->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
}

void surface_release(struct surface *surface) {
    if (surface->fonts != NULL) {
        for (size_t i = 0; i < surface->font_cnt; i++) {
            if (surface->fonts[i] != NULL) {
                free(surface->fonts[i]);
                surface->fonts[i] = NULL;
            }
        }
        free(surface->fonts);
        surface->fonts = NULL;
    }
    if (surface->sprites != NULL) {
        for (size_t i = 0; i < surface->sprite_cnt; i++) {
            if (surface->sprites[i].pixels != NULL) {
                free(surface->sprites[i].pixels);
                surface->sprites[i].pixels = NULL;
            }
        }
        free(surface->sprites);
        surface->sprites = NULL;
    }
    if (surface->texture != NULL) {
        SDL_DestroyTexture(surface->texture);
        surface->texture = NULL;
    }
    if (surface->renderer != NULL) {
        SDL_DestroyRenderer(surface->renderer);
        surface->renderer = NULL;
    }
}

void surface_sprite_release(struct surface *surface) {
    for (size_t i = 0; i < surface->sprite_cnt; i++) {
        if (surface->sprites[i].pixels != NULL) {
            free(surface->sprites[i].pixels);
            surface->sprites[i].pixels = NULL;
        }
    }
}

size_t surface_sprite_define_rgb_raster(struct surface *surface,
                                        uint8_t *raster, uint8_t *palette_r,
                                        uint8_t *palette_g, uint8_t *palette_b,
                                        size_t width, size_t height) {
    if (surface->sprite_cnt >= surface->max_sprite_cnt) {
        return surface->max_sprite_cnt;
    }

    size_t index = surface->sprite_cnt++;
    struct sprite *sprite = &surface->sprites[index];

    sprite->pixels = malloc(sizeof(uint32_t) * (width * height));
    sprite->width = width;
    sprite->height = height;
    sprite->trans_x = 0;
    sprite->trans_y = 0;

    for (size_t i = 0; i < (width * height); i++) {
        sprite->pixels[i] = (0xff << 24) | (palette_r[raster[i]] << 16) |
                            (palette_g[raster[i]] << 8) |
                            (palette_b[raster[i]]);
    }

    return index;
}

size_t surface_font_define(struct surface *surface, uint8_t *font,
                           size_t size) {
    if (surface->font_cnt >= surface->max_font_cnt) {
        return surface->max_font_cnt;
    }

    size_t index = surface->font_cnt;

    surface->fonts[index] = malloc(sizeof(uint8_t) * size);
    memcpy(surface->fonts[index], font, size);
    surface->font_cnt += 1;
    return index;
}

void surface_set_rect(struct surface *surface, size_t o_x, size_t o_y,
                      size_t t_x, size_t t_y) {
    if (t_x > surface->game->width) {
        t_x = surface->game->width;
    }
    if (t_y > surface->game->height) {
        t_y = surface->game->height;
    }
    surface->origin_x = o_x;
    surface->origin_y = o_y;
    surface->top_x = t_x;
    surface->top_y = t_y;
}

void surface_reset_rect(struct surface *surface) {
    surface->origin_x = 0;
    surface->origin_y = 0;
    surface->top_x = 0;
    surface->top_y = 0;
}

void surface_render_begin(struct surface *surface) {
    SDL_LockTexture(surface->texture, NULL, (void **)&surface->pixels,
                    (int *)&surface->pitch);
}

void surface_render_end(struct surface *surface) {
    SDL_UnlockTexture(surface->texture);
    SDL_RenderClear(surface->renderer);
    SDL_RenderCopy(surface->renderer, surface->texture, NULL, NULL);
    SDL_RenderPresent(surface->renderer);
}

void surface_clear(struct surface *surface) {
    size_t step = 1;

    if (surface->game->reduce_lag) {
        step = 2;
    }

    size_t ptr = 0;

    for (size_t y = 0; y < surface->game->height; y += step) {
        for (size_t x = 0; x < surface->game->width; x++) {
            surface->pixels[ptr++] = SURFACE_BLACK;
        }
    }
}

void surface_text_draw(struct surface *surface, const char *text, size_t x,
                       size_t y, size_t font, uint32_t color) {
    if (font >= surface->font_cnt) {
        return;
    }

    uint8_t *glyphs = surface->fonts[font];
    size_t text_len = strlen(text);

    for (size_t i = 0; i < text_len; i++) {
        if (text[i] == '@' && (text_len - i) >= 4 && text[i + 4] == '@') {
#define set_col_if(str, col)                                                   \
    if (strncmp(&text[i + 1], str, 3) == 0) {                                  \
        color = (col);                                                         \
    }

            set_col_if("red", SURFACE_RED);
            set_col_if("lre", 0xFF9040);
            set_col_if("yel", SURFACE_YELLOW);
            set_col_if("gre", SURFACE_GREEN);
            set_col_if("blu", SURFACE_BLUE);
            set_col_if("cya", SURFACE_CYAN);
            set_col_if("mag", SURFACE_MAGENTA);
            set_col_if("whi", SURFACE_WHITE);
            set_col_if("bla", SURFACE_BLACK);
            set_col_if("dre", 0xC00000);
            set_col_if("ora", 0xFF9040);
            set_col_if("ran", rand() * 0xFFFFFF);
            set_col_if("or1", 0xFFB000);
            set_col_if("or2", 0xFF7000);
            set_col_if("or3", 0xFF3000);
            set_col_if("gr1", 0xC0FF00);
            set_col_if("gr2", 0x80FF00);
            set_col_if("gr3", 0x40FF00);

            i += 4;
#undef set_col_if
        } else { // TODO implement offsets with ~
            int32_t idx = glyph_tbl[(size_t)text[i]];

            if (surface->text_shadows && color != SURFACE_BLACK) {
                char_plot(surface, idx, x + 1, y, SURFACE_BLACK, glyphs);
                char_plot(surface, idx, x, y + 1, SURFACE_BLACK, glyphs);
            }

            char_plot(surface, idx, x, y, color, glyphs);
            x += glyphs[idx + 7];
        }
    }
}

void surface_text_draw_cent(struct surface *surface, const char *text, size_t x,
                            size_t y, size_t font, uint32_t color) {
    surface_text_draw(surface, text,
                      x - (surface_text_width(surface, text, font) / 2), y,
                      font, color);
}

size_t surface_text_width(struct surface *surface, const char *text,
                          size_t font) {
    size_t result = 0;
    size_t text_len = strlen(text);
    uint8_t *glyphs = surface->fonts[font];

    for (size_t i = 0; i < text_len; i++) {
        if (text[i] == '@' && (text_len - i) >= 4 && text[i + 4] == '@') {
            i += 4;
        } else { // TODO implement offsets with ~
            result += glyphs[glyph_tbl[(size_t)text[i]] + 7];
        }
    }

    return result;
}

size_t surface_text_height(struct surface *surface, size_t font) {
    return font == 0 ? (surface->fonts[font][8] - 2)
                     : (surface->fonts[font][8] - 1);
}

void surface_sprite_plot(struct surface *surface, size_t x, size_t y,
                         size_t id) {
    if (id >= surface->max_sprite_cnt) {
        return;
    }

    struct sprite *sprite = &surface->sprites[id];

    if (sprite->pixels == NULL) {
        return;
    }

    int32_t w = (int32_t)sprite->width;
    int32_t h = (int32_t)sprite->height;

    int32_t ptr = x + (y * surface->game->width);
    int32_t s_ptr = 0;
    int32_t ptr_step = surface->game->width - w;
    int32_t s_ptr_step = 0;

    if (y < surface->origin_y) {
        int32_t dy = (int32_t)surface->origin_y - y;
        h -= dy;
        y = (int32_t)surface->origin_y;
        ptr += dy * surface->game->width;
        s_ptr += dy * w;
    }
    if (y + h > surface->top_y) {
        h -= y + h - surface->top_y - 1;
    }
    if (x < surface->origin_x) {
        int32_t dx = (int32_t)surface->origin_x - x;
        w -= dx;
        x = (int32_t)surface->origin_x;
        ptr += dx;
        s_ptr += dx;
        ptr_step += dx;
        s_ptr_step += dx;
    }
    if (x + w > surface->top_x) {
        int32_t dx = x + w - (int32_t)surface->top_x + 1;
        w -= dx;
        ptr_step += dx;
        s_ptr_step += dx;
    }
    if (w <= 0 || h <= 0) {
        return;
    }
    uint8_t step = 1;

    if (surface->game->reduce_lag) {
        step = 2;
        ptr_step += surface->game->width;
        s_ptr_step += sprite->width;

        if ((y & 1) != 0) {
            ptr += surface->game->width;
            h -= 1;
        }
    }

    sprite_plot(surface->pixels, sprite->pixels, ptr, s_ptr, ptr_step,
                s_ptr_step, w, h, step);
}

void surface_circle_fill(struct surface *surface, size_t x, size_t y,
                         size_t radius, uint32_t color, uint8_t opacity) {
    uint8_t a = 255 - opacity;
    uint32_t r = (color >> 16 & 0xff) * opacity;
    uint32_t g = (color >> 8 & 0xff) * opacity;
    uint32_t b = (color & 0xff) * opacity;

    size_t start_y = 0;
    size_t end_y = y + radius;
    if (y > radius) {
        start_y = y - radius;
    }
    if (end_y >= surface->game->height) {
        end_y = surface->game->height - 1;
    }

    uint8_t step = 1;

    if (surface->game->reduce_lag) {
        step = 2;
        if ((start_y & 1) != 0) {
            start_y += 1;
        }
    }

    for (size_t dy = start_y; dy < end_y; dy += step) {
        size_t dy2 = dy - y;
        size_t len = (size_t)sqrt(radius * radius - dy2 * dy2);
        size_t start_x = 0;
        size_t end_x = x + len;
        if (x > len) {
            start_x = x - len;
        }
        if (end_x >= surface->game->width) {
            end_x = surface->game->width - 1;
        }

        size_t ptr = start_x + (dy * surface->game->width);

        for (size_t dx = start_x; dx <= end_x; dx++) {
            uint32_t cr = (surface->pixels[ptr] >> 16 & 0xff) * a;
            uint32_t cg = (surface->pixels[ptr] >> 8 & 0xff) * a;
            uint32_t cb = (surface->pixels[ptr] & 0xff) * a;
            surface->pixels[ptr++] = (0xff << 24) | ((r + cr) >> 8 << 16) |
                                     ((g + cg) >> 8 << 8) | ((b + cb) >> 8);
        }
    }
}

static void sprite_plot(uint32_t *out, uint32_t *in, size_t out_ptr,
                        size_t in_ptr, size_t out_ptr_step, size_t in_ptr_step,
                        int32_t w, int32_t h, uint8_t step) {
    int32_t blocks = -(w >> 2);
    w = -(w & 0x3);
    uint32_t i;

#define transfer                                                               \
    i = in[in_ptr++];                                                          \
    if (i != 0) {                                                              \
        out[out_ptr++] = (0xff << 24) | i;                                     \
    } else {                                                                   \
        out_ptr++;                                                             \
    }

    for (int32_t y = -h; y < 0; y += step) {
        for (int32_t x = blocks; x < 0; x++) {
            transfer;
            transfer;
            transfer;
            transfer;
        }
        for (int32_t x = w; x < 0; x++) {
            transfer;
        }
        out_ptr += out_ptr_step;
        in_ptr += in_ptr_step;
    }

#undef transfer
}

static void transfer_char(uint32_t *out, uint8_t *glyphs, uint32_t color,
                          size_t out_ptr, size_t in_ptr, int32_t w, int32_t h,
                          size_t out_step, size_t in_step) {
    int32_t blocks = -(w >> 2);
    w = -(w & 3);

#define transfer                                                               \
    if (glyphs[in_ptr++] != SURFACE_BLACK) {                                   \
        out[out_ptr++] = color;                                                \
    } else {                                                                   \
        out_ptr++;                                                             \
    }

    for (int32_t y = -h; y < 0; y++) {
        for (int32_t x = blocks; x < 0; x++) {
            transfer;
            transfer;
            transfer;
            transfer;
        }

        for (int32_t x = w; x < 0; x++) {
            transfer;
        }

        out_ptr += out_step;
        in_ptr += in_step;
    }
#undef transfer
}

static void char_plot(struct surface *surface, int32_t idx, size_t x, size_t y,
                      uint32_t color, uint8_t *glyphs) {
    int32_t start_x = x + glyphs[idx + 5];
    int32_t start_y = y - glyphs[idx + 6];
    int32_t w = glyphs[idx + 3];
    int32_t h = glyphs[idx + 4];
    size_t in_ptr =
        glyphs[idx] * 16384 + glyphs[idx + 1] * 128 + glyphs[idx + 2];
    size_t out_ptr = start_x + start_y * surface->game->width;
    size_t out_step = surface->game->width - w;
    size_t in_step = 0;

    if (start_y < (int32_t)surface->origin_y) {
        size_t dy = surface->origin_y - start_y;
        h -= dy;
        start_y = surface->origin_y;
        in_ptr += dy * w;
        out_ptr += dy * surface->game->width;
    }
    if (start_y + h >= (int32_t)surface->top_y) {
        h -= (start_y + h) - surface->top_y - 1;
    }
    if (start_x < (int32_t)surface->origin_x) {
        size_t dx = surface->origin_x - start_x;
        w -= dx;
        start_x = surface->origin_x;
        in_ptr += dx;
        out_ptr += dx;
        in_step += dx;
        out_ptr += dx;
    }
    if (start_x + w >= (int32_t)surface->top_x) {
        size_t dx = (start_x + w) - surface->top_x + 1;
        w -= dx;
        in_step += dx;
        out_step += dx;
    }

    if (w > 0 && h > 0) {
        transfer_char(surface->pixels, glyphs, color, out_ptr, in_ptr, w, h,
                      out_step, in_step);
    }
}
