#include <EPD.h>

const unsigned char lut_full_update[] =
{
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
    0x35, 0x51, 0x51, 0x19, 0x01, 0x00
};

void EPD::initializeDevice() {
    pinMode(_cs, OUTPUT);
    pinMode(_dc, OUTPUT);
    pinMode(_reset, OUTPUT);
    pinMode(_busy, INPUT);

    digitalWrite(_cs, LOW);
    digitalWrite(_reset, HIGH);
    delay(10);
    digitalWrite(_reset, LOW);
    delay(10);
    digitalWrite(_reset, HIGH);
    delay(10);

    _spi->begin();

    configDevice();

    command(EPD_WRITE_LUT_REGISTER);
    for (int i = 0; i < 30; i++) {
        data(lut_full_update[i]);
    }

}

void EPD::setPixel(int x, int y, color_t c) {
    if (x < 0 || x >= _width) return;
    if (y < 0 || y >= _height) return;

//    translateCoordinates(&x, &y);

    int off = 0;
    int pos = 0;
    int bit = 0;

    switch (rotation) {
        case 0:
            off = y * _hardWidth + x;
            break;

        case 1:
            off = x * _hardWidth + (_hardWidth - 1 - y);
            break;

        case 2:
            off = (_hardHeight - 1 - y) * _hardWidth + (_hardWidth - 1 - x);
            break;

        case 3:
            off = (_hardHeight - 1 - x) * _hardWidth + y;
            break;
    }

    pos = off / 8;
    bit = off % 8;


    if (c == 0) {
        _buffer[pos] &= ~(0x80 >> bit);
    } else {
        _buffer[pos] |= (0x80 >> bit);
    }
        

    if (_bufferDepth <= 0) {
        _bufferDepth = 0;
        updateDisplay();
    }
}

void EPD::startBuffer() {
    _bufferDepth++;
}

void EPD::endBuffer() {
    _bufferDepth--;

    if (_bufferDepth <= 0) {
        _bufferDepth = 0;
        updateDisplay();
    }
}

void EPD::updateDisplay() {
    command(EPD_SET_RAM_X_ADDRESS_START_END_POSITION, true);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    data(0, true);
    data(((_hardWidth-1) >> 3) & 0xFF, true);
    command(EPD_SET_RAM_Y_ADDRESS_START_END_POSITION, true);
    data(0, true);
    data(0, true);
    data((_hardHeight-1) & 0xFF, true);
    data(((_hardHeight-1) >> 8) & 0xFF, true);

    command(EPD_SET_RAM_X_ADDRESS_COUNTER, true);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    data(0, true);
    command(EPD_SET_RAM_Y_ADDRESS_COUNTER, true);
    data(0, true);
    data(0, true);

    command(EPD_WRITE_RAM, true);
    /* send the image data */
    if (_invert) {
        for (int i = 0; i < _hardWidth / 8 * _hardHeight; i++) {
            data(~_buffer[i], true);
        }
    } else {
        for (int i = 0; i < _hardWidth / 8 * _hardHeight; i++) {
            data(_buffer[i], true);
        }
    }

    waitUntilIdle();
    command(EPD_DISPLAY_UPDATE_CONTROL_2, true);
    data(0xC4, true);
    command(EPD_MASTER_ACTIVATION, true);
    command(EPD_TERMINATE_FRAME_READ_WRITE);

}

void EPD::setRotation(int r) {
    rotation = r & 0x03;
    switch (rotation) {
        case 0:
        case 2:
            _width = _hardWidth;
            _height = _hardHeight;
            break;

        case 1:
        case 3:
            _height = _hardWidth;
            _width = _hardHeight;
            break;
    }
}

int EPD::getWidth() {
    return _width;
}

int EPD::getHeight() {
    return _height;
}

/*--------------------------*/

void EPD29::configDevice() {
    waitUntilIdle();
    command(EPD_DRIVER_OUTPUT_CONTROL, true);
    data(295 & 0xFF, true);
    data((295 >> 8) & 0xFF, true);
    data(0x00, true);                     // GD = 0; SM = 0; TB = 0;
    command(EPD_BOOSTER_SOFT_START_CONTROL, true);
    data(0xD7, true);
    data(0xD6, true);
    data(0x9D, true);
    command(EPD_WRITE_VCOM_REGISTER, true);
    data(0xA8, true);                     // VCOM 7C
    command(EPD_SET_DUMMY_LINE_PERIOD, true);
    data(0x1A, true);                     // 4 dummy lines per gate
    command(EPD_SET_GATE_TIME, true);
    data(0x08, true);                     // 2us per line
    command(EPD_DATA_ENTRY_MODE_SETTING, true);
    data(0x03);                     // X increment; Y increment
    waitUntilIdle();
}
