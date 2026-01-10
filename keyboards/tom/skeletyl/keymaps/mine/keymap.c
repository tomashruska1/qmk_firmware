#include QMK_KEYBOARD_H
#include "quantum/quantum.h"
#include "print.h"
#include "keymap.h"

layer_state_t active_base_layer      = BASE_LAYER_COLEMAK;
bool          one_shot_layer_active  = false;
bool          permanent_layer_active = false;
bool          caps_lock_active       = false;
bool          caps_lock_pressed      = false;
uint32_t      timers[9]              = {0, 0, 0, 0, 0, 0, 0, 0, 0};

#ifdef LAYER_BASED_BACKLIGHT
uint8_t layer_colors[4][3] = {{HSV_AZURE}, {HSV_GREEN}, {HSV_PURPLE}, {HSV_TEAL}};
#endif

uint8_t lalt_mod_bit = MOD_BIT(KC_LALT);

uint32_t *get_timer(uint16_t keycode) {
    switch (keycode) {
        case LS:
            return &timers[0];
        case M_A:
        case M_GRV:
            return &timers[1];
        case M_S:
        case MC_R:
        case M_HOME:
            return &timers[2];
        case M_D:
        case MC_S:
        case M_END:
            return &timers[3];
        case M_F:
        case MC_T:
        case M_LBRC:
            return &timers[4];
        case M_J:
        case MC_N:
        case M_DOWN:
            return &timers[5];
        case M_K:
        case MC_E:
        case M_UP:
            return &timers[6];
        case M_L:
        case MC_I:
        case M_RGHT:
            return &timers[7];
        case M_SCLN:
        case MC_O:
            return &timers[8];
        default:
            return NULL;
    }
}

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case M_D:
        case M_K:
        case M_END:
        case M_UP:
        case MC_S:
        case MC_E:
            return TAPPING_TERM - 70;
        case M_F:
        case M_LBRC:
        case M_J:
        case M_DOWN:
        case MC_T:
        case MC_N:
            return TAPPING_TERM - 25;
        case M_A: // MC_A is handled by this as well
            return TAPPING_TERM + 100;
        default:
            return TAPPING_TERM;
    }
}

void set_base_layer_rgb(void) {
#ifdef LAYER_BASED_BACKLIGHT
    // if (active_base_layer == BASE_LAYER_QWERTY) {
        print("current layer is qwerty, will cycle spiral\n");
        rgb_matrix_mode(RGB_MATRIX_CYCLE_SPIRAL);
        return;
    // }
    // print("current layer is colemak, will cycle left to right\n");
    // rgb_matrix_mode(RGB_MATRIX_CYCLE_LEFT_RIGHT);
#endif
}

void reset(void) {
    layer_clear();
    one_shot_layer_active  = false;
    permanent_layer_active = false;
    set_base_layer_rgb();
}

void housekeeping_task_user(void) {
    if (permanent_layer_active) {
        return;
    }

    // if (one_shot_layer_active || (get_highest_layer(layer_state) != BASE_LAYER_QWERTY && get_highest_layer(layer_state) != BASE_LAYER_COLEMAK)) {
    if (one_shot_layer_active || get_highest_layer(layer_state) > 0) {
        if (last_input_activity_elapsed() > KEYSTROKE_TIMER) {
            print("housekeeping task: reset\n");
            reset();
        }
    }
}

bool is_not_mod(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case M_A:
        case M_S:
        case M_D:
        case M_F:
        case M_J:
        case M_K:
        case M_L:
        case M_SCLN:
        case M_GRV:
        case M_HOME:
        case M_END:
        case M_LBRC:
        case M_DOWN:
        case M_UP:
        case M_RGHT:
        case MC_R:
        case MC_S:
        case MC_T:
        case MC_N:
        case MC_E:
        case MC_I:
        case MC_O:
            return !record->event.pressed;
        default:
            return true;
    }
}

void process_modifiers(uint16_t keycode, uint8_t mod_mask, bool right_alt) {
    // TODO: implement modifier suppression
    // probably has to be a block per modifier, then a block for the rest of the keys on a side
    // maybe will have to rethink the get_timer function to accept a modifier directly
    // or another function doing the same by modifier key
    // another gotcha is when accidentally holding down mod key, and then pressing multiple keys in quick succession
    // have to ensure the mod key is fired only once
    // yet another gotcha is when holding down both alts
    // more observation required
    uprintf("mod_mask: %d, keycode: %d\n", mod_mask, keycode);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    static bool right_alt = false;
    static bool left_alt  = false;

    uprintf("keycode: %d was pressed %d\n", keycode, record->event.pressed);

    if (keycode == LAYER_CLEAR) {
        print("layer clear: reset\n");
        reset();
        return false;
    }

    uint8_t current_layer = get_highest_layer(layer_state);

    if (keycode == LAYER_SWITCH) {
        return process_layer_switch(keycode, record, current_layer);
    }

    if (one_shot_layer_active && is_not_mod(keycode, record) && !record->event.pressed) {
        print("deactivate one shot layer: reset\n");
        reset();
    }

    if (!caps_lock_active && caps_lock_pressed && is_shift(keycode) && !record->event.pressed) {
        shift other_shift = get_other_shift(keycode);
        if (!other_shift.pressed) {
            caps_lock_active = true;
        }
    } else if (should_deactivate_caps_lock(keycode, record)) {
        deactivate_caps_lock();
    } else if (should_activate_caps_lock(keycode, record)) {
        activate_caps_lock();
    }

    if ((keycode == M_L || keycode == M_RGHT || keycode == MC_I) && !record->tap.count) {
        right_alt = record->event.pressed;
        uprintf("right alt is pressed %d\n", right_alt);
    }

    if ((keycode == M_S || keycode == M_HOME || keycode == MC_R) && !record->tap.count) {
        left_alt = record->event.pressed;
        uprintf("left alt is pressed %d\n", left_alt);
    }

    uint8_t mod_mask = get_mods();

    if (keycode == KC_SPC && record->event.pressed && left_alt && (mod_mask & lalt_mod_bit) == lalt_mod_bit) {
        print("tab shenanigans\n");
        if (!right_alt) {
            tap_code(KC_RCTL);
            del_mods(lalt_mod_bit);
        }
        tap_code(KC_TAB);
        if (!right_alt) {
            add_mods(lalt_mod_bit);
            tap_code(KC_RCTL);
        }
        return false;
    }

    return true;
}

void update_default_layer(layer_state_t default_layer) {
    default_layer_set(default_layer + 1);
    active_base_layer = default_layer;
    set_base_layer_rgb();
}

bool process_layer_switch(uint16_t keycode, keyrecord_t *record, uint8_t current_layer) {
    uprintf("current layer: %d\n", current_layer);
    // tap_code(KC_LCTL); // why am I pressing this?

    if (current_layer == TOP_LAYER) {
        print("current layer is top layer, will do nothing\n");
        return false;
    }

    // if (current_layer == BASE_LAYER_COLEMAK) {
    //     current_layer = BASE_LAYER_QWERTY;
    // }

    uint8_t active_mods = get_mods();

    // if (active_mods & MOD_BIT(KC_RGUI)) {
    //     print("setting layer 1 as default layer\n");
    //     update_default_layer(BASE_LAYER_QWERTY);
    //     layer_clear();
    //     return false;
    // }

    // if (active_mods & MOD_BIT(KC_LGUI)) {
    //     print("setting layer 0 as default layer\n");
    //     update_default_layer(BASE_LAYER_COLEMAK);
    //     layer_clear();
    //     return false;
    // }

    uint32_t *switch_timer = get_timer(keycode);

    if (record->event.pressed) {
        uprintf("switching to layer %d\n", current_layer + 1);
        *switch_timer         = timer_read32();
        one_shot_layer_active = false;
        layer_move(current_layer + 1);

#ifdef LAYER_BASED_BACKLIGHT
        uint8_t *layer_color = layer_colors[current_layer];
        rgb_matrix_sethsv(layer_color[0], layer_color[1], layer_color[2]);
#endif

        if (get_mods() & MOD_MASK_SHIFT) {
            permanent_layer_active = true;
#ifdef LAYER_BASED_BACKLIGHT
            rgb_matrix_mode(RGB_MATRIX_BREATHING);
        } else {
            rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
#endif
        }
    } else if (!permanent_layer_active && (timer_read32() - *switch_timer) <= SWITCH_TIMER) {
        print("switching to one shot layer\n");
        one_shot_layer_active = true;
#ifdef LAYER_BASED_BACKLIGHT
        rgb_matrix_mode(RGB_MATRIX_BAND_PINWHEEL_VAL);
#endif
    }

    return false;
}

bool is_shift(uint16_t keycode) {
    return keycode == M_D
        || keycode == M_K
        || keycode == M_END
        || keycode == M_UP
        || keycode == MC_S
        || keycode == MC_E;
}

shift get_other_shift(uint16_t keycode) {
    uint32_t *other_shift_timer = NULL;
    bool      other_shift_pressed = false;

    switch (keycode) {
        case M_D:
        case M_END:
        case MC_S:
            other_shift_timer   = get_timer(M_K);
            other_shift_pressed = get_mods() & MOD_BIT(KC_RSFT);
            break;
        case M_K:
        case M_UP:
        case MC_E:
            other_shift_timer   = get_timer(M_D);
            other_shift_pressed = get_mods() & MOD_BIT(KC_LSFT);
            break;
    }

    shift sft = { other_shift_pressed, other_shift_timer };
    return sft;
}

bool should_activate_caps_lock(uint16_t keycode, keyrecord_t *record) {
    if (!is_shift(keycode) || caps_lock_pressed) {
        return false;
    }

    uint32_t *timer = get_timer(keycode);

    if (record->event.pressed) {
        print("shift pressed, saving time\n");
        *timer = timer_read32();
        return false;
    }

    if ((timer_read32() - *timer) < CAPS_LOCK_TIMER) {
        print("shift not held for enough time\n");
        return false;
    }

    shift other_shift = get_other_shift(keycode);

    if (!other_shift.pressed) {
        printf("other shift not pressed. current mods: %d\n", get_mods());
        return false;
    }

    return (timer_read32() - *other_shift.timer) >= CAPS_LOCK_TIMER;
}

bool should_deactivate_caps_lock(uint16_t keycode, keyrecord_t *record) {
    return caps_lock_active
        && is_shift(keycode)
        && !record->event.pressed
        && (get_mods() & MOD_MASK_SHIFT);
}

void activate_caps_lock() {
    printf("activating caps lock. current mods: %d\n", get_mods());
    caps_lock_pressed = true;
    tap_code(KC_CAPS);
}

void deactivate_caps_lock() {
    print("deactivating caps lock\n");
    caps_lock_active  = false;
    caps_lock_pressed = false;
    tap_code(KC_CAPS);
}

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] =
{
#ifdef COLEMAK
    [0] = LAYOUT_split_3x5_3(
        KC_Q,    KC_W,    KC_F,    KC_P,    KC_B,                               KC_J,    KC_L,    KC_U,    KC_Y,    KC_SCLN,
        MC_A,    MC_R,    MC_S,    MC_T,    KC_G,                               KC_M,    MC_N,    MC_E,    MC_I,    MC_O,
        KC_SLSH, KC_Z,    KC_X,    KC_C,    KC_D,                               KC_V,    KC_K,    KC_H,    KC_COMM, KC_DOT,
                                   KC_DEL,  KC_BSPC, KC_ESC,           KC_SPC,  KC_ENT,  LS
    ),
#else
    [0] = LAYOUT_split_3x5_3(
        KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                               KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,
        M_A,     M_S,     M_D,     M_F,     KC_G,                               KC_H,    M_J,     M_K,     M_L,     M_SCLN,
        KC_SLSH, KC_Z,    KC_X,    KC_C,    KC_V,                               KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,
                                   KC_DEL,  KC_BSPC, KC_ESC,           KC_SPC,  KC_ENT,  LS
    ),
#endif
    [1] = LAYOUT_split_3x5_3(
        KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                               KC_6,    KC_7,    KC_8,    KC_9,    KC_0,
        M_GRV,   M_HOME,  M_END,   M_LBRC,  KC_RBRC,                            KC_LEFT, M_DOWN,  M_UP,    M_RGHT,  KC_TRNS,
        KC_NUBS, KC_TRNS, KC_TRNS, KC_PGUP, KC_PGDN,                            KC_MINS, KC_EQL,  KC_QUOT, KC_NUHS, KC_NUBS,
                                   KC_TRNS, KC_TRNS, LC,               KC_SPC,  KC_TRNS, LS
    ),
    [2] = LAYOUT_split_3x5_3(
        KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,                              KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,
        KC_F1,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            KC_LEFT, M_DOWN,  M_UP,    M_RGHT,  KC_F12,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                                   KC_TRNS, KC_TRNS, LC,               KC_PSCR, KC_ENT,  LS
    ),
    [3] = LAYOUT_split_3x5_3(
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            KC_P1,   KC_P2,   KC_P3,   KC_P0,   KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            KC_P4,   KC_P5,   KC_P6,   KC_COMM, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,                            KC_P7,   KC_P8,   KC_P9,   KC_TRNS, KC_TRNS,
                                   KC_TRNS, KC_TRNS, LC,               KC_TRNS, KC_ENT,  KC_NO
    )
};
