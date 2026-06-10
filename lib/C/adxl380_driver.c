/**
 * @file adxl380_driver.c
 * @brief ADXL380 3-Axis Digital Accelerometer Driver Implementation
 *
 * Complete low-level C driver for the Analog Devices ADXL380 over SPI.
 * Implements every function declared in adxl380_driver.h.
 */

#include "adxl380_driver.h"

/* ────────────────────────────────────────────────────────────────────────── */
/*  Internal Helpers                                                         */
/* ────────────────────────────────────────────────────────────────────────── */

/** @brief Number of self-test averaging samples. */
#define ADXL380_ST_AVG_SAMPLES  4

/** @brief Data-ready polling timeout (iterations). */
#define ADXL380_DRDY_TIMEOUT    5000

/** @brief Delay between data-ready polls (microseconds). */
#define ADXL380_DRDY_POLL_US    100

/**
 * @brief Validate that the device handle and SPI callback are non-NULL.
 */
static adxl380_status_t validate_dev(const adxl380_dev_t *dev)
{
    if (dev == NULL || dev->spi_xfer == NULL) {
        return ADXL380_ERR_PARAM;
    }
    return ADXL380_OK;
}

/**
 * @brief Compute the absolute value of a 32-bit signed integer.
 */
static int32_t abs32(int32_t v)
{
    return (v < 0) ? -v : v;
}

/**
 * @brief Wait until the data-ready flag in STATUS3 is set.
 *
 * Polls up to ADXL380_DRDY_TIMEOUT times with ADXL380_DRDY_POLL_US between.
 */
static adxl380_status_t wait_data_ready(const adxl380_dev_t *dev)
{
    uint16_t i;
    bool ready = false;
    adxl380_status_t st;

    for (i = 0; i < ADXL380_DRDY_TIMEOUT; i++) {
        st = adxl380_is_data_ready(dev, &ready);
        if (st != ADXL380_OK) {
            return st;
        }
        if (ready) {
            return ADXL380_OK;
        }
        if (dev->delay_us != NULL) {
            dev->delay_us(ADXL380_DRDY_POLL_US, dev->user_data);
        }
    }
    return ADXL380_ERR_TIMEOUT;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Core Register Access                                                     */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Read a single register over SPI.
 *
 * Builds a 2-byte half-duplex frame: address byte with R/W=1, then a
 * dummy TX byte.  The device returns the register value in RX byte 1.
 */
adxl380_status_t adxl380_read_reg(const adxl380_dev_t *dev, uint8_t reg,
                                  uint8_t *val)
{
    uint8_t tx[2];
    uint8_t rx[2];

    if (dev == NULL || dev->spi_xfer == NULL || val == NULL) {
        return ADXL380_ERR_PARAM;
    }

    tx[0] = (uint8_t)((reg << ADXL380_SPI_ADDR_SHIFT) | ADXL380_SPI_READ_BIT);
    tx[1] = 0x00;

    if (dev->spi_xfer(tx, rx, 2, dev->user_data) != 0) {
        return ADXL380_ERR_SPI;
    }

    *val = rx[1];
    return ADXL380_OK;
}

/**
 * @brief Write a single register over SPI.
 *
 * Builds a 2-byte frame: address byte with R/W=0, then the data byte.
 */
adxl380_status_t adxl380_write_reg(const adxl380_dev_t *dev, uint8_t reg,
                                   uint8_t val)
{
    uint8_t tx[2];
    uint8_t rx[2];

    if (dev == NULL || dev->spi_xfer == NULL) {
        return ADXL380_ERR_PARAM;
    }

    tx[0] = (uint8_t)((reg << ADXL380_SPI_ADDR_SHIFT) | ADXL380_SPI_WRITE_BIT);
    tx[1] = val;

    if (dev->spi_xfer(tx, rx, 2, dev->user_data) != 0) {
        return ADXL380_ERR_SPI;
    }

    return ADXL380_OK;
}

/**
 * @brief Burst-read multiple consecutive registers.
 *
 * Reads one register at a time in a loop to avoid dynamic allocation
 * and keep the implementation fully portable for embedded targets.
 */
adxl380_status_t adxl380_read_regs(const adxl380_dev_t *dev, uint8_t reg,
                                   uint8_t *buf, uint16_t len)
{
    uint16_t i;
    adxl380_status_t st;

    if (dev == NULL || dev->spi_xfer == NULL || buf == NULL) {
        return ADXL380_ERR_PARAM;
    }

    for (i = 0; i < len; i++) {
        st = adxl380_read_reg(dev, (uint8_t)(reg + i), &buf[i]);
        if (st != ADXL380_OK) {
            return st;
        }
    }

    return ADXL380_OK;
}

/**
 * @brief Burst-write multiple consecutive registers.
 *
 * Writes one register at a time in a loop for portability.
 */
adxl380_status_t adxl380_write_regs(const adxl380_dev_t *dev, uint8_t reg,
                                    const uint8_t *buf, uint16_t len)
{
    uint16_t i;
    adxl380_status_t st;

    if (dev == NULL || dev->spi_xfer == NULL || buf == NULL) {
        return ADXL380_ERR_PARAM;
    }

    for (i = 0; i < len; i++) {
        st = adxl380_write_reg(dev, (uint8_t)(reg + i), buf[i]);
        if (st != ADXL380_OK) {
            return st;
        }
    }

    return ADXL380_OK;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Initialisation & Reset                                                   */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Issue a software reset by writing the reset code to REG_RESET.
 */
adxl380_status_t adxl380_soft_reset(const adxl380_dev_t *dev)
{
    return adxl380_write_reg(dev, ADXL380_REG_REG_RESET,
                             ADXL380_REG_RESET_CODE);
}

/**
 * @brief Initialise the ADXL380 device.
 *
 * Performs a soft reset, waits the startup delay, verifies the three
 * identification registers, and sets default range / operating mode.
 */
adxl380_status_t adxl380_init(adxl380_dev_t *dev)
{
    adxl380_status_t st;
    uint8_t id;

    if (dev == NULL || dev->spi_xfer == NULL || dev->delay_us == NULL) {
        return ADXL380_ERR_PARAM;
    }

    /* Soft reset the device */
    st = adxl380_soft_reset(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    /* Wait for reset + NVM refresh */
    dev->delay_us(ADXL380_STARTUP_DELAY_US, dev->user_data);

    /* Verify DEVID_AD == 0xAD */
    st = adxl380_read_reg(dev, ADXL380_REG_DEVID_AD, &id);
    if (st != ADXL380_OK) {
        return st;
    }
    if (id != ADXL380_RESET_DEVID_AD) {
        return ADXL380_ERR_DEVICE;
    }

    /* Verify DEVID_MST == 0x1D */
    st = adxl380_read_reg(dev, ADXL380_REG_DEVID_MST, &id);
    if (st != ADXL380_OK) {
        return st;
    }
    if (id != ADXL380_RESET_DEVID_MST) {
        return ADXL380_ERR_DEVICE;
    }

    /* Verify PART_ID == 0x17 */
    st = adxl380_read_reg(dev, ADXL380_REG_PART_ID, &id);
    if (st != ADXL380_OK) {
        return st;
    }
    if (id != ADXL380_RESET_PART_ID) {
        return ADXL380_ERR_DEVICE;
    }

    /* Set default range and mode in the device handle */
    dev->range   = ADXL380_RANGE_4G;
    dev->op_mode = ADXL380_MODE_STANDBY;

    return ADXL380_OK;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Configuration                                                            */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Set the operating mode (bits[3:0] of OP_MODE register).
 *
 * Reads the register, clears the mode bits, sets the new mode, and
 * caches the value in the device handle.
 */
adxl380_status_t adxl380_set_op_mode(adxl380_dev_t *dev,
                                     adxl380_op_mode_t mode)
{
    uint8_t val;
    adxl380_status_t st;

    if (dev == NULL || dev->spi_xfer == NULL) {
        return ADXL380_ERR_PARAM;
    }

    st = adxl380_read_reg(dev, ADXL380_REG_OP_MODE, &val);
    if (st != ADXL380_OK) {
        return st;
    }

    val = (uint8_t)((val & (uint8_t)~ADXL380_OP_MODE_MODE_MSK) |
                    ((uint8_t)mode & ADXL380_OP_MODE_MODE_MSK));

    st = adxl380_write_reg(dev, ADXL380_REG_OP_MODE, val);
    if (st != ADXL380_OK) {
        return st;
    }

    dev->op_mode = mode;
    return ADXL380_OK;
}

/**
 * @brief Read the current operating mode from the OP_MODE register.
 */
adxl380_status_t adxl380_get_op_mode(const adxl380_dev_t *dev,
                                     adxl380_op_mode_t *mode)
{
    uint8_t val;
    adxl380_status_t st;

    if (dev == NULL || dev->spi_xfer == NULL || mode == NULL) {
        return ADXL380_ERR_PARAM;
    }

    st = adxl380_read_reg(dev, ADXL380_REG_OP_MODE, &val);
    if (st != ADXL380_OK) {
        return st;
    }

    *mode = (adxl380_op_mode_t)(val & ADXL380_OP_MODE_MODE_MSK);
    return ADXL380_OK;
}

/**
 * @brief Set the measurement range (bits[7:6] of OP_MODE register).
 *
 * Validates the range, performs a read-modify-write on OP_MODE, and
 * caches the new range in the device handle.
 */
adxl380_status_t adxl380_set_range(adxl380_dev_t *dev,
                                   adxl380_range_t range)
{
    uint8_t val;
    adxl380_status_t st;

    if (dev == NULL || dev->spi_xfer == NULL) {
        return ADXL380_ERR_PARAM;
    }

    /* Only 0, 1, 2 are valid range codes */
    if ((uint8_t)range > (uint8_t)ADXL380_RANGE_16G) {
        return ADXL380_ERR_PARAM;
    }

    st = adxl380_read_reg(dev, ADXL380_REG_OP_MODE, &val);
    if (st != ADXL380_OK) {
        return st;
    }

    val = (uint8_t)((val & (uint8_t)~ADXL380_OP_MODE_RANGE_MSK) |
                    ((uint8_t)range << ADXL380_OP_MODE_RANGE_POS));

    st = adxl380_write_reg(dev, ADXL380_REG_OP_MODE, val);
    if (st != ADXL380_OK) {
        return st;
    }

    dev->range = range;
    return ADXL380_OK;
}

/**
 * @brief Read the current measurement range from OP_MODE bits[7:6].
 */
adxl380_status_t adxl380_get_range(const adxl380_dev_t *dev,
                                   adxl380_range_t *range)
{
    uint8_t val;
    adxl380_status_t st;

    if (dev == NULL || dev->spi_xfer == NULL || range == NULL) {
        return ADXL380_ERR_PARAM;
    }

    st = adxl380_read_reg(dev, ADXL380_REG_OP_MODE, &val);
    if (st != ADXL380_OK) {
        return st;
    }

    *range = (adxl380_range_t)((val & ADXL380_OP_MODE_RANGE_MSK) >>
                               ADXL380_OP_MODE_RANGE_POS);
    return ADXL380_OK;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Channel Enable                                                           */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Enable or disable X, Y, Z, and temperature measurement channels.
 *
 * Performs a read-modify-write on the DIG_EN register, setting bits[7:4]
 * for Temp, Z, Y, X enables while preserving the lower nibble.
 */
adxl380_status_t adxl380_enable_channels(const adxl380_dev_t *dev,
                                         bool x, bool y, bool z, bool temp)
{
    uint8_t val;
    adxl380_status_t st;

    st = validate_dev(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_read_reg(dev, ADXL380_REG_DIG_EN, &val);
    if (st != ADXL380_OK) {
        return st;
    }

    /* Clear channel enable bits [7:4] */
    val &= 0x0F;

    /* Set requested channels */
    if (x) {
        val |= ADXL380_DIG_EN_X_EN;
    }
    if (y) {
        val |= ADXL380_DIG_EN_Y_EN;
    }
    if (z) {
        val |= ADXL380_DIG_EN_Z_EN;
    }
    if (temp) {
        val |= ADXL380_DIG_EN_TEMP_EN;
    }

    return adxl380_write_reg(dev, ADXL380_REG_DIG_EN, val);
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Data Acquisition                                                         */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Read raw 16-bit acceleration data for all three axes.
 *
 * Reads 6 consecutive bytes from XDATA_H through ZDATA_L and combines
 * each high/low pair into a signed 16-bit value (big-endian on-wire).
 */
adxl380_status_t adxl380_get_accel_data(const adxl380_dev_t *dev,
                                        adxl380_accel_data_t *data)
{
    uint8_t buf[6];
    adxl380_status_t st;

    if (dev == NULL || dev->spi_xfer == NULL || data == NULL) {
        return ADXL380_ERR_PARAM;
    }

    st = adxl380_read_regs(dev, ADXL380_REG_XDATA_H, buf, 6);
    if (st != ADXL380_OK) {
        return st;
    }

    data->x = (int16_t)((uint16_t)((uint16_t)buf[0] << 8) | buf[1]);
    data->y = (int16_t)((uint16_t)((uint16_t)buf[2] << 8) | buf[3]);
    data->z = (int16_t)((uint16_t)((uint16_t)buf[4] << 8) | buf[5]);

    return ADXL380_OK;
}

/**
 * @brief Read the raw 12-bit temperature value.
 *
 * Reads TDATA_H and TDATA_L, combines into a 12-bit twos-complement
 * value, and sign-extends to 16 bits.
 */
adxl380_status_t adxl380_get_temp_raw(const adxl380_dev_t *dev,
                                      int16_t *raw_temp)
{
    uint8_t buf[2];
    adxl380_status_t st;
    int16_t val;

    if (dev == NULL || dev->spi_xfer == NULL || raw_temp == NULL) {
        return ADXL380_ERR_PARAM;
    }

    st = adxl380_read_regs(dev, ADXL380_REG_TDATA_H, buf, 2);
    if (st != ADXL380_OK) {
        return st;
    }

    /* 12-bit value: high byte is bits[11:4], low byte upper nibble is bits[3:0] */
    val = (int16_t)(((uint16_t)buf[0] << 4) | ((uint16_t)buf[1] >> 4));

    /* Sign-extend from 12-bit */
    if (val & 0x0800) {
        val |= (int16_t)0xF000;
    }

    *raw_temp = val;
    return ADXL380_OK;
}

/**
 * @brief Convert a raw 12-bit temperature reading to degrees Celsius.
 *
 * Uses the formula: T(°C) = 25 + (raw - OFFSET_AT_25C) / SENSITIVITY
 */
float adxl380_convert_temp_c(int16_t raw_temp)
{
    return 25.0f + ((float)(raw_temp - ADXL380_TEMP_OFFSET_AT_25C) /
                    ADXL380_TEMP_SENSITIVITY);
}

/**
 * @brief Check the data-ready flag in STATUS3.
 */
adxl380_status_t adxl380_is_data_ready(const adxl380_dev_t *dev, bool *ready)
{
    uint8_t val;
    adxl380_status_t st;

    if (dev == NULL || dev->spi_xfer == NULL || ready == NULL) {
        return ADXL380_ERR_PARAM;
    }

    st = adxl380_read_reg(dev, ADXL380_REG_STATUS3, &val);
    if (st != ADXL380_OK) {
        return st;
    }

    *ready = (val & ADXL380_STATUS3_DATA_READY) != 0;
    return ADXL380_OK;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Sensitivity Helper                                                       */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Return the sensitivity in LSB/g for the given range setting.
 */
uint16_t adxl380_get_sensitivity(adxl380_range_t range)
{
    switch (range) {
    case ADXL380_RANGE_4G:
        return ADXL380_SENSITIVITY_4G;
    case ADXL380_RANGE_8G:
        return ADXL380_SENSITIVITY_8G;
    case ADXL380_RANGE_16G:
        return ADXL380_SENSITIVITY_16G;
    default:
        return 0;
    }
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Self-Test                                                                */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Execute the built-in self-test and report pass/fail.
 *
 * Saves current config, switches to ±8g / HP mode, reads baseline data,
 * applies positive then negative self-test forces, computes deltas in mg,
 * and compares against datasheet limits.  Restores original config on exit.
 */
adxl380_status_t adxl380_self_test(adxl380_dev_t *dev, bool *passed)
{
    adxl380_status_t st;
    adxl380_op_mode_t saved_mode;
    adxl380_range_t saved_range;
    adxl380_accel_data_t sample;
    int32_t pos_x = 0, pos_y = 0, pos_z = 0;
    int32_t neg_x = 0, neg_y = 0, neg_z = 0;
    int32_t delta_x, delta_y, delta_z;
    int32_t delta_mg_x, delta_mg_y, delta_mg_z;
    uint8_t axis_en;
    uint16_t sensitivity;
    uint8_t i;
    bool x_pass, y_pass, z_pass;

    if (dev == NULL || dev->spi_xfer == NULL || passed == NULL) {
        return ADXL380_ERR_PARAM;
    }

    *passed = false;

    /* Save current configuration */
    saved_mode  = dev->op_mode;
    saved_range = dev->range;

    /* 1. Go to standby */
    st = adxl380_set_op_mode(dev, ADXL380_MODE_STANDBY);
    if (st != ADXL380_OK) {
        return st;
    }

    /* 2. Set range to ±8g */
    st = adxl380_set_range(dev, ADXL380_RANGE_8G);
    if (st != ADXL380_OK) {
        goto restore;
    }

    /* 3. Enable XYZ channels */
    st = adxl380_enable_channels(dev, true, true, true, false);
    if (st != ADXL380_OK) {
        goto restore;
    }

    /* 4. Set HP mode for measurement */
    st = adxl380_set_op_mode(dev, ADXL380_MODE_HP);
    if (st != ADXL380_OK) {
        goto restore;
    }

    /* 5. Discard initial samples so the sensor settles in HP mode */
    for (i = 0; i < ADXL380_ST_AVG_SAMPLES; i++) {
        st = wait_data_ready(dev);
        if (st != ADXL380_OK) {
            goto restore;
        }
        st = adxl380_get_accel_data(dev, &sample);
        if (st != ADXL380_OK) {
            goto restore;
        }
    }

    /* 6. Apply positive self-test (ST_DIR = 0) — read-modify-write */
    st = adxl380_read_reg(dev, ADXL380_REG_SNSR_AXIS_EN, &axis_en);
    if (st != ADXL380_OK) {
        goto restore;
    }
    axis_en &= (uint8_t)~(ADXL380_SNSR_AXIS_EN_ST_MODE |
                           ADXL380_SNSR_AXIS_EN_ST_FORCE |
                           ADXL380_SNSR_AXIS_EN_ST_DIR);
    axis_en |= (ADXL380_SNSR_AXIS_EN_ST_MODE | ADXL380_SNSR_AXIS_EN_ST_FORCE);
    st = adxl380_write_reg(dev, ADXL380_REG_SNSR_AXIS_EN, axis_en);
    if (st != ADXL380_OK) {
        goto restore;
    }

    /* Read positive self-test data (averaged) */
    for (i = 0; i < ADXL380_ST_AVG_SAMPLES; i++) {
        st = wait_data_ready(dev);
        if (st != ADXL380_OK) {
            goto restore;
        }
        st = adxl380_get_accel_data(dev, &sample);
        if (st != ADXL380_OK) {
            goto restore;
        }
        pos_x += sample.x;
        pos_y += sample.y;
        pos_z += sample.z;
    }
    pos_x /= ADXL380_ST_AVG_SAMPLES;
    pos_y /= ADXL380_ST_AVG_SAMPLES;
    pos_z /= ADXL380_ST_AVG_SAMPLES;

    /* 7. Apply negative self-test (ST_DIR = 1) */
    st = adxl380_read_reg(dev, ADXL380_REG_SNSR_AXIS_EN, &axis_en);
    if (st != ADXL380_OK) {
        goto restore;
    }
    axis_en &= (uint8_t)~(ADXL380_SNSR_AXIS_EN_ST_MODE |
                           ADXL380_SNSR_AXIS_EN_ST_FORCE |
                           ADXL380_SNSR_AXIS_EN_ST_DIR);
    axis_en |= (ADXL380_SNSR_AXIS_EN_ST_MODE |
                ADXL380_SNSR_AXIS_EN_ST_FORCE |
                ADXL380_SNSR_AXIS_EN_ST_DIR);
    st = adxl380_write_reg(dev, ADXL380_REG_SNSR_AXIS_EN, axis_en);
    if (st != ADXL380_OK) {
        goto restore;
    }

    /* Read negative self-test data (averaged) */
    for (i = 0; i < ADXL380_ST_AVG_SAMPLES; i++) {
        st = wait_data_ready(dev);
        if (st != ADXL380_OK) {
            goto restore;
        }
        st = adxl380_get_accel_data(dev, &sample);
        if (st != ADXL380_OK) {
            goto restore;
        }
        neg_x += sample.x;
        neg_y += sample.y;
        neg_z += sample.z;
    }
    neg_x /= ADXL380_ST_AVG_SAMPLES;
    neg_y /= ADXL380_ST_AVG_SAMPLES;
    neg_z /= ADXL380_ST_AVG_SAMPLES;

    /* 8. Disable self-test — clear ST_MODE, ST_FORCE, ST_DIR */
    st = adxl380_read_reg(dev, ADXL380_REG_SNSR_AXIS_EN, &axis_en);
    if (st != ADXL380_OK) {
        goto restore;
    }
    axis_en &= (uint8_t)~(ADXL380_SNSR_AXIS_EN_ST_MODE |
                           ADXL380_SNSR_AXIS_EN_ST_FORCE |
                           ADXL380_SNSR_AXIS_EN_ST_DIR);
    st = adxl380_write_reg(dev, ADXL380_REG_SNSR_AXIS_EN, axis_en);
    if (st != ADXL380_OK) {
        goto restore;
    }

    /* 9. Compute delta = positive - negative for each axis */
    delta_x = pos_x - neg_x;
    delta_y = pos_y - neg_y;
    delta_z = pos_z - neg_z;

    /* Convert delta to mg: delta_mg = (|delta| * 1000) / sensitivity_8g */
    sensitivity = ADXL380_SENSITIVITY_8G;
    delta_mg_x = (abs32(delta_x) * 1000) / sensitivity;
    delta_mg_y = (abs32(delta_y) * 1000) / sensitivity;
    delta_mg_z = (abs32(delta_z) * 1000) / sensitivity;

    /* 10. Check against datasheet limits */
    x_pass = (delta_mg_x >= ADXL380_SELF_TEST_XY_MIN_MG) &&
             (delta_mg_x <= ADXL380_SELF_TEST_XY_MAX_MG);
    y_pass = (delta_mg_y >= ADXL380_SELF_TEST_XY_MIN_MG) &&
             (delta_mg_y <= ADXL380_SELF_TEST_XY_MAX_MG);
    z_pass = (delta_mg_z >= ADXL380_SELF_TEST_Z_MIN_MG) &&
             (delta_mg_z <= ADXL380_SELF_TEST_Z_MAX_MG);

    *passed = (x_pass && y_pass && z_pass);

restore:
    /* Restore original range and mode */
    adxl380_set_range(dev, saved_range);
    adxl380_set_op_mode(dev, saved_mode);

    return st;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  FIFO                                                                     */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Set the FIFO operating mode (bits[1:0] of FIFO_CFG0).
 */
adxl380_status_t adxl380_set_fifo_mode(const adxl380_dev_t *dev,
                                       adxl380_fifo_mode_t mode)
{
    uint8_t val;
    adxl380_status_t st;

    st = validate_dev(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_read_reg(dev, ADXL380_REG_FIFO_CFG0, &val);
    if (st != ADXL380_OK) {
        return st;
    }

    val = (uint8_t)((val & 0xFC) | ((uint8_t)mode & 0x03));

    return adxl380_write_reg(dev, ADXL380_REG_FIFO_CFG0, val);
}

/**
 * @brief Read the number of valid FIFO entries (9-bit count).
 *
 * Combines FIFO_STATUS0 (lower 8 bits) and FIFO_STATUS1 bit 0 (bit 8).
 */
adxl380_status_t adxl380_get_fifo_entries(const adxl380_dev_t *dev,
                                          uint16_t *entries)
{
    uint8_t s0, s1;
    adxl380_status_t st;

    if (dev == NULL || dev->spi_xfer == NULL || entries == NULL) {
        return ADXL380_ERR_PARAM;
    }

    st = adxl380_read_reg(dev, ADXL380_REG_FIFO_STATUS0, &s0);
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_read_reg(dev, ADXL380_REG_FIFO_STATUS1, &s1);
    if (st != ADXL380_OK) {
        return st;
    }

    *entries = ((uint16_t)(s1 & 0x01) << 8) | (uint16_t)s0;
    return ADXL380_OK;
}

/**
 * @brief Read data bytes from the FIFO data register.
 *
 * FIFO reads address the same register repeatedly (no auto-increment).
 */
adxl380_status_t adxl380_read_fifo(const adxl380_dev_t *dev,
                                   uint8_t *buf, uint16_t len)
{
    uint16_t i;
    adxl380_status_t st;

    if (dev == NULL || dev->spi_xfer == NULL || buf == NULL) {
        return ADXL380_ERR_PARAM;
    }

    for (i = 0; i < len; i++) {
        st = adxl380_read_reg(dev, ADXL380_REG_FIFO_DATA, &buf[i]);
        if (st != ADXL380_OK) {
            return st;
        }
    }

    return ADXL380_OK;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Interrupt Configuration                                                  */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Configure interrupt pin mapping registers.
 *
 * int_num 0 → INT0_MAP0 / INT0_MAP1, int_num 1 → INT1_MAP0 / INT1_MAP1.
 */
adxl380_status_t adxl380_set_int_map(const adxl380_dev_t *dev,
                                     uint8_t int_num, uint8_t map0,
                                     uint8_t map1)
{
    adxl380_status_t st;
    uint8_t reg_map0, reg_map1;

    st = validate_dev(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    if (int_num == 0) {
        reg_map0 = ADXL380_REG_INT0_MAP0;
        reg_map1 = ADXL380_REG_INT0_MAP1;
    } else if (int_num == 1) {
        reg_map0 = ADXL380_REG_INT1_MAP0;
        reg_map1 = ADXL380_REG_INT1_MAP1;
    } else {
        return ADXL380_ERR_PARAM;
    }

    st = adxl380_write_reg(dev, reg_map0, map0);
    if (st != ADXL380_OK) {
        return st;
    }

    return adxl380_write_reg(dev, reg_map1, map1);
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Activity / Inactivity Detection                                          */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Set the 16-bit activity threshold (high and low bytes).
 */
adxl380_status_t adxl380_set_act_threshold(const adxl380_dev_t *dev,
                                           uint16_t threshold)
{
    adxl380_status_t st;

    st = validate_dev(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_write_reg(dev, ADXL380_REG_THRESH_ACT_H,
                           (uint8_t)(threshold >> 8));
    if (st != ADXL380_OK) {
        return st;
    }

    return adxl380_write_reg(dev, ADXL380_REG_THRESH_ACT_L,
                             (uint8_t)(threshold & 0xFF));
}

/**
 * @brief Set the 16-bit inactivity threshold (high and low bytes).
 */
adxl380_status_t adxl380_set_inact_threshold(const adxl380_dev_t *dev,
                                             uint16_t threshold)
{
    adxl380_status_t st;

    st = validate_dev(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_write_reg(dev, ADXL380_REG_THRESH_INACT_H,
                           (uint8_t)(threshold >> 8));
    if (st != ADXL380_OK) {
        return st;
    }

    return adxl380_write_reg(dev, ADXL380_REG_THRESH_INACT_L,
                             (uint8_t)(threshold & 0xFF));
}

/**
 * @brief Set the 24-bit activity time across three registers (H, M, L).
 */
adxl380_status_t adxl380_set_act_time(const adxl380_dev_t *dev, uint32_t time)
{
    adxl380_status_t st;

    st = validate_dev(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_write_reg(dev, ADXL380_REG_TIME_ACT_H,
                           (uint8_t)((time >> 16) & 0xFF));
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_write_reg(dev, ADXL380_REG_TIME_ACT_M,
                           (uint8_t)((time >> 8) & 0xFF));
    if (st != ADXL380_OK) {
        return st;
    }

    return adxl380_write_reg(dev, ADXL380_REG_TIME_ACT_L,
                             (uint8_t)(time & 0xFF));
}

/**
 * @brief Set the 24-bit inactivity time across three registers (H, M, L).
 */
adxl380_status_t adxl380_set_inact_time(const adxl380_dev_t *dev,
                                        uint32_t time)
{
    adxl380_status_t st;

    st = validate_dev(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_write_reg(dev, ADXL380_REG_TIME_INACT_H,
                           (uint8_t)((time >> 16) & 0xFF));
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_write_reg(dev, ADXL380_REG_TIME_INACT_M,
                           (uint8_t)((time >> 8) & 0xFF));
    if (st != ADXL380_OK) {
        return st;
    }

    return adxl380_write_reg(dev, ADXL380_REG_TIME_INACT_L,
                             (uint8_t)(time & 0xFF));
}

/**
 * @brief Write the activity/inactivity control register directly.
 */
adxl380_status_t adxl380_set_act_inact_ctl(const adxl380_dev_t *dev,
                                           uint8_t ctl)
{
    adxl380_status_t st;

    st = validate_dev(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    return adxl380_write_reg(dev, ADXL380_REG_ACT_INACT_CTL, ctl);
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Tap Detection                                                            */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Configure all five tap-detection registers in one call.
 */
adxl380_status_t adxl380_set_tap_config(const adxl380_dev_t *dev,
                                        uint8_t thresh, uint8_t dur,
                                        uint8_t latent, uint8_t window,
                                        uint8_t cfg)
{
    adxl380_status_t st;

    st = validate_dev(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_write_reg(dev, ADXL380_REG_TAP_THRESH, thresh);
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_write_reg(dev, ADXL380_REG_TAP_DUR, dur);
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_write_reg(dev, ADXL380_REG_TAP_LATENT, latent);
    if (st != ADXL380_OK) {
        return st;
    }

    st = adxl380_write_reg(dev, ADXL380_REG_TAP_WINDOW, window);
    if (st != ADXL380_OK) {
        return st;
    }

    return adxl380_write_reg(dev, ADXL380_REG_TAP_CFG, cfg);
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Status                                                                   */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Read all four status registers.  Any output pointer may be NULL.
 */
adxl380_status_t adxl380_get_status(const adxl380_dev_t *dev,
                                    uint8_t *status0, uint8_t *status1,
                                    uint8_t *status2, uint8_t *status3)
{
    uint8_t val;
    adxl380_status_t st;

    st = validate_dev(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    if (status0 != NULL) {
        st = adxl380_read_reg(dev, ADXL380_REG_STATUS0, &val);
        if (st != ADXL380_OK) {
            return st;
        }
        *status0 = val;
    }

    if (status1 != NULL) {
        st = adxl380_read_reg(dev, ADXL380_REG_STATUS1, &val);
        if (st != ADXL380_OK) {
            return st;
        }
        *status1 = val;
    }

    if (status2 != NULL) {
        st = adxl380_read_reg(dev, ADXL380_REG_STATUS2, &val);
        if (st != ADXL380_OK) {
            return st;
        }
        *status2 = val;
    }

    if (status3 != NULL) {
        st = adxl380_read_reg(dev, ADXL380_REG_STATUS3, &val);
        if (st != ADXL380_OK) {
            return st;
        }
        *status3 = val;
    }

    return ADXL380_OK;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Device Identification                                                    */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Read the three device identification registers.
 *
 * Any output pointer may be NULL to skip that particular register.
 */
adxl380_status_t adxl380_get_device_id(const adxl380_dev_t *dev,
                                       uint8_t *devid_ad, uint8_t *devid_mst,
                                       uint8_t *part_id)
{
    uint8_t val;
    adxl380_status_t st;

    st = validate_dev(dev);
    if (st != ADXL380_OK) {
        return st;
    }

    if (devid_ad != NULL) {
        st = adxl380_read_reg(dev, ADXL380_REG_DEVID_AD, &val);
        if (st != ADXL380_OK) {
            return st;
        }
        *devid_ad = val;
    }

    if (devid_mst != NULL) {
        st = adxl380_read_reg(dev, ADXL380_REG_DEVID_MST, &val);
        if (st != ADXL380_OK) {
            return st;
        }
        *devid_mst = val;
    }

    if (part_id != NULL) {
        st = adxl380_read_reg(dev, ADXL380_REG_PART_ID, &val);
        if (st != ADXL380_OK) {
            return st;
        }
        *part_id = val;
    }

    return ADXL380_OK;
}
