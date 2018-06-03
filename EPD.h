#ifndef _EPD_H
#define _EPD_H

#define EPD_DRIVER_OUTPUT_CONTROL                       0x01
#define EPD_BOOSTER_SOFT_START_CONTROL                  0x0C
#define EPD_GATE_SCAN_START_POSITION                    0x0F
#define EPD_DEEP_SLEEP_MODE                             0x10
#define EPD_DATA_ENTRY_MODE_SETTING                     0x11
#define EPD_SW_RESET                                    0x12
#define EPD_TEMPERATURE_SENSOR_CONTROL                  0x1A
#define EPD_MASTER_ACTIVATION                           0x20
#define EPD_DISPLAY_UPDATE_CONTROL_1                    0x21
#define EPD_DISPLAY_UPDATE_CONTROL_2                    0x22
#define EPD_WRITE_RAM                                   0x24
#define EPD_WRITE_VCOM_REGISTER                         0x2C
#define EPD_WRITE_LUT_REGISTER                          0x32
#define EPD_SET_DUMMY_LINE_PERIOD                       0x3A
#define EPD_SET_GATE_TIME                               0x3B
#define EPD_BORDER_WAVEFORM_CONTROL                     0x3C
#define EPD_SET_RAM_X_ADDRESS_START_END_POSITION        0x44
#define EPD_SET_RAM_Y_ADDRESS_START_END_POSITION        0x45
#define EPD_SET_RAM_X_ADDRESS_COUNTER                   0x4E
#define EPD_SET_RAM_Y_ADDRESS_COUNTER                   0x4F
#define EPD_TERMINATE_FRAME_READ_WRITE                  0xFF

#include <DSPI.h>
#include <DisplayCore.h>

class EPD : public DisplayCore {
    private:
        DSPI *_spi;
        uint8_t _cs;
        uint8_t _dc;
        uint8_t _reset;
        uint8_t _busy;
        int _width;
        int _height;
        int _hardWidth;
        int _hardHeight;
        uint8_t *_buffer;
        int _bufferDepth;
        bool _invert;

    public:
        EPD(DSPI *spi, uint8_t cs, uint8_t dc, uint8_t reset, uint8_t busy, int width, int height, uint8_t *buffer) : 
            _spi(spi), _cs(cs), _dc(dc), _reset(reset), _busy(busy), _width(width), _height(height), _hardWidth(width), _hardHeight(height), _buffer(buffer), _bufferDepth(0), _invert(false) { rotation = 0; }
        EPD(DSPI &spi, uint8_t cs, uint8_t dc, uint8_t reset, uint8_t busy, int width, int height, uint8_t *buffer) : 
            _spi(&spi), _cs(cs), _dc(dc), _reset(reset), _busy(busy), _width(width), _height(height), _hardWidth(width), _hardHeight(height), _buffer(buffer), _bufferDepth(0), _invert(false) { rotation = 0; }

        int getWidth() override;
        int getHeight() override;

        void initializeDevice() override;
        virtual void configDevice() = 0;

        void setPixel(int x, int y, color_t c) override;
        void startBuffer() override;
        void endBuffer() override;
        void updateDisplay();

        void setRotation(int rotation) override;
        void displayOn() override {}
        void displayOff() override {}
        void invertDisplay(bool i) override { _invert = i; }
        
        void command(uint8_t c, bool cont = false) {
            digitalWrite(_dc, LOW);
            digitalWrite(_cs, LOW);
            _spi->transfer(c);
            if (!cont) digitalWrite(_cs, HIGH);
        }

        void data(uint8_t d, bool cont = false) {
            digitalWrite(_dc, HIGH);
            digitalWrite(_cs, LOW);
            _spi->transfer(d);
            if (!cont) digitalWrite(_cs, HIGH);
        }

        void waitUntilIdle() {
            while (digitalRead(_busy)) {
                delay(1);
            }
        }

};

class EPD29 : public EPD {

    private:
        uint8_t _buffer[128 * 296 / 8];

    public:
        EPD29(DSPI *spi, uint8_t cs, uint8_t dc, uint8_t res, uint8_t busy) : 
            EPD(spi, cs, dc, res, busy, 128, 296, _buffer) {}
        EPD29(DSPI &spi, uint8_t cs, uint8_t dc, uint8_t res, uint8_t busy) : 
            EPD(&spi, cs, dc, res, busy, 128, 296, _buffer) {}

        void configDevice();
};

#endif
