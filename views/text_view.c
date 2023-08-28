#include <gui/gui.h>
#include "text_view.h"
#include "office.h"

#define animation_speed 1

static void animation(Canvas* canvas, Fnaf* fnaf) {
    if (fnaf->counter > animation_speed * 10) {
        fnaf->counter = 0;
        if (fnaf->progress > 6) SWITCH_VIEW(main_menu); else night_start(fnaf);
    } else {
        canvas_set_color(canvas, 1);
        uint8_t y = 35 - fnaf->counter / animation_speed;
        canvas_draw_str(canvas, 50, y, "5");
        canvas_draw_str(canvas, 50, y + 10, "6");
        canvas_set_color(canvas, 0);
        canvas_draw_box(canvas, 50, 37, 5, 9);
        canvas_draw_box(canvas, 50, 16, 5, 9);
        canvas_set_color(canvas, 1);

        fnaf->counter += 1;
    }
}

void text_view(Canvas* canvas, Fnaf* fnaf) {
    FURI_LOG_D(TAG, "text_view");
    switch (fnaf->current_view) {
    case night_number:
        if (fnaf->counter > 7) {
            FURI_LOG_D(TAG, "text_view is over");
            SWITCH_VIEW(office);
            furi_timer_start(fnaf->hourly_timer, first_hour_time);
            furi_timer_start(fnaf->electricity->timer, power_time);
        } else {
            FURI_LOG_D(TAG, "Night number, counter = %u", fnaf->counter);
            canvas_set_bitmap_mode(canvas, 1);
            canvas_set_color(canvas, 1);
            char messages[7][10] = { "1st Night", "2nd Night", "3rd Night", "4th Night", "5th Night", "6th Night", "7th Night" };
            canvas_set_font(canvas, FontPrimary);
            canvas_draw_str(canvas, 46, 28, "12 AM");
            canvas_draw_str(canvas, 41, 40, messages[fnaf->progress]);
            fnaf->counter += 1;
        }
        break;
    case night_complete:
        canvas_set_bitmap_mode(canvas, 1);
        canvas_set_color(canvas, 1);
        canvas_set_font(canvas, FontPrimary);
        canvas_draw_str(canvas, 58, 35, "AM");
        animation(canvas, fnaf);
        // canvas_draw_str(canvas, 50, 35, "6");
        break;
    case game_over:
        canvas_set_bitmap_mode(canvas, 1);
        canvas_set_color(canvas, 1);
        canvas_draw_str(canvas, 37, 34, "Game Over");
        break;
    }
}
