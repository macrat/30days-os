#include "graphics.h"
#include "hello.h"
#include "input_devices.h"
#include "math.h"


typedef struct MouseDecoder {
    uint8_t       buf[3];
    int8_t        index;
    mouse_state_t decoded;
} mouse_decoder_t;

static mouse_decoder_t _mouse_decoder = {0};

static uint8_t _keyboard_modifier = 0;


static void wait_keyboard_controller_ready() {
    while (io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY);
}

void init_keyboard() {
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYDAT, KBC_MODE);
}

void init_mouse() {
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_keyboard_controller_ready();
    io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);

    _mouse_decoder.index = -1;
    _mouse_decoder.decoded.x = get_screen_width() / 2;
    _mouse_decoder.decoded.y = get_screen_height() / 2;
}

void init_input_devices() {
    init_keyboard();
    init_mouse();
}

void on_raw_keyboard(const event_t* ev) {
    static const char keytable[0x80] = {
        0x00,
        0x1b, '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=',  '\b',
        '\t', 'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']',  '\n', 0x00,
        'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  '\'', '`',  0x00,
        '\\', 'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',  '.',  '/',  0x00,
        '*',  0x00,
        ' ',
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        '7',  '8', '9', '-',
        '4',  '5', '6', '+',
        '1',  '2', '3',
        '0',  '.',
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        '_',
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        '\\',
        0x00, 0x00,
    };

    uint8_t modifier = 0;
    switch (ev->uint8[0] & ~0x80) {
    case 0x1d:
        modifier = KEY_MOD_CTRL;
        break;
    case 0x2a:
        modifier = KEY_MOD_LEFT_SHIFT;
        break;
    case 0x36:
        modifier = KEY_MOD_RIGHT_SHIFT;
        break;
    case 0x38:
        modifier = KEY_MOD_ALT;
        break;
    }

    if (ev->uint8[0] & 0x80) {
        _keyboard_modifier &= ~modifier;
    } else {
        _keyboard_modifier |= modifier;
    }

    char character = keytable[ev->uint8[0] & ~0x80];

    if ('a' <= character && character <= 'z' && (_keyboard_modifier & KEY_MOD_SHIFT)) {
        character -= 'a' - 'A';
    }

    keyboard_event_t ke = {
        .type      = (ev->uint8[0] & 0x80) ? KEY_RELEASE : KEY_PRESS,
        .keycode   = ev->uint8[0],
        .character = character,
        .modifier  = _keyboard_modifier,
    };

    event_t ev2 = {
        .type = EVENT_KEYBOARD,
        .data = *(event_data_t*)&ke,
    };

    push_event(ev2);
}

static void decode_mouse() {
    _mouse_decoder.decoded.buttons = _mouse_decoder.buf[0] & 0x07;

    int32_t mx = _mouse_decoder.buf[1];
    int32_t my = _mouse_decoder.buf[2];

    if (_mouse_decoder.buf[0] & 0x10) {
        mx |= 0xffffff00;
    }
    if (_mouse_decoder.buf[0] & 0x20) {
        my |= 0xffffff00;
    }

    _mouse_decoder.decoded.x = max(0, min(get_screen_width(), _mouse_decoder.decoded.x + mx));
    _mouse_decoder.decoded.y = max(0, min(get_screen_height(), _mouse_decoder.decoded.y - my));
}

void on_raw_mouse(const event_t* ev) {
    if (_mouse_decoder.index < 0) {
        if (ev->uint8[0] == 0xfa) {
            _mouse_decoder.index = 0;
        }

        return;
    }

    if (_mouse_decoder.index == 0 && (ev->uint8[0] & 0xc8) != 0x08) return;  // drop invalid data

    _mouse_decoder.buf[_mouse_decoder.index] = ev->uint8[0];
    _mouse_decoder.index++;

    if (_mouse_decoder.index >= 3) {
        decode_mouse();
        _mouse_decoder.index = 0;

        event_t ev = {
            .type = EVENT_MOUSE,
            .data = *(event_data_t*)(&_mouse_decoder.decoded),
        };
        push_event(ev);
    }
}

const mouse_state_t* get_mouse_state() {
    return &_mouse_decoder.decoded;
}
