#include <furi.h>

#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/variable_item_list.h>
#include <toolbox/value_index.h>

#include "quac.h"
#include "scenes.h"
#include "scene_settings.h"
#include "../quac_settings.h"

#include <lib/toolbox/path.h>

// "About" is always the last item added in scene_settings_on_enter. Its list index is
// tracked here (rather than hardcoded) so reordering or adding settings above it can't
// silently break the event match in scene_settings_on_event.
static uint32_t scene_settings_about_index = 0;

static const char* const layout_text[2] = {"Vert", "Horiz"};
static const uint32_t layout_value[2] = {QUAC_APP_PORTRAIT, QUAC_APP_LANDSCAPE};

static const char* const show_offon_text[2] = {"OFF", "ON"};
static const uint32_t show_offon_value[2] = {false, true};

#define V_DURATION_COUNT 8
static const char* const duration_text[V_DURATION_COUNT] = {
    "500 ms",
    "1 sec",
    "1.5 sec",
    "2 sec",
    "2.5 sec",
    "3 sec",
    "5 sec",
    "10 sec",
};
static const uint32_t duration_value[V_DURATION_COUNT] = {
    500,
    1000,
    1500,
    2000,
    2500,
    3000,
    5000,
    10000,
};

static const char* const disabled_enabled_text[2] = {"Disabled", "Enabled"};
static const uint32_t disabled_enabled_value[2] = {false, true};

static void scene_settings_layout_changed(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, layout_text[index]);
    app->settings.layout = layout_value[index];
}

static void scene_settings_show_icons_changed(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, show_offon_text[index]);
    app->settings.show_icons = show_offon_value[index];
}

static void scene_settings_show_headers_changed(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, show_offon_text[index]);
    app->settings.show_headers = show_offon_value[index];
}

static void scene_settings_subghz_duration_changed(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, duration_text[index]);
    app->settings.subghz_duration = duration_value[index];
}

static void scene_settings_rfid_duration_changed(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, duration_text[index]);
    app->settings.rfid_duration = duration_value[index];
}

static void scene_settings_nfc_duration_changed(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, duration_text[index]);
    app->settings.nfc_duration = duration_value[index];
}

static void scene_settings_ibutton_duration_changed(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, duration_text[index]);
    app->settings.ibutton_duration = duration_value[index];
}

static void scene_settings_picopass_duration_changed(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, duration_text[index]);
    app->settings.picopass_duration = duration_value[index];
}

static void scene_settings_ir_ext_changed(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, disabled_enabled_text[index]);
    app->settings.ir_use_ext_module = disabled_enabled_value[index];
}

static void scene_settings_show_hidden_changed(VariableItem* item) {
    App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, show_offon_text[index]);
    app->settings.show_hidden = show_offon_value[index];
}

static void scene_settings_enter_callback(void* context, uint32_t index) {
    App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static VariableItem* scene_settings_add_item(
    VariableItemList* vil,
    uint32_t* item_index,
    const char* label,
    uint8_t values_count,
    VariableItemChangeCallback callback,
    void* context) {
    VariableItem* item = variable_item_list_add(vil, label, values_count, callback, context);
    (*item_index)++;
    return item;
}

// For each scene, implement handler callbacks
void scene_settings_on_enter(void* context) {
    App* app = context;

    VariableItemList* vil = app->vil_settings;
    variable_item_list_reset(vil);

    VariableItem* item;
    uint8_t value_index;
    uint32_t item_index = 0;

    item = scene_settings_add_item(
        vil, &item_index, "Layout", 2, scene_settings_layout_changed, app);
    value_index = value_index_uint32(app->settings.layout, layout_value, 2);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, layout_text[value_index]);

    item = scene_settings_add_item(
        vil, &item_index, "Show Icons", 2, scene_settings_show_icons_changed, app);
    value_index = value_index_uint32(app->settings.show_icons, show_offon_value, 2);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, show_offon_text[value_index]);

    item = scene_settings_add_item(
        vil, &item_index, "Show Headers", 2, scene_settings_show_headers_changed, app);
    value_index = value_index_uint32(app->settings.show_headers, show_offon_value, 2);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, show_offon_text[value_index]);

    item = scene_settings_add_item(
        vil,
        &item_index,
        "SubGhz Duration",
        V_DURATION_COUNT,
        scene_settings_subghz_duration_changed,
        app);
    value_index =
        value_index_uint32(app->settings.subghz_duration, duration_value, V_DURATION_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, duration_text[value_index]);

    item = scene_settings_add_item(
        vil,
        &item_index,
        "RFID Duration",
        V_DURATION_COUNT,
        scene_settings_rfid_duration_changed,
        app);
    value_index =
        value_index_uint32(app->settings.rfid_duration, duration_value, V_DURATION_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, duration_text[value_index]);

    item = scene_settings_add_item(
        vil,
        &item_index,
        "NFC Duration",
        V_DURATION_COUNT,
        scene_settings_nfc_duration_changed,
        app);
    value_index = value_index_uint32(app->settings.nfc_duration, duration_value, V_DURATION_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, duration_text[value_index]);

    item = scene_settings_add_item(
        vil,
        &item_index,
        "iButton Duration",
        V_DURATION_COUNT,
        scene_settings_ibutton_duration_changed,
        app);
    value_index =
        value_index_uint32(app->settings.ibutton_duration, duration_value, V_DURATION_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, duration_text[value_index]);

    item = scene_settings_add_item(
        vil,
        &item_index,
        "Picopass Duration",
        V_DURATION_COUNT,
        scene_settings_picopass_duration_changed,
        app);
    value_index =
        value_index_uint32(app->settings.picopass_duration, duration_value, V_DURATION_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, duration_text[value_index]);

    item = scene_settings_add_item(
        vil, &item_index, "IR Ext Module", 2, scene_settings_ir_ext_changed, app);
    value_index = value_index_uint32(app->settings.ir_use_ext_module, disabled_enabled_value, 2);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, disabled_enabled_text[value_index]);

    item = scene_settings_add_item(
        vil, &item_index, "Show Hidden", 2, scene_settings_show_hidden_changed, app);
    value_index = value_index_uint32(app->settings.show_hidden, show_offon_value, 2);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, show_offon_text[value_index]);

    // Last item is always "About"; record its index for scene_settings_on_event
    scene_settings_about_index = item_index;
    item = scene_settings_add_item(vil, &item_index, "About", 1, NULL, NULL);
    variable_item_list_set_enter_callback(vil, scene_settings_enter_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, QView_Settings);
}

bool scene_settings_on_event(void* context, SceneManagerEvent event) {
    App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == scene_settings_about_index) {
            consumed = true;
            scene_manager_next_scene(app->scene_manager, QScene_About);
        }
    }

    return consumed;
}

void scene_settings_on_exit(void* context) {
    App* app = context;
    VariableItemList* vil = app->vil_settings;
    variable_item_list_reset(vil);

    quac_save_settings(app);
}
