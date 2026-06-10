/**
 * @file main.cpp
 * @brief ADXL380 C++ driver usage example.
 *
 * Demonstrates the RAII Accelerometer wrapper class with
 * exception handling. Uses stub SPI/delay functions.
 */

#include <cstdio>
#include <cstring>
#include <cstdint>
#include "../../lib/Cpp/adxl380_driver.hpp"

/* Platform stubs */
static int platform_spi_xfer(uint8_t *tx_buf, uint8_t *rx_buf,
                              uint16_t len, void * /*user_data*/)
{
    (void)tx_buf;
    if (rx_buf) {
        std::memset(rx_buf, 0, len);
    }
    return 0;
}

static void platform_delay_us(uint32_t /*us*/, void * /*user_data*/)
{
    /* Stub delay */
}

int main()
{
    std::printf("ADXL380 C++ Driver Example\n");
    std::printf("==========================\n\n");

    try {
        /* Create accelerometer instance with stub callbacks */
        adxl380::Accelerometer accel(platform_spi_xfer, platform_delay_us);

        /* Initialize */
        accel.init();

        /* Print device ID */
        uint8_t devid_ad, devid_mst, part_id;
        accel.getDeviceId(&devid_ad, &devid_mst, &part_id);
        std::printf("Device ID:  AD=0x%02X  MST=0x%02X  PART=0x%02X\n",
                    devid_ad, devid_mst, part_id);

        /* Configure */
        accel.setRange(ADXL380_RANGE_8G);
        std::printf("Range: +/-8g (sensitivity: %u LSB/g)\n",
                    accel.getSensitivity());

        accel.enableChannels(true, true, true, false);
        std::printf("Channels: X, Y, Z enabled\n");

        accel.setOpMode(ADXL380_MODE_HP);
        std::printf("Mode: High Performance\n\n");

        /* Read data */
        std::printf("Reading acceleration data...\n");
        for (int i = 0; i < 5; i++) {
            bool ready = accel.isDataReady();
            (void)ready;

            adxl380::AccelData data = accel.getAccelData();
            std::printf("  Sample %d: X=%6d  Y=%6d  Z=%6d\n",
                        i, data.x, data.y, data.z);
        }

        /* Temperature */
        float temp = accel.getTempCelsius();
        std::printf("\nTemperature: %.1f C\n", temp);

        /* Self-test */
        std::printf("\nRunning self-test...\n");
        bool passed = accel.selfTest();
        std::printf("Self-test: %s\n", passed ? "PASSED" : "FAILED");

        /* Standby (also done automatically by destructor) */
        accel.setOpMode(ADXL380_MODE_STANDBY);
        std::printf("\nDevice set to standby. Done.\n");

    } catch (const adxl380::DriverError &e) {
        std::printf("Driver error (status %d): %s\n", e.status(), e.what());
        std::printf("(Expected with stub SPI - replace with real HW)\n");
        return 1;
    }

    return 0;
}
