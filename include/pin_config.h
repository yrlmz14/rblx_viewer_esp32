#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// Waveshare 2.06" AMOLED ESP32-S3 (ESP32-S3-N32R8) Pin Definitions

// QSPI AMOLED Display (CO5300 / SH8601 Driver)
#define AMOLED_QSPI_CS       9
#define AMOLED_QSPI_SCK      10
#define AMOLED_QSPI_D0       11
#define AMOLED_QSPI_D1       12
#define AMOLED_QSPI_D2       13
#define AMOLED_QSPI_D3       14
#define AMOLED_RST          21
#define AMOLED_TE           22
#define AMOLED_BL           45

// Touch Panel (I2C)
#define TOUCH_I2C_SDA       4
#define TOUCH_I2C_SCL       5
#define TOUCH_INT           6
#define TOUCH_RST           7

// SD Card (SPI Mode / SDMMC)
#define SD_SPI_CS           1
#define SD_SPI_SCK          2
#define SD_SPI_MOSI         3
#define SD_SPI_MISO         8

// Display Resolution
#define SCREEN_WIDTH        410
#define SCREEN_HEIGHT       502

#endif // PIN_CONFIG_H
