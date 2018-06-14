#include <game.h>

#include <log.h>

void engine_loop(struct game *game);
void engine_poll_events(struct game *game);

void game_init(struct game *game, const char *title, size_t width,
               size_t height) {
    SDL_Init(SDL_INIT_EVERYTHING);

    game->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED, width, height,
                                    SDL_WINDOW_SHOWN);
    game->width = width;
    game->height = height;
    game->state = GAME_STATE_IDLE;
    game->fps = 0;
    game->reduce_lag = false;

    game->input_short.buffer = calloc(21, sizeof(char));
    game->input_short.entered = calloc(21, sizeof(char));
    game->input_short.buffer_sz = 0;
    game->input_short.entered_sz = 0;
    game->input_short.max_sz = 20;

    game->input_long.buffer = calloc(81, sizeof(char));
    game->input_long.entered = calloc(81, sizeof(char));
    game->input_long.buffer_sz = 0;
    game->input_long.entered_sz = 0;
    game->input_long.max_sz = 80;

    game->mouse_x = 0;
    game->mouse_y = 0;
    game->mouse_click = 0;

    game_set_rate(game, 20);
}

void game_deinit(struct game *game) {
    if (game->input_short.buffer != NULL) {
        free(game->input_short.buffer);
        free(game->input_short.entered);
        game->input_short.buffer = NULL;
        game->input_short.entered = NULL;
    }
    if (game->input_long.buffer != NULL) {
        free(game->input_long.buffer);
        free(game->input_long.entered);
        game->input_long.buffer = NULL;
        game->input_short.entered = NULL;
    }
    if (game->window != NULL) {
        SDL_DestroyWindow(game->window);
        game->window = NULL;
    }
    SDL_Quit();
}

void game_start(struct game *game) {
    log_info("starting application");

    if (game->state == GAME_STATE_IDLE) {
        game->state = GAME_STATE_RUNNING;
        engine_loop(game);
    }

    game_release_resources(game);
}

void game_stop(struct game *game) {
    if (game->state == GAME_STATE_RUNNING) {
        game->state = GAME_STATE_CLOSING;
    }
}

void game_set_rate(struct game *game, uint8_t rate) {
    log_info("frame rate set to %u", rate);
    game->period = (1000 / rate);
}

void engine_loop(struct game *game) {
    game_load_resources(game);

    uint8_t ptr = 0;
    uint16_t step = 256;
    uint32_t delta = 1;
    uint32_t elapsed = 0;
    uint16_t lag_accum = 0;
    uint64_t ticks[10];

    while (game->state == GAME_STATE_RUNNING) {
        uint16_t prev_step = step;
        uint32_t prev_delta = delta;

        step = 256;
        delta = 1;

        uint64_t tick = SDL_GetTicks();

        if (ticks[ptr] == 0) {
            step = prev_step;
            delta = prev_delta;
        } else {
            step = (2560 * game->period / (tick - ticks[ptr]));
        }

        if (step < 25) {
            step = 25;
        } else if (step > 256) {
            step = 256;
            delta = (game->period - (tick - ticks[ptr]) / 10);

            if (delta == 0) {
                delta = 1;
            }
        }

        SDL_Delay(delta);

        ticks[ptr] = tick;
        ptr = (ptr + 1) % 10;

        if (delta > 1) {
            for (size_t i = 0; i < 10; i++) {
                if (ticks[i] != 0) {
                    ticks[i] += delta;
                }
            }
        }

        engine_poll_events(game);

        uint16_t updates = 0;

        while (elapsed < 256) {
            game_update(game);

            elapsed += step;
            updates += 1;

            if (updates > 1000) {
                elapsed = 0;
                lag_accum += 6;

                if (lag_accum > 25) {
                    lag_accum = 0;
                    game->reduce_lag = true;
                }
            }
        }

        if (lag_accum > 0) {
            lag_accum -= 1;
        }
        elapsed &= 255;

        if (game->period > 0) {
            game->fps = (1001 * step) / (game->period * 256);
        }

        game_render(game);

        game->mouse_click = 0;
    }
}

void engine_poll_events(struct game *game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            log_warn("received quit signal");
            game_stop(game);
            break;
        case SDL_MOUSEMOTION:
            game->mouse_x = event.motion.x;
            game->mouse_y = event.motion.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                game->mouse_click |= GAME_MOUSE_LEFT;
            } else if (event.button.button == SDL_BUTTON_RIGHT) {
                game->mouse_click |= GAME_MOUSE_RIGHT;
            }
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_F1) {
                game->reduce_lag = !game->reduce_lag;
            } else if (event.key.keysym.sym == SDLK_RETURN) {
                memset(game->input_short.entered, 0, game->input_short.max_sz);
                memset(game->input_long.entered, 0, game->input_long.max_sz);

                strncpy(game->input_short.entered, game->input_short.buffer,
                        game->input_short.buffer_sz);
                strncpy(game->input_long.entered, game->input_long.buffer,
                        game->input_long.buffer_sz);

                game->input_short.entered_sz = game->input_short.buffer_sz;
                game->input_short.entered_sz = game->input_short.buffer_sz;
                game->input_short.buffer_sz = 0;
                game->input_short.buffer_sz = 0;
                log_info("set entered text");
            } else {
            }
            break;
        }
    }
}
