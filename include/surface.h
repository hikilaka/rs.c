#ifndef SURFACE_H
#define SURFACE_H

#pragma once

#include <SDL_render.h>
#include <stdbool.h>
#include <stdint.h>

#include <game.h>

#define SURFACE_BLACK 0x000000
#define SURFACE_WHITE 0xFFFFFF
#define SURFACE_RED 0xFF0000
#define SURFACE_DARK_RED 0xBB0000
#define SURFACE_GREEN 0x00FF00
#define SURFACE_BLUE 0x0000FF
#define SURFACE_YELLOW 0xFFFF00
#define SURFACE_CYAN 0x00FFFF
#define SURFACE_MAGENTA 0xFF00FF
#define SURFACE_LIGHT_GRAY 0xC0C0C0
#define SURFACE_GRAY 0x808080
#define SURFACE_DARK_GRAY 0x404040
#define SURFACE_ORANGE 0xFF7700

#define SURFACE_FONT_REG 0
#define SURFACE_FONT_BOLD 1
#define SURFACE_FONT_M_BOLD 3
#define SURFACE_FONT_L_BOLD 4
#define SURFACE_FONT_XL_BOLD 5
#define SURFACE_FONT_XXL_BOLD 7

struct sprite {
    uint32_t *pixels;
    uint16_t width;
    uint16_t height;
    uint8_t trans_x;
    uint8_t trans_y;
};

struct surface {
    struct game *game;

    SDL_Renderer *renderer;
    SDL_Texture *texture;
    uint32_t *pixels;
    size_t pitch;

    size_t max_sprite_cnt;
    size_t sprite_cnt;
    struct sprite *sprites;

    size_t max_font_cnt;
    size_t font_cnt;
    uint8_t **fonts;
    bool text_shadows;

    size_t origin_x;
    size_t origin_y;
    size_t top_x;
    size_t top_y;
    size_t max_w;
    size_t max_h;
};

void surface_init(struct surface *surface, struct game *game,
		  size_t sprite_cnt);

void surface_release(struct surface *surface);

void surface_sprite_release(struct surface *surface);

size_t surface_sprite_define_rgb_raster(struct surface *surface,
					uint8_t *raster, uint8_t *palette_r,
					uint8_t *palette_g, uint8_t *palette_b,
					size_t width, size_t height);

size_t surface_font_define(struct surface *surface, uint8_t *font, size_t size);

void surface_set_rect(struct surface *surface, size_t o_x, size_t o_y,
		      size_t t_x, size_t t_y);

void surface_render_begin(struct surface *surface);

void surface_render_end(struct surface *surface);

void surface_clear(struct surface *surface);

void surface_text_draw(struct surface *surface, const char *text, size_t x,
		       size_t y, size_t font, uint32_t color);

void surface_text_draw_cent(struct surface *surface, const char *text, size_t x,
			    size_t y, size_t font, uint32_t color);

size_t surface_text_width(struct surface *surface, const char *text,
			  size_t font);

size_t surface_text_height(struct surface *surface, size_t font);

void surface_sprite_plot(struct surface *surface, size_t x, size_t y,
			 size_t id);

void surface_circle_fill(struct surface *surface, size_t x, size_t y,
			 size_t radius, uint32_t color, uint8_t opacity);

#endif // SURFACE_H
