#include QMK_KEYBOARD_H

#define KEYSTROKE_TIMER 5000
#define CAPS_LOCK_TIMER 500
#define SWITCH_TIMER 175
// #define BASE_LAYER_QWERTY 1
// #define BASE_LAYER_COLEMAK 0
#define TOP_LAYER 4

#define DEFAULT_RGB_SPEED 100

#define M_A LGUI_T(KC_A)
#define M_S LALT_T(KC_S)
#define M_D LSFT_T(KC_D)
#define M_F LCTL_T(KC_F)

#define M_J RCTL_T(KC_J)
#define M_K RSFT_T(KC_K)
#define M_L ALT_T(KC_L)
#define M_SCLN RGUI_T(KC_SCLN)

#define MC_A LGUI_T(KC_A)
#define MC_R LALT_T(KC_R)
#define MC_S LSFT_T(KC_S)
#define MC_T LCTL_T(KC_T)

#define MC_N RCTL_T(KC_N)
#define MC_E RSFT_T(KC_E)
#define MC_I ALT_T(KC_I)
#define MC_O RGUI_T(KC_O)

#define M_GRV LGUI_T(KC_GRV)
#define M_HOME LALT_T(KC_HOME)
#define M_END LSFT_T(KC_END)
#define M_LBRC LCTL_T(KC_LBRC)

#define M_DOWN RCTL_T(KC_DOWN)
#define M_UP RSFT_T(KC_UP)
#define M_RGHT ALT_T(KC_RGHT)

#define LAYER_BASED_BACKLIGHT
// #define COLEMAK

enum new_keycodes
{
    OFFSET = SAFE_RANGE,
    LAYER_SWITCH,
    LAYER_CLEAR,

    LS = LAYER_SWITCH,
    LC = LAYER_CLEAR
};

typedef struct {
    bool pressed;
    uint32_t* timer;
} shift;


bool process_layer_switch(uint16_t keycode, keyrecord_t *record, uint8_t current_layer);
bool is_shift(uint16_t keycode);
shift get_other_shift(uint16_t keycode);
bool should_activate_caps_lock(uint16_t keycode, keyrecord_t *record);
bool should_deactivate_caps_lock(uint16_t keycode, keyrecord_t *record);
void activate_caps_lock(void);
void deactivate_caps_lock(void);
