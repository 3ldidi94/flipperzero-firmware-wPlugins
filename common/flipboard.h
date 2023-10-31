#pragma once

#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include "key_setting_model.h"
#include "keyboard_input.h"

typedef struct Flipboard Flipboard;
typedef struct FlipboardModel FlipboardModel;

typedef View* (*GetPrimaryView)(void* context);

Flipboard* flipboard_alloc(
    char* app_name,
    char* primary_item_name,
    char* about_text,
    KeySettingModelFields fields,
    bool single_mode_button,
    bool attach_keyboard,
    KeyboardInputKey* keyboard_keys,
    KeyboardInputKey* keyboard_shift_keys,
    uint8_t keyboard_rows,
    GetPrimaryView get_primary_view);

FlipboardModel* flipboard_get_model(Flipboard* app);
ViewDispatcher* flipboard_get_view_dispatcher(Flipboard* app);
uint32_t flipboard_navigation_show_app_menu(void* context);
View* flipboard_get_primary_view(Flipboard* app);
void flipboard_override_config_view(Flipboard* app, View* view);
void flipboard_free(Flipboard* app);