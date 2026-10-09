#include "display_driver.h"
#include <driver/spi_master.h>
#include <driver/gpio.h>

static spi_device_handle_t spi;

DisplayDriver::DisplayDriver() {}

bool DisplayDriver::init() {
    pinMode(AMOLED_RST, OUTPUT);
    digitalWrite(AMOLED_RST, HIGH);
    delay(10);
    digitalWrite(AMOLED_RST, LOW);
    delay(20);
    digitalWrite(AMOLED_RST, HIGH);
    delay(120);

    spi_bus_config_t buscfg = {};
    buscfg.sclk_io_num = AMOLED_QSPI_SCK;
    buscfg.data0_io_num = AMOLED_QSPI_D0;
    buscfg.data1_io_num = AMOLED_QSPI_D1;
    buscfg.data2_io_num = AMOLED_QSPI_D2;
    buscfg.data3_io_num = AMOLED_QSPI_D3;
    buscfg.max_transfer_sz = SCREEN_WIDTH * SCREEN_HEIGHT * 2;

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = 40 * 1000 * 1000; // 40MHz QSPI
    devcfg.mode = 0;
    devcfg.spics_io_num = AMOLED_QSPI_CS;
    devcfg.queue_size = 7;
    devcfg.flags = SPI_DEVICE_HALFDUPLEX;

    if (spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO) != ESP_OK) {
        return false;
    }
    if (spi_bus_add_device(SPI2_HOST, &devcfg, &spi) != ESP_OK) {
        return false;
    }

    // Initial CO5300 QSPI AMOLED init sequence
    writeCommand(0x11); // Sleep Out
    delay(120);

    // Interface Pixel Format: 16-bit RGB565
    writeCommand(0x3A);
    writeData(0x55);

    // Display On
    writeCommand(0x29);
    delay(20);

    setBrightness(255);
    return true;
}

void DisplayDriver::writeCommand(uint8_t cmd) {
    spi_transaction_t t = {};
    t.length = 8;
    t.tx_buffer = &cmd;
    spi_device_polling_transmit(spi, &t);
}

void DisplayDriver::writeData(uint8_t data) {
    spi_transaction_t t = {};
    t.length = 8;
    t.tx_buffer = &data;
    spi_device_polling_transmit(spi, &t);
}

void DisplayDriver::writeDataBuffer(const uint8_t* buffer, uint32_t len) {
    spi_transaction_t t = {};
    t.length = len * 8;
    t.tx_buffer = buffer;
    spi_device_polling_transmit(spi, &t);
}

void DisplayDriver::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    writeCommand(0x2A); // Column Address Set
    writeData(x0 >> 8);
    writeData(x0 & 0xFF);
    writeData(x1 >> 8);
    writeData(x1 & 0xFF);

    writeCommand(0x2B); // Row Address Set
    writeData(y0 >> 8);
    writeData(y0 & 0xFF);
    writeData(y1 >> 8);
    writeData(y1 & 0xFF);

    writeCommand(0x2C); // Memory Write
}

void DisplayDriver::writePixels(const uint16_t* colors, uint32_t len) {
    writeDataBuffer((const uint8_t*)colors, len * 2);
}

void DisplayDriver::setBrightness(uint8_t brightness) {
    writeCommand(0x51);
    writeData(brightness);
}

void DisplayDriver::clear(uint16_t color) {
    setAddrWindow(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
    const int chunk = 512;
    uint16_t buf[chunk];
    for(int i = 0; i < chunk; ++i) buf[i] = color;

    uint32_t total = (uint32_t)SCREEN_WIDTH * SCREEN_HEIGHT;
    while(total > 0) {
        uint32_t count = total > chunk ? chunk : total;
        writePixels(buf, count);
        total -= count;
    }
}
