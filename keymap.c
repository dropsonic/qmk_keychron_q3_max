 /*
 Features:
 - Caps Word when tapping LShift+RShift: https://docs.qmk.fm/features/caps_word
 - Caps Lock is Ctrl on hold, Win+Space (switch languages) on tap
 - Prt Sc is Win+Shift+S (Snipping Tool)
 - Pause is Tap-to-talk in Microsoft PowerToys
 - Scroll Lock is enable/disable mic in Microsoft PowerToys
 - Holding Tab activates Numpad on the right block (PrtSc, etc., Left is 0, Down is Del) plus zoom in / zoom out / reset zoom on the encoder
 - Fn + Left/Right switches virtual desktops in Windows
 - Fn + End puts the PC to sleep, Fn + Home wakes it up
 - Fn + \ opens Calculator
 - Fn + PgUp turns on or off "RGB always on" mode for layers (when a non-base layer is active, RGB is always on if the mode is active)
 
 Encoder:
 - Rotate:   Volume up / Volume down, Press: Play/Pause
 - Hold Tab: Zoom in / Zoom out,      Press: Reset zoom
 - Hold Fn:  Undo / Redo,             Press: Mute/Unmute
 - Hold 7 / 8 / 9 / 0: adjusting RGB settings
 */

#include QMK_KEYBOARD_H
#include "keychron_common.h"

typedef union {
    uint32_t raw;
    struct {
        bool layer_rgb_override :1; // If activating a layer temporarly turns on RGB Matrix even if it was turned off
    };
} user_config_t;

user_config_t user_config; // Persistent config that is saved to EEPROM

static bool rgb_matrix_was_disabled = false;
static uint8_t previous_rgb_mode;

struct rgb_t {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

enum custom_keycodes {
    CST_RGBOVRRD = SAFE_RANGE,
	CST_RGB_M1,
	CST_RGB_M2,
};

enum layers {
    MAC_BASE,
    MAC_FN,
    WIN_BASE,
    WIN_FN,
    WIN_NUMPAD,
    WIN_RGB1,
    WIN_RGB2,
    WIN_RGB3,
    WIN_RGB4,
};

// clang-format off
const struct rgb_t layer_rgb_map[] = {
    [WIN_FN]     = { RGB_YELLOW },
    [WIN_NUMPAD] = { RGB_BLUE },
};

// clang-format off
const struct rgb_t key_rgb_map[] = {
    [KC_SYSTEM_SLEEP] = { RGB_RED },
    [C(G(KC_LEFT))]   = { RGB_BLUE },
    [C(G(KC_RIGHT))]  = { RGB_BLUE },
    [KC_MPLY]         = { RGB_AZURE },
    [KC_MPRV]         = { RGB_AZURE },
    [KC_MNXT]         = { RGB_AZURE },
    [CST_RGBOVRRD]    = { RGB_PURPLE },
    [RGB_TOG]         = { RGB_PURPLE },
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [MAC_BASE] = LAYOUT_tkl_ansi(
        KC_ESC,                 KC_BRID,  KC_BRIU,  KC_MCTRL, KC_LNPAD, RGB_VAD,  RGB_VAI,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,    KC_MUTE,    KC_SNAP,      KC_SIRI,   RGB_MOD,
        KC_GRV,                 KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,     KC_BSPC,    KC_INS,       KC_HOME,   KC_PGUP,
        KC_TAB,                 KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,    KC_BSLS,    KC_DEL,       KC_END,    KC_PGDN,
        KC_CAPS,                KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,              KC_ENT,
        KC_LSFT,                          KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,              KC_RSFT,                  KC_UP,
        KC_LCTL,                KC_LOPTN, KC_LCMMD,                               KC_SPC,                                 KC_RCMMD, KC_ROPTN, MO(MAC_FN), KC_RCTL,    KC_LEFT,      KC_DOWN,   KC_RGHT),

    [MAC_FN] =   LAYOUT_tkl_ansi(
        _______,                KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,     RGB_TOG,    _______,      _______,   RGB_TOG,
        _______,                BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,      _______,   _______,
        RGB_TOG,                RGB_MOD,  RGB_VAI,  RGB_HUI,  RGB_SAI,  RGB_SPI,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,      _______,   _______,
        _______,                RGB_RMOD, RGB_VAD,  RGB_HUD,  RGB_SAD,  RGB_SPD,  _______,  _______,  _______,  _______,  _______,  _______,              _______,
        _______,                          _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,  _______,  _______,              _______,                  _______,
        _______,                _______,  _______,                                _______,                                _______,  _______,  _______,    _______,    _______,      _______,   _______),

    [WIN_BASE] = LAYOUT_tkl_ansi(
        KC_ESC,                 KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,              KC_F8,              KC_F9,              KC_F10,             KC_F11,   KC_F12,     KC_MPLY,    LSG(KC_S),    LSG(KC_I), LSG(KC_A),
        KC_GRV,                 KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     LT(WIN_RGB1, KC_7), LT(WIN_RGB2, KC_8), LT(WIN_RGB3, KC_9), LT(WIN_RGB4, KC_0), KC_MINS,  KC_EQL,     KC_BSPC,    KC_INS,       KC_HOME,   KC_PGUP,
        LT(WIN_NUMPAD, KC_TAB), KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,               KC_I,               KC_O,               KC_P,               KC_LBRC,  KC_RBRC,    KC_BSLS,    KC_DEL,       KC_END,    KC_PGDN,
        LCTL_T(G(KC_SPC)),      KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,               KC_K,               KC_L,               KC_SCLN,            KC_QUOT,              KC_ENT,
        KC_LSFT,                          KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,               KC_M,               KC_COMM,            KC_DOT,             KC_SLSH,              KC_RSFT,                  KC_UP,
        KC_LCTL,                KC_LCMD,  KC_LALT,                                KC_SPC,                                                               KC_RALT,            KC_RWIN,  MO(WIN_FN), KC_RCTL,    KC_LEFT,      KC_DOWN,   KC_RGHT),

    [WIN_FN] =   LAYOUT_tkl_ansi(
        _______,                KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  RGB_VAD,  RGB_VAI,  KC_MPRV,  KC_MPLY,  KC_MNXT,    KC_MUTE,    KC_VOLD,  KC_VOLU,    KC_MUTE,    RGB_RMOD,      RGB_MOD,         RGB_TOG,
        _______,                BT_HST1,  BT_HST2,  BT_HST3,  P2P4G,    _______,  _______,  _______,  _______,  CST_RGB_M2, CST_RGB_M1, _______,  _______,    _______,    _______,       KC_SYSTEM_WAKE,  CST_RGBOVRRD,
        RGB_TOG,                RGB_MOD,  RGB_VAI,  RGB_HUI,  RGB_SAI,  RGB_SPI,  _______,  _______,  _______,  _______,    _______,    _______,  _______,    KC_CALC,    _______,       KC_SYSTEM_SLEEP, _______,
        _______,                RGB_RMOD, RGB_VAD,  RGB_HUD,  RGB_SAD,  RGB_SPD,  _______,  _______,  _______,  _______,    _______,    _______,              _______,
        _______,                          _______,  _______,  _______,  _______,  BAT_LVL,  NK_TOGG,  _______,  _______,    _______,    _______,              _______,                   _______,
        _______,                _______,  _______,                                _______,                                  _______,    _______,  _______,    _______,    C(G(KC_LEFT)), _______,         C(G(KC_RIGHT))),
        
    [WIN_NUMPAD] =  LAYOUT_tkl_ansi(
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    C(KC_0),    KC_7,         KC_8,      KC_9,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    KC_4,         KC_5,      KC_6,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    KC_1,         KC_2,      KC_3,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,              _______,
        _______,                          _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                  KC_NO,
        _______,                _______,  _______,                                _______,                                _______,  _______,  _______,    _______,    KC_0,         KC_DEL,    KC_NO),
    
    [WIN_RGB1] =  LAYOUT_tkl_ansi(
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    RGB_TOG,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,              _______,
        _______,                          _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                  _______,
        _______,                _______,  _______,                                _______,                                _______,  _______,  _______,    _______,    _______,      _______,   _______),
        
    [WIN_RGB2] =  LAYOUT_tkl_ansi(
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    RGB_TOG,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,              _______,
        _______,                          _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                  _______,
        _______,                _______,  _______,                                _______,                                _______,  _______,  _______,    _______,    _______,      _______,   _______),
        
    [WIN_RGB3] =  LAYOUT_tkl_ansi(
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    RGB_TOG,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,              _______,
        _______,                          _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                  _______,
        _______,                _______,  _______,                                _______,                                _______,  _______,  _______,    _______,    _______,      _______,   _______),
        
    [WIN_RGB4] =  LAYOUT_tkl_ansi(
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    RGB_TOG,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,    _______,    _______,      _______,   _______,
        _______,                _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,              _______,
        _______,                          _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,              _______,                  _______,
        _______,                _______,  _______,                                _______,                                _______,  _______,  _______,    _______,    _______,      _______,   _______),
};

// clang-format on
#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][2] = {
    [MAC_BASE]   = {ENCODER_CCW_CW(KC_VOLD,     KC_VOLU)},
    [MAC_FN]     = {ENCODER_CCW_CW(RGB_VAD,     RGB_VAI)},
    [WIN_BASE]   = {ENCODER_CCW_CW(KC_VOLD,     KC_VOLU)},
    [WIN_FN]     = {ENCODER_CCW_CW(C(KC_Z),     C(KC_Y))},
    [WIN_NUMPAD] = {ENCODER_CCW_CW(C(KC_MINUS), C(KC_EQUAL))},
    [WIN_RGB1]   = {ENCODER_CCW_CW(RGB_HUD,     RGB_HUI)},
    [WIN_RGB2]   = {ENCODER_CCW_CW(RGB_SAD,     RGB_SAI)},
    [WIN_RGB3]   = {ENCODER_CCW_CW(RGB_VAD,     RGB_VAI)},
    [WIN_RGB4]   = {ENCODER_CCW_CW(RGB_SPD,     RGB_SPI)},
};
#endif // ENCODER_MAP_ENABLE

void keyboard_post_init_user(void) {
  // Read the user config from EEPROM
  user_config.raw = eeconfig_read_user();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_keychron_common(keycode, record)) {
        return false;
    }
    
    switch (keycode) {
        case LCTL_T(G(KC_SPC)):
            if (record->tap.count && record->event.pressed) {
                tap_code16(G(KC_SPC)); // Send Left Win + Space
                return false; // Skip default key processing for this keycode
            }
            break;
        case RGB_TOG:
            if (record->event.pressed) {
                if (rgb_matrix_was_disabled) {
                    rgb_matrix_was_disabled = false;
                    rgb_matrix_disable_noeeprom();
                }
            }
            break;
        case RGB_RMOD:
			rgb_matrix_step_reverse();
			previous_rgb_mode = rgb_matrix_get_mode();
			return false;
			break;
        case RGB_MOD:
			rgb_matrix_step();
            previous_rgb_mode = rgb_matrix_get_mode();
			return false;
            break;
        case CST_RGBOVRRD:
            if (record->event.pressed) {
                user_config.layer_rgb_override ^= 1; // Toggles the status
                eeconfig_update_user(user_config.raw); // Writes the new status to EEPROM
            }
            return false;
            break;
		case CST_RGB_M1:
			previous_rgb_mode = RGB_MATRIX_TYPING_HEATMAP;
			return false;
			break;
		case CST_RGB_M2:
			previous_rgb_mode = RGB_MATRIX_SPLASH;
			return false;
			break;
    }
    
    return true;  // Process all other keycodes normally
}

// Highlight layers
uint8_t get_rgb_layer(layer_state_t state) {
    if (get_highest_layer(state) > 0) {
        const uint8_t layer = get_highest_layer(state);
        
        if (layer < (sizeof(layer_rgb_map) / sizeof(layer_rgb_map[0])) && (layer_rgb_map[layer].r > 0 || layer_rgb_map[layer].g > 0 || layer_rgb_map[layer].b > 0)) {
            return layer;
        }
    }
    
    return 0;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    if (user_config.layer_rgb_override > 0 && get_rgb_layer(state) > 0) {
        if (!rgb_matrix_is_enabled()) {
            rgb_matrix_was_disabled = true;  // Track that it was off
            rgb_matrix_enable_noeeprom();    // Enable RGB Matrix
        } else {
            previous_rgb_mode = rgb_matrix_get_mode();
            rgb_matrix_mode_noeeprom(RGB_MATRIX_NONE);
        }
    } else {
        // If RGB Matrix was originally off, disable it again
        if (rgb_matrix_was_disabled) {
            rgb_matrix_disable_noeeprom();
            rgb_matrix_was_disabled = false;  // Reset the tracking variable
        } else {
            rgb_matrix_mode_noeeprom(previous_rgb_mode);
        }
    }
    
    return state;
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    const uint8_t layer = get_rgb_layer(layer_state);
    
    if (layer > 0) {
        const struct rgb_t layer_color = layer_rgb_map[layer];
        
        for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
            for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
                uint8_t index = g_led_config.matrix_co[row][col];

                if (index >= led_min && index < led_max && index != NO_LED) {
                    const uint16_t key = keymap_key_to_keycode(layer, (keypos_t){col,row});
                    
                    if (key > KC_TRNS) {
                        struct rgb_t key_color = layer_color;
                        
                        if (key < (sizeof(key_rgb_map) / sizeof(key_rgb_map[0])) && (key_rgb_map[key].r > 0 || key_rgb_map[key].g > 0 || key_rgb_map[key].b > 0)) {
                            key_color = key_rgb_map[key];
                        }
                        
                        rgb_matrix_set_color(index, key_color.r, key_color.g, key_color.b);
                    } else {
                        rgb_matrix_set_color(index, RGB_OFF);
                    }
                }
            }
        }
    }
        
    return false;
}
