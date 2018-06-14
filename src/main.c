#include <stdint.h>
#include <stdlib.h>

#include <entity.h>
#include <game.h>
#include <globals.h>
#include <jag_arc.h>
#include <log.h>
#include <surface.h>

static size_t sprite_jag_logo;

struct bubble {
    size_t x;
    size_t y;
    size_t type;
    size_t alive;
};

#define MAX_BUBBLE_CNT 50
static size_t bubble_cnt = 0;
static struct bubble bubbles[MAX_BUBBLE_CNT];

void bubble_add(size_t x, size_t y, size_t type) {
    if (bubble_cnt >= MAX_BUBBLE_CNT) {
        return;
    }
    struct bubble *b = &bubbles[bubble_cnt++];
    b->x = x;
    b->y = y;
    b->type = type;
    b->alive = 0;
}

void game_load_resources(struct game *game) {
    struct jag_arc jagex_arc;
    int error = jag_arc_load(&jagex_arc, "release/jagex.jag");

    if (error) {
        log_fatal("failed to load jagex.jag");
        game_stop(game);
        return;
    }

    struct jag_arc_entry *entry = jag_arc_get(&jagex_arc, "logo.tga");
    sprite_jag_logo = jagex_logo_decode(g_screen, entry);

    entry = jag_arc_get(&jagex_arc, "h11p.jf");
    surface_font_define(g_screen, entry->data, entry->size);
    entry = jag_arc_get(&jagex_arc, "h12b.jf");
    surface_font_define(g_screen, entry->data, entry->size);
    entry = jag_arc_get(&jagex_arc, "h12p.jf");
    surface_font_define(g_screen, entry->data, entry->size);
    entry = jag_arc_get(&jagex_arc, "h13b.jf");
    surface_font_define(g_screen, entry->data, entry->size);
    entry = jag_arc_get(&jagex_arc, "h14b.jf");
    surface_font_define(g_screen, entry->data, entry->size);
    entry = jag_arc_get(&jagex_arc, "h16b.jf");
    surface_font_define(g_screen, entry->data, entry->size);
    entry = jag_arc_get(&jagex_arc, "h20b.jf");
    surface_font_define(g_screen, entry->data, entry->size);
    entry = jag_arc_get(&jagex_arc, "h24b.jf");
    surface_font_define(g_screen, entry->data, entry->size);

    jag_arc_unload(&jagex_arc);

    game_set_rate(game, 50);
}

void draw_sys_info(void) {
    char info_buf[80];
    snprintf(info_buf, 80, "fps: %u", g_game->fps);
    size_t y =
        g_game->height - surface_text_height(g_screen, SURFACE_FONT_BOLD) + 9;

    surface_text_draw(
        g_screen, info_buf,
        g_game->width -
            surface_text_width(g_screen, info_buf, SURFACE_FONT_BOLD) - 3,
        y, SURFACE_FONT_BOLD, SURFACE_YELLOW);

    snprintf(info_buf, 80, "mouse loc: (%zu, %zu)", g_game->mouse_x,
             g_game->mouse_y);
    y -= surface_text_height(g_screen, SURFACE_FONT_BOLD);
    surface_text_draw(
        g_screen, info_buf,
        g_game->width -
            surface_text_width(g_screen, info_buf, SURFACE_FONT_BOLD) - 3,
        y, SURFACE_FONT_BOLD, SURFACE_YELLOW);
}

void game_release_resources(struct game *game) { surface_release(g_screen); }

uint8_t opacity = 0;

void game_update(struct game *game) {
    for (size_t i = 0; i < bubble_cnt; i++) {
        struct bubble *b = &bubbles[i];

        b->alive += 1;

        if (b->alive >= 50) {
            bubble_cnt -= 1;

            for (size_t j = i; j < bubble_cnt; j++) {
                memcpy(&bubbles[j], &bubbles[j + 1], sizeof(struct bubble));
            }
        }
    }

    if ((game->mouse_click & GAME_MOUSE_LEFT) != 0) {
        bubble_add(game->mouse_x, game->mouse_y, 0);
    }
    if ((game->mouse_click & GAME_MOUSE_RIGHT) != 0) {
        bubble_add(game->mouse_x, game->mouse_y, 1);
    }
}

void game_render(struct game *game) {
    (void)game;
    surface_render_begin(g_screen);
    surface_clear(g_screen);

    // surface_sprite_plot(g_screen, 100, 100, sprite_jag_logo);

    for (size_t i = 0; i < bubble_cnt; i++) {
        struct bubble *b = &bubbles[i];

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

    size_t x = game->width / 2;
    size_t y = game->height / 4;
    const char *text = "hello @ran@w@ran@o@ran@r@ran@l@ran@d@ran@!";

    for (size_t font = 0; font < g_screen->font_cnt; font++) {
        surface_text_draw_cent(g_screen, text, x, y, font, SURFACE_GREEN);
        y += surface_text_height(g_screen, font);
    }

    draw_sys_info();
    surface_render_end(g_screen);
}

int main(void) {
    struct game game;
    struct surface surface;
    memset(&surface, 0, sizeof(struct surface));
    game_init(&game, "runescape classic", 512, 384);

    surface_init(&surface, &game, 10);

    g_game = &game;
    g_screen = &surface;

    game_start(&game);
    game_deinit(&game);
}
