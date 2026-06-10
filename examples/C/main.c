/**
 * @file main.c
 * @brief ADXL380 C driver usage example.
 *
 * This example demonstrates initializing the ADXL380, configuring it,
 * and reading acceleration and temperature data. The SPI and delay
 * functions are stubs that must be replaced with platform-specific
 * implementations.
 */

#include <stdio.h>
#include <string.h>
#include "../../lib/C/adxl380_driver.h"

/* --- Platform stubs (replace with actual HW implementation) --- */

/**
 * @brief SPI transfer stub.
 *
 * In a real application, this would perform an actual SPI transaction.
 * The ADXL380 uses 16-bit half-duplex SPI (Mode 0, CPOL=0, CPHA=0).
 */
static int platform_spi_xfer(uint8_t *tx_buf, uint8_t *rx_buf,
                              uint16_t len, void *user_data)
{
    (void)tx_buf;
    (void)user_data;
    /* Stub: zero out receive buffer */
    if (rx_buf) {
        memset(rx_buf, 0, len);
    }
    return 0;
}

/**
 * @brief Microsecond delay stub.
 */
static void platform_delay_us(uint32_t us, void *user_data)
{
    (void)us;
    (void)user_data;
    /* Stub: in a real application, implement a microsecond delay */
}

/* --- Main --- */

int main(void)
{
    adxl380_dev_t dev;
    adxl380_status_t status;

    printf("ADXL380 C Driver Example\n");
    printf("========================\n\n");

    /* Set up device handle */
    memset(&dev, 0, sizeof(dev));
    dev.spi_xfer  = platform_spi_xfer;
    dev.delay_us  = platform_delay_us;
    dev.user_data = NULL;

    /* Initialize the device */
    status = adxl380_init(&dev);
    if (status != ADXL380_OK) {
        printf("Error: adxl380_init failed with status %d\n", status);
        printf("(Expected with stub SPI - replace with real HW)\n");
        return 1;
    }

    /* Read and print device ID */
    uint8_t devid_ad, devid_mst, part_id;
    status = adxl380_get_device_id(&dev, &devid_ad, &devid_mst, &part_id);
    if (status == ADXL380_OK) {
        printf("Device ID:  AD=0x%02X  MST=0x%02X  PART=0x%02X\n",
               devid_ad, devid_mst, part_id);
    }

    /* Configure range to ±8g */
    status = adxl380_set_range(&dev, ADXL380_RANGE_8G);
    if (status != ADXL380_OK) {
        printf("Error: set_range failed (%d)\n", status);
        return 1;
    }
    printf("Range set to +/-8g (sensitivity: %u LSB/g)\n",
           adxl380_get_sensitivity(ADXL380_RANGE_8G));

    /* Enable X, Y, Z channels */
    status = adxl380_enable_channels(&dev, true, true, true, false);
    if (status != ADXL380_OK) {
        printf("Error: enable_channels failed (%d)\n", status);
        return 1;
    }
    printf("Channels enabled: X, Y, Z\n");

    /* Enter High Performance mode */
    status = adxl380_set_op_mode(&dev, ADXL380_MODE_HP);
    if (status != ADXL380_OK) {
        printf("Error: set_op_mode failed (%d)\n", status);
        return 1;
    }
    printf("Operating mode: High Performance\n\n");

    /* Read acceleration data */
    printf("Reading acceleration data...\n");
    for (int i = 0; i < 5; i++) {
        bool ready = false;
        adxl380_is_data_ready(&dev, &ready);

        adxl380_accel_data_t accel;
        status = adxl380_get_accel_data(&dev, &accel);
        if (status == ADXL380_OK) {
            printf("  Sample %d: X=%6d  Y=%6d  Z=%6d\n",
                   i, accel.x, accel.y, accel.z);
        }
    }

    /* Read temperature */
    int16_t temp_raw;
    status = adxl380_get_temp_raw(&dev, &temp_raw);
    if (status == ADXL380_OK) {
        float temp_c = adxl380_convert_temp_c(temp_raw);
        printf("\nTemperature: raw=%d  %.1f C\n", temp_raw, temp_c);
    }

    /* Run self-test */
    printf("\nRunning self-test...\n");
    bool st_passed;
    status = adxl380_self_test(&dev, &st_passed);
    if (status == ADXL380_OK) {
        printf("Self-test: %s\n", st_passed ? "PASSED" : "FAILED");
    } else {
        printf("Self-test error: %d\n", status);
    }

    /* Return to standby */
    adxl380_set_op_mode(&dev, ADXL380_MODE_STANDBY);
    printf("\nDevice set to standby. Done.\n");

    return 0;
}
