#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include "gui/font.h"
#include "gui/assets.h"
#include "gui/enemies.h"
#include "weapon.h"

static void game_update(GameState* game_state) {
    // update position
    if(game_state->up && game_state->player.position.y > HERO_MIN_Y) {
        game_state->player.position.y--;
    } else if(game_state->down && game_state->player.position.y < HERO_MAX_Y) {
        game_state->player.position.y++;
    }

    // update bullets positions
    weapon_update(game_state);

    if(game_state->fire) {
        weapon_try_fire_bullet(game_state);
    }
}

// draw here ui
static void draw_callback(Canvas* canvas, void* ctx) {
    GameState* game_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(game_state == NULL) {
        return;
    }

    render_clear(game_state->level.isInvertedColor);

    // life
    for(int i = 0; i < game_state->player.life; i++) {
        draw_ui_asset(i * 6 + 1, 1, ui_heart);
    }
    // rockets
    draw_ui_asset(50, 1, ui_rocket);
    draw_number(58, 1, game_state->player.rockets, 2);
    // score
    draw_number(90, 1, game_state->player.score, 5);
    // player
    draw_ui_asset(game_state->player.position.x, game_state->player.position.y, ui_hero);
    // bullets
    for(int i = 0; i < BULLET_PULL; i++) {
        if(game_state->player.bullets[i].x >= BULLET_X)
            draw_ui_asset(
                game_state->player.bullets[i].x, game_state->player.bullets[i].y, ui_bullet);
    }

    // enemies
    draw_ui_asset(90, 30, enemy_1);

    render_draw(canvas);

    release_mutex((ValueMutex*)ctx, game_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeInput, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    GameEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

int32_t impact_game_main() {
    // Create event queue for 8 elements of GameEvent type size
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(GameEvent));

    // Create game state
    GameState* game_state = malloc(sizeof(GameState));
    // Init game with start values
    game_init(game_state);

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, game_state, sizeof(GameState))) {
        FURI_LOG_E(TAG, "cannot create mutex\r\n");
        free(game_state);
        return 255;
    }

    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Set timer
    FuriTimer* timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 30); //12

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    FURI_LOG_E(TAG, ">>> started\r\n");
    GameEvent event;
    // Game loop
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        GameState* game_state = (GameState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            // Game update event
            if(event.type == EventTypeTick) {
                game_update(game_state);
            }

            // Key press events
            if(event.type == EventTypeInput) {
                if(event.input.type == InputTypePress) {
                    if(event.input.key == InputKeyOk) {
                        game_state->fire = true;
                    } else if(event.input.key == InputKeyUp) {
                        game_state->up = true;
                    } else if(event.input.key == InputKeyDown) {
                        game_state->down = true;
                    } else if(event.input.key == InputKeyRight) {
                        game_state->fire = true;
                    } else if(event.input.key == InputKeyLeft) {
                        game_state->rocket = true;
                    }
                }
                if(event.input.type == InputTypeRelease) {
                    if(event.input.key == InputKeyOk) {
                        game_state->fire = false;
                    } else if(event.input.key == InputKeyUp) {
                        game_state->up = false;
                    } else if(event.input.key == InputKeyDown) {
                        game_state->down = false;
                    } else if(event.input.key == InputKeyRight) {
                        game_state->fire = false;
                    } else if(event.input.key == InputKeyLeft) {
                        game_state->rocket = false;
                    }
                }
                if(event.input.key == InputKeyBack) {
                    processing = false;
                }
            }
        } else {
            FURI_LOG_D(TAG, "osMessageQueue: event timeout");
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, game_state);
    }

    // Clear resources
    furi_timer_free(timer);
    furi_message_queue_free(event_queue);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);

    return 0;
}