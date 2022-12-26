#include "../shapshup_i.h"
#include "../shapshup_custom_event.h"

#define TAG "ShapShupSceneLoadFile"

void shapshup_scene_load_file_on_enter(void* context) {
    furi_assert(context);
    ShapShupState* instance = (ShapShupState*)context;

    // Input events and views are managed by file_browser
    FuriString* file_path;
    FuriString* app_folder;

    file_path = furi_string_alloc();
    app_folder = furi_string_alloc_set_str(SHAPSHUP_PATH);

#ifdef SHAPSHUP_FAST_TRACK
    bool res = true;
    furi_string_printf(file_path, "%s", "/ext/subghz/temp/noise.sub");
#else
    DialogsFileBrowserOptions browser_options;
    dialog_file_browser_set_basic_options(&browser_options, SHAPSHUP_FILE_EXT, &I_sub1_10px);

    bool res =
        dialog_file_browser_show(instance->dialogs, file_path, app_folder, &browser_options);
#endif
#ifdef FURI_DEBUG
    FURI_LOG_D(
        TAG,
        "file_path: %s, app_folder: %s",
        furi_string_get_cstr(file_path),
        furi_string_get_cstr(app_folder));
#endif
    if(res) {
        ShapShupFileResults result =
            shapshup_main_view_load_file(instance->view_main, furi_string_get_cstr(file_path));

        if(result == ShapShupFileResultOk) {
            scene_manager_next_scene(instance->scene_manager, ShapshupSceneStart);
        } else {
            const char* desc = shapshup_files_result_description(result);
            FURI_LOG_E(TAG, "Returned error: %d (%s)", result, desc);

            FuriString* dialog_msg;
            dialog_msg = furi_string_alloc_printf("Cannot parse file\n%s", desc);
            dialog_message_show_storage_error(instance->dialogs, furi_string_get_cstr(dialog_msg));
#ifdef SHAPSHUP_FAST_TRACK
            view_dispatcher_stop(instance->view_dispatcher);
#else
            scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, ShapshupSceneStart);
#endif
            furi_string_free(dialog_msg);
        }
    } else {
        scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, ShapshupSceneStart);
    }

    furi_string_free(app_folder);
    furi_string_free(file_path);
}

void shapshup_scene_load_file_on_exit(void* context) {
    UNUSED(context);
}

bool shapshup_scene_load_file_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}