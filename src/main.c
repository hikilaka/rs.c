#include <stdint.h>
#include <stdlib.h>

#include <entity.h>
#include <game.h>
#include <globals.h>
#include <jag_arc.h>
#include <log.h>
#include <model.h>
#include <surface.h>

static size_t sprite_jag_logo;

struct model cube;

void model_cube_init(struct model *model) {
    static size_t face_cnt = 12;
    static int16_t faces[12][3] = {{0, 6, 4}, {0, 2, 6}, {0, 3, 2}, {0, 1, 3},
				   {2, 7, 6}, {2, 3, 7}, {4, 6, 7}, {4, 7, 5},
				   {0, 4, 5}, {0, 5, 1}, {1, 5, 7}, {1, 7, 3}};

    int32_t front = 0;
    int32_t back = 0;

    model_init(model, 8, face_cnt);
    model_vert_add(model, -64, -64, -64);
    model_vert_add(model, -64, -64, 64);
    model_vert_add(model, -64, 64, -64);
    model_vert_add(model, -64, 64, 64);
    model_vert_add(model, 64, -64, -64);
    model_vert_add(model, 64, -64, 64);
    model_vert_add(model, 64, 64, -64);
    model_vert_add(model, 64, 64, 64);

    for (size_t i = 0; i < face_cnt; i++) {
	model_face_add(model, 3, faces[i], front, back);
    }
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
    sprite_jag_logo = entity_tga_decode(entry);

#define load_font(name)                                                        \
    do {                                                                       \
	entry = jag_arc_get(&jagex_arc, name);                                 \
	if (entry == NULL) {                                                   \
	    log_fatal("failed to load %s from jagex.jag", name);               \
	    return;                                                            \
	}                                                                      \
	surface_font_define(g_screen, entry->data, entry->size);               \
    } while (0);

    load_font("h11p.jf");
    load_font("h12b.jf");
    load_font("h12p.jf");
    load_font("h13b.jf");
    load_font("h14b.jf");
    load_font("h16b.jf");
    load_font("h20b.jf");
    load_font("h24b.jf");

#undef load_font

    jag_arc_unload(&jagex_arc);

    model_cube_init(&cube);

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

void game_release_resources(struct game *game) {
    (void)game;
    model_deinit(&cube);
    surface_release(g_screen);
}

uint8_t opacity = 0;

void game_update(struct game *game) {
    for (size_t i = 0; i < entity_bubble_cnt; i++) {
	struct entity_bubble *b = &entity_bubbles[i];

	b->alive += 1;

	if (b->alive >= 50) {
	    entity_bubble_cnt -= 1;

	    for (size_t j = i; j < entity_bubble_cnt; j++) {
		memcpy(&entity_bubbles[j], &entity_bubbles[j + 1],
		       sizeof(struct entity_bubble));
	    }
	}
    }

    if ((game->mouse_click & GAME_MOUSE_LEFT) != 0) {
	entity_bubble_add(game->mouse_x, game->mouse_y, 0);
	game->mouse_click &= ~GAME_MOUSE_LEFT;
    }
    if ((game->mouse_click & GAME_MOUSE_RIGHT) != 0) {
	entity_bubble_add(game->mouse_x, game->mouse_y, 1);
	game->mouse_click &= ~GAME_MOUSE_RIGHT;
    }
}

void game_render(struct game *game) {
    (void)game;
    surface_render_begin(g_screen);
    surface_clear(g_screen);

    surface_sprite_plot(g_screen, (game->width / 2) - 140, 100,
			sprite_jag_logo);

    surface_text_draw_cent(g_screen, "sucks", game->width / 2, 200,
			   SURFACE_FONT_XXL_BOLD, SURFACE_ORANGE);

    entity_bubble_render();

    draw_sys_info();
    surface_render_end(g_screen);
}

int main(void) {
    struct game game;
    struct surface surface;

    g_game = &game;
    g_screen = &surface;

    memset(&surface, 0, sizeof(struct surface));

    game_init(&game, "runescape classic", 512, 384);

    surface_init(&surface, &game, 10);

    game_start(&game);
    game_deinit(&game);
}
