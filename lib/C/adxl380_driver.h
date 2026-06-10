/**
 * @file adxl380_driver.h
 * @brief ADXL380 Digital Accelerometer Driver — Register Map & API
 *
 * Complete register definitions, bit fields, enumerations, and function
 * declarations for the Analog Devices ADXL380 3-axis digital accelerometer.
 */

#ifndef ADXL380_DRIVER_H
#define ADXL380_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ────────────────────────────────────────────────────────────────────────── */
/*  Register Map                                                            */
/* ────────────────────────────────────────────────────────────────────────── */

/** @defgroup reg_map ADXL380 Register Addresses
 *  @{ */

/* Identification registers */
#define ADXL380_REG_DEVID_AD            0x00  /**< Analog Devices ID (reset: 0xAD) */
#define ADXL380_REG_DEVID_MST           0x01  /**< MEMS ID (reset: 0x1D)           */
#define ADXL380_REG_PART_ID             0x02  /**< Part ID (reset: 0x17)           */
#define ADXL380_REG_PART_ID_REV_ID      0x03  /**< Part/Rev ID (reset: 0xC3)       */

/* Serial number registers */
#define ADXL380_REG_SN0                 0x04  /**< Serial number byte 0 */
#define ADXL380_REG_SN1                 0x05  /**< Serial number byte 1 */
#define ADXL380_REG_SN2                 0x06  /**< Serial number byte 2 */
#define ADXL380_REG_SN3                 0x07  /**< Serial number byte 3 */
#define ADXL380_REG_SN4                 0x08  /**< Serial number byte 4 */
#define ADXL380_REG_SN5                 0x09  /**< Serial number byte 5 */
#define ADXL380_REG_SN6                 0x0A  /**< Serial number byte 6 */

/* Device delta/trim registers */
#define ADXL380_REG_DEV_DELTA_Q_X       0x0B  /**< Delta Q X trim */
#define ADXL380_REG_DEV_DELTA_Q_Y       0x0C  /**< Delta Q Y trim */
#define ADXL380_REG_DEV_DELTA_Q_Z       0x0D  /**< Delta Q Z trim */
#define ADXL380_REG_DEV_DELTA_F0_X      0x0E  /**< Delta F0 X trim */
#define ADXL380_REG_DEV_DELTA_F0_Y      0x0F  /**< Delta F0 Y trim */
#define ADXL380_REG_DEV_DELTA_F0_Z      0x10  /**< Delta F0 Z trim */

/* Status registers */
#define ADXL380_REG_STATUS0             0x11  /**< Status 0 (reset: 0x80) */
#define ADXL380_REG_STATUS1             0x12  /**< Status 1 */
#define ADXL380_REG_STATUS2             0x13  /**< Status 2 (reset: 0x04) */
#define ADXL380_REG_STATUS3             0x14  /**< Status 3 */

/* Acceleration data registers (16-bit, MSB first) */
#define ADXL380_REG_XDATA_H             0x15  /**< X-axis data high byte */
#define ADXL380_REG_XDATA_L             0x16  /**< X-axis data low byte  */
#define ADXL380_REG_YDATA_H             0x17  /**< Y-axis data high byte */
#define ADXL380_REG_YDATA_L             0x18  /**< Y-axis data low byte  */
#define ADXL380_REG_ZDATA_H             0x19  /**< Z-axis data high byte */
#define ADXL380_REG_ZDATA_L             0x1A  /**< Z-axis data low byte  */

/* Temperature data registers (16-bit, MSB first) */
#define ADXL380_REG_TDATA_H             0x1B  /**< Temperature data high byte */
#define ADXL380_REG_TDATA_L             0x1C  /**< Temperature data low byte  */

/* FIFO data and status */
#define ADXL380_REG_FIFO_DATA           0x1D  /**< FIFO data output port  */
#define ADXL380_REG_FIFO_STATUS0        0x1E  /**< FIFO status byte 0     */
#define ADXL380_REG_FIFO_STATUS1        0x1F  /**< FIFO status byte 1     */

/* Miscellaneous configuration */
#define ADXL380_REG_MISC0               0x20  /**< Misc config 0 */
#define ADXL380_REG_MISC1               0x21  /**< Misc config 1 */

/* Sensor / clock / mode configuration */
#define ADXL380_REG_SENS_DSM            0x24  /**< Sensor DSM config    */
#define ADXL380_REG_CLK_CTRL            0x25  /**< Clock control        */
#define ADXL380_REG_OP_MODE             0x26  /**< Operating mode       */
#define ADXL380_REG_DIG_EN              0x27  /**< Digital enables       */
#define ADXL380_REG_SAR_I2C             0x28  /**< SAR / I2C config     */
#define ADXL380_REG_NVM_CTL             0x29  /**< NVM control          */
#define ADXL380_REG_REG_RESET           0x2A  /**< Software reset       */

/* Interrupt mapping */
#define ADXL380_REG_INT0_MAP0           0x2B  /**< INT0 mapping byte 0 */
#define ADXL380_REG_INT0_MAP1           0x2C  /**< INT0 mapping byte 1 */
#define ADXL380_REG_INT1_MAP0           0x2D  /**< INT1 mapping byte 0 */
#define ADXL380_REG_INT1_MAP1           0x2E  /**< INT1 mapping byte 1 */

/* FIFO configuration */
#define ADXL380_REG_FIFO_CFG0           0x30  /**< FIFO config byte 0 */
#define ADXL380_REG_FIFO_CFG1           0x31  /**< FIFO config byte 1 */

/* Signal path / timing / sync / PDM */
#define ADXL380_REG_SPT_CFG0            0x32  /**< Signal path config 0 */
#define ADXL380_REG_SPT_CFG1            0x33  /**< Signal path config 1 */
#define ADXL380_REG_SPT_CFG2            0x34  /**< Signal path config 2 */
#define ADXL380_REG_SYNC_CFG            0x35  /**< Sync configuration   */
#define ADXL380_REG_PDM_CFG             0x36  /**< PDM configuration    */

/* Activity / Inactivity */
#define ADXL380_REG_ACT_INACT_CTL       0x37  /**< Activity/Inactivity control */
#define ADXL380_REG_SNSR_AXIS_EN        0x38  /**< Sensor axis enable / self-test */
#define ADXL380_REG_THRESH_ACT_H        0x39  /**< Activity threshold high */
#define ADXL380_REG_THRESH_ACT_L        0x3A  /**< Activity threshold low  */
#define ADXL380_REG_TIME_ACT_H          0x3B  /**< Activity time high      */
#define ADXL380_REG_TIME_ACT_M          0x3C  /**< Activity time mid       */
#define ADXL380_REG_TIME_ACT_L          0x3D  /**< Activity time low       */
#define ADXL380_REG_THRESH_INACT_H      0x3E  /**< Inactivity threshold high */
#define ADXL380_REG_THRESH_INACT_L      0x3F  /**< Inactivity threshold low  */
#define ADXL380_REG_TIME_INACT_H        0x40  /**< Inactivity time high      */
#define ADXL380_REG_TIME_INACT_M        0x41  /**< Inactivity time mid       */
#define ADXL380_REG_TIME_INACT_L        0x42  /**< Inactivity time low       */

/* Tap detection */
#define ADXL380_REG_TAP_THRESH          0x43  /**< Tap threshold    */
#define ADXL380_REG_TAP_DUR             0x44  /**< Tap duration     */
#define ADXL380_REG_TAP_LATENT          0x45  /**< Tap latent time  */
#define ADXL380_REG_TAP_WINDOW          0x46  /**< Tap window time  */
#define ADXL380_REG_TAP_CFG             0x47  /**< Tap configuration */

/* Orientation / trigger */
#define ADXL380_REG_OR_CFG              0x48  /**< Orientation config  */
#define ADXL380_REG_TRIG_CFG            0x49  /**< Trigger config      */

/* Offset registers */
#define ADXL380_REG_X_SAR_OFFSET        0x4A  /**< X-axis SAR offset */
#define ADXL380_REG_Y_SAR_OFFSET        0x4B  /**< Y-axis SAR offset */
#define ADXL380_REG_Z_SAR_OFFSET        0x4C  /**< Z-axis SAR offset */
#define ADXL380_REG_X_DSM_OFFSET        0x4D  /**< X-axis DSM offset */
#define ADXL380_REG_Y_DSM_OFFSET        0x4E  /**< Y-axis DSM offset */
#define ADXL380_REG_Z_DSM_OFFSET        0x4F  /**< Z-axis DSM offset */

/* Digital filter */
#define ADXL380_REG_FILTER              0x50  /**< Filter configuration */

/* User temperature sensor trim */
#define ADXL380_REG_USER_TEMP_SENS_0    0x55  /**< User temp sensor byte 0 */
#define ADXL380_REG_USER_TEMP_SENS_1    0x56  /**< User temp sensor byte 1 */

/* Pad / pin control */
#define ADXL380_REG_MISO                0x58  /**< MISO pad control       */
#define ADXL380_REG_SOUT0_PAD_CTRL      0x59  /**< SOUT0 pad control      */
#define ADXL380_REG_MCLK_PAD_CTRL       0x5A  /**< MCLK pad control       */
#define ADXL380_REG_BCLK_PAD_CTRL       0x5B  /**< BCLK pad control       */
#define ADXL380_REG_FSYNC_PAD_CTRL      0x5C  /**< FSYNC pad control      */
#define ADXL380_REG_INT0_PAD_CTRL       0x5D  /**< INT0 pad control       */
#define ADXL380_REG_INT1_PAD_CTRL       0x5E  /**< INT1 pad control       */

/** @} */ /* end reg_map */

/* ────────────────────────────────────────────────────────────────────────── */
/*  Reset Value Constants                                                   */
/* ────────────────────────────────────────────────────────────────────────── */

/** @defgroup reset_vals ADXL380 Reset Values
 *  @{ */
#define ADXL380_RESET_DEVID_AD          0xAD  /**< DEVID_AD reset value   */
#define ADXL380_RESET_DEVID_MST         0x1D  /**< DEVID_MST reset value  */
#define ADXL380_RESET_PART_ID           0x17  /**< PART_ID reset value    */
#define ADXL380_RESET_PART_ID_REV       0xC3  /**< PART_ID_REV reset val  */
#define ADXL380_RESET_STATUS0           0x80  /**< STATUS0 reset value    */
#define ADXL380_RESET_STATUS2           0x04  /**< STATUS2 reset value    */
/** @} */

/* ────────────────────────────────────────────────────────────────────────── */
/*  Bit Field Defines                                                       */
/* ────────────────────────────────────────────────────────────────────────── */

/** @defgroup bitfields ADXL380 Bit Field Definitions
 *  @{ */

/* STATUS0 (0x11) */
#define ADXL380_STATUS0_NVM_BUSY            (1 << 7)  /**< NVM busy flag           */
#define ADXL380_STATUS0_FIFO_FULL           (1 << 6)  /**< FIFO full flag          */
#define ADXL380_STATUS0_FIFO_OVR            (1 << 5)  /**< FIFO overrun flag       */
#define ADXL380_STATUS0_FIFO_WM             (1 << 4)  /**< FIFO watermark flag     */
#define ADXL380_STATUS0_FIFO_RDY            (1 << 3)  /**< FIFO ready flag         */
#define ADXL380_STATUS0_PARITY_ERR_STICKY   (1 << 1)  /**< Parity error (sticky)   */

/* STATUS1 (0x12) */
#define ADXL380_STATUS1_NVM_IRQ             (1 << 7)  /**< NVM interrupt flag      */
#define ADXL380_STATUS1_ACT                 (1 << 4)  /**< Activity detected       */
#define ADXL380_STATUS1_INACT               (1 << 3)  /**< Inactivity detected     */
#define ADXL380_STATUS1_OR                  (1 << 2)  /**< Orientation change      */
#define ADXL380_STATUS1_DOUBLE_TAP          (1 << 1)  /**< Double-tap detected     */
#define ADXL380_STATUS1_SINGLE_TAP          (1 << 0)  /**< Single-tap detected     */

/* STATUS2 (0x13) */
#define ADXL380_STATUS2_NVM_ECC_ERR         (1 << 3)  /**< NVM ECC error           */
#define ADXL380_STATUS2_NVM_CRC_ERR         (1 << 2)  /**< NVM CRC error           */
#define ADXL380_STATUS2_UV_FLAG_STICKY      (1 << 0)  /**< Undervoltage (sticky)   */

/* STATUS3 (0x14) */
#define ADXL380_STATUS3_DATA_READY          (1 << 0)  /**< Data ready flag         */

/* OP_MODE register (0x26) */
#define ADXL380_OP_MODE_RANGE_MSK           0xC0      /**< Range mask  Bits[7:6]   */
#define ADXL380_OP_MODE_RANGE_POS           6         /**< Range field position    */
#define ADXL380_OP_MODE_PDM_MODE            (1 << 5)  /**< PDM mode enable         */
#define ADXL380_OP_MODE_AUDIO_MODE          (1 << 4)  /**< Audio mode enable       */
#define ADXL380_OP_MODE_MODE_MSK            0x0F      /**< Mode mask   Bits[3:0]   */

/* DIG_EN register (0x27) */
#define ADXL380_DIG_EN_TEMP_EN              (1 << 7)  /**< Temperature enable      */
#define ADXL380_DIG_EN_Z_EN                 (1 << 6)  /**< Z-axis enable           */
#define ADXL380_DIG_EN_Y_EN                 (1 << 5)  /**< Y-axis enable           */
#define ADXL380_DIG_EN_X_EN                 (1 << 4)  /**< X-axis enable           */
#define ADXL380_DIG_EN_FIFO_EN              (1 << 3)  /**< FIFO enable             */
#define ADXL380_DIG_EN_DOUBLE_SPEED         (1 << 2)  /**< Double-speed mode       */
#define ADXL380_DIG_EN_INT01_EVENT          (1 << 1)  /**< INT0/1 event mode       */
#define ADXL380_DIG_EN_PARITY_EN            (1 << 0)  /**< Parity enable           */

/* REG_RESET (0x2A) */
#define ADXL380_REG_RESET_CODE              0x52      /**< Reset code (ASCII 'R')  */

/* SNSR_AXIS_EN (0x38) */
#define ADXL380_SNSR_AXIS_EN_ST_MODE        (1 << 7)  /**< Self-test mode          */
#define ADXL380_SNSR_AXIS_EN_ST_FORCE       (1 << 6)  /**< Self-test force         */
#define ADXL380_SNSR_AXIS_EN_ST_DIR         (1 << 5)  /**< Self-test direction     */

/* FILTER register (0x50) */
#define ADXL380_FILTER_DCF_BYPASS           (1 << 7)  /**< DC filter bypass        */
#define ADXL380_FILTER_EQ_BYPASS            (1 << 6)  /**< Equaliser bypass        */
#define ADXL380_FILTER_LPF_MODE_MSK         0x30      /**< LPF mode Bits[5:4]      */
#define ADXL380_FILTER_LPF_MODE_POS         4         /**< LPF mode position       */
#define ADXL380_FILTER_HPF_PATH             (1 << 3)  /**< HPF data path select    */
#define ADXL380_FILTER_HPF_CORNER_MSK       0x07      /**< HPF corner Bits[2:0]    */

/* SPI protocol defines */
#define ADXL380_SPI_READ_BIT                0x01      /**< SPI read bit            */
#define ADXL380_SPI_WRITE_BIT               0x00      /**< SPI write bit           */
#define ADXL380_SPI_ADDR_SHIFT              1         /**< Address shift in cmd    */

/** @} */ /* end bitfields */

/* ────────────────────────────────────────────────────────────────────────── */
/*  Enumerations                                                            */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief Operating mode selection (Bits[3:0] of OP_MODE register).
 */
typedef enum {
    ADXL380_MODE_STANDBY    = 0x00,  /**< Standby (default)              */
    ADXL380_MODE_HS         = 0x01,  /**< Heart Sound                    */
    ADXL380_MODE_ULP        = 0x02,  /**< Ultra Low Power                */
    ADXL380_MODE_VLP        = 0x03,  /**< Very Low Power                 */
    ADXL380_MODE_LP         = 0x04,  /**< Low Power                      */
    ADXL380_MODE_LP_ULP     = 0x06,  /**< LP + ULP concurrent            */
    ADXL380_MODE_LP_VLP     = 0x07,  /**< LP + VLP concurrent            */
    ADXL380_MODE_RBW        = 0x08,  /**< Reduced Bandwidth              */
    ADXL380_MODE_RBW_ULP    = 0x0A,  /**< RBW + ULP concurrent           */
    ADXL380_MODE_RBW_VLP    = 0x0B,  /**< RBW + VLP concurrent           */
    ADXL380_MODE_HP         = 0x0C,  /**< High Performance               */
    ADXL380_MODE_HP_ULP     = 0x0E,  /**< HP + ULP concurrent            */
    ADXL380_MODE_HP_VLP     = 0x0F,  /**< HP + VLP concurrent            */
} adxl380_op_mode_t;

/**
 * @brief Measurement range selection (Bits[7:6] of OP_MODE register).
 */
typedef enum {
    ADXL380_RANGE_4G        = 0x00,  /**< ±4 g  */
    ADXL380_RANGE_8G        = 0x01,  /**< ±8 g  */
    ADXL380_RANGE_16G       = 0x02,  /**< ±16 g */
} adxl380_range_t;

/**
 * @brief FIFO operating mode selection.
 */
typedef enum {
    ADXL380_FIFO_DISABLED   = 0x00,  /**< FIFO disabled     */
    ADXL380_FIFO_OLDEST     = 0x01,  /**< Oldest-saved mode */
    ADXL380_FIFO_STREAM     = 0x02,  /**< Stream mode       */
    ADXL380_FIFO_TRIGGERED  = 0x03,  /**< Triggered mode    */
} adxl380_fifo_mode_t;

/**
 * @brief Driver return / status codes.
 */
typedef enum {
    ADXL380_OK              =  0,    /**< Success           */
    ADXL380_ERR_SPI         = -1,    /**< SPI transfer fail */
    ADXL380_ERR_DEVICE      = -2,    /**< Device ID mismatch */
    ADXL380_ERR_PARAM       = -3,    /**< Invalid parameter */
    ADXL380_ERR_TIMEOUT     = -4,    /**< Timeout           */
} adxl380_status_t;

/* ────────────────────────────────────────────────────────────────────────── */
/*  Sensitivity Constants                                                   */
/* ────────────────────────────────────────────────────────────────────────── */

/** @defgroup sensitivity ADXL380 Sensitivity & Temperature Constants
 *  @{ */
#define ADXL380_SENSITIVITY_4G          7500   /**< LSB/g at ±4 g  */
#define ADXL380_SENSITIVITY_8G          3750   /**< LSB/g at ±8 g  */
#define ADXL380_SENSITIVITY_16G         1875   /**< LSB/g at ±16 g */

#define ADXL380_TEMP_OFFSET_AT_25C      550    /**< LSB at 25 °C              */
#define ADXL380_TEMP_SENSITIVITY        10.2f  /**< LSB/°C (default)          */
#define ADXL380_TEMP_SENSITIVITY_HG     16.5f  /**< LSB/°C (high-gain mode)   */
/** @} */

/* ────────────────────────────────────────────────────────────────────────── */
/*  Self-Test Constants                                                     */
/* ────────────────────────────────────────────────────────────────────────── */

/** @defgroup selftest ADXL380 Self-Test Limits
 *  @{ */
#define ADXL380_SELF_TEST_XY_MIN_MG     2500   /**< Min XY delta  (2.5 g in mg) */
#define ADXL380_SELF_TEST_XY_MAX_MG     5500   /**< Max XY delta  (5.5 g in mg) */
#define ADXL380_SELF_TEST_Z_MIN_MG      2000   /**< Min Z  delta  (2.0 g in mg) */
#define ADXL380_SELF_TEST_Z_MAX_MG      3800   /**< Max Z  delta  (3.8 g in mg) */
/** @} */

/* ────────────────────────────────────────────────────────────────────────── */
/*  Timing Constants                                                        */
/* ────────────────────────────────────────────────────────────────────────── */

/** @defgroup timing ADXL380 Timing Constants
 *  @{ */
#define ADXL380_RESET_LATENCY_US        500    /**< Reset latency (0.5 ms)    */
#define ADXL380_NVM_REFRESH_US          2200   /**< NVM refresh   (2.2 ms)    */
#define ADXL380_STARTUP_DELAY_US        (ADXL380_RESET_LATENCY_US + ADXL380_NVM_REFRESH_US)
/** @} */

/* ────────────────────────────────────────────────────────────────────────── */
/*  Type Definitions                                                        */
/* ────────────────────────────────────────────────────────────────────────── */

/**
 * @brief SPI transfer callback.
 *
 * @param tx_buf    Transmit buffer (may be NULL for read-only).
 * @param rx_buf    Receive buffer  (may be NULL for write-only).
 * @param len       Number of bytes to transfer.
 * @param user_data Opaque pointer forwarded from the device handle.
 * @return 0 on success, non-zero on failure.
 */
typedef int (*adxl380_spi_xfer_t)(uint8_t *tx_buf, uint8_t *rx_buf,
                                  uint16_t len, void *user_data);

/**
 * @brief Microsecond delay callback.
 *
 * @param us        Delay duration in microseconds.
 * @param user_data Opaque pointer forwarded from the device handle.
 */
typedef void (*adxl380_delay_us_t)(uint32_t us, void *user_data);

/**
 * @brief ADXL380 device handle.
 *
 * Populate @c spi_xfer, @c delay_us, and optionally @c user_data before
 * calling @ref adxl380_init.
 */
typedef struct {
    adxl380_spi_xfer_t spi_xfer;   /**< SPI transfer function          */
    adxl380_delay_us_t delay_us;   /**< Microsecond delay function     */
    void              *user_data;  /**< Opaque user context pointer    */
    adxl380_range_t    range;      /**< Currently configured range     */
    adxl380_op_mode_t  op_mode;    /**< Currently configured mode      */
} adxl380_dev_t;

/**
 * @brief Raw 16-bit acceleration sample for all three axes.
 */
typedef struct {
    int16_t x;  /**< X-axis acceleration (raw) */
    int16_t y;  /**< Y-axis acceleration (raw) */
    int16_t z;  /**< Z-axis acceleration (raw) */
} adxl380_accel_data_t;

/* ────────────────────────────────────────────────────────────────────────── */
/*  Function Declarations                                                   */
/* ────────────────────────────────────────────────────────────────────────── */

/** @defgroup core Core Register Access
 *  @{ */

/**
 * @brief Read a single register.
 *
 * @param dev  Device handle.
 * @param reg  Register address.
 * @param val  Pointer to store the read value.
 * @return Status code.
 */
adxl380_status_t adxl380_read_reg(const adxl380_dev_t *dev, uint8_t reg,
                                  uint8_t *val);

/**
 * @brief Write a single register.
 *
 * @param dev  Device handle.
 * @param reg  Register address.
 * @param val  Value to write.
 * @return Status code.
 */
adxl380_status_t adxl380_write_reg(const adxl380_dev_t *dev, uint8_t reg,
                                   uint8_t val);

/**
 * @brief Burst-read multiple consecutive registers.
 *
 * @param dev  Device handle.
 * @param reg  Starting register address.
 * @param buf  Destination buffer.
 * @param len  Number of bytes to read.
 * @return Status code.
 */
adxl380_status_t adxl380_read_regs(const adxl380_dev_t *dev, uint8_t reg,
                                   uint8_t *buf, uint16_t len);

/**
 * @brief Burst-write multiple consecutive registers.
 *
 * @param dev  Device handle.
 * @param reg  Starting register address.
 * @param buf  Source buffer.
 * @param len  Number of bytes to write.
 * @return Status code.
 */
adxl380_status_t adxl380_write_regs(const adxl380_dev_t *dev, uint8_t reg,
                                    const uint8_t *buf, uint16_t len);

/** @} */ /* end core */

/** @defgroup init Initialisation & Reset
 *  @{ */

/**
 * @brief Initialise the ADXL380 device.
 *
 * Verifies device identity and loads default configuration.
 *
 * @param dev  Device handle (spi_xfer and delay_us must be set).
 * @return Status code.
 */
adxl380_status_t adxl380_init(adxl380_dev_t *dev);

/**
 * @brief Issue a software reset.
 *
 * Writes @ref ADXL380_REG_RESET_CODE to the REG_RESET register.
 *
 * @param dev  Device handle.
 * @return Status code.
 */
adxl380_status_t adxl380_soft_reset(const adxl380_dev_t *dev);

/** @} */ /* end init */

/** @defgroup config Configuration
 *  @{ */

/**
 * @brief Set the operating mode.
 *
 * @param dev   Device handle.
 * @param mode  Desired operating mode.
 * @return Status code.
 */
adxl380_status_t adxl380_set_op_mode(adxl380_dev_t *dev,
                                     adxl380_op_mode_t mode);

/**
 * @brief Read the current operating mode.
 *
 * @param dev   Device handle.
 * @param mode  Pointer to store the current mode.
 * @return Status code.
 */
adxl380_status_t adxl380_get_op_mode(const adxl380_dev_t *dev,
                                     adxl380_op_mode_t *mode);

/**
 * @brief Set the measurement range.
 *
 * @param dev    Device handle.
 * @param range  Desired range.
 * @return Status code.
 */
adxl380_status_t adxl380_set_range(adxl380_dev_t *dev,
                                   adxl380_range_t range);

/**
 * @brief Read the current measurement range.
 *
 * @param dev    Device handle.
 * @param range  Pointer to store the current range.
 * @return Status code.
 */
adxl380_status_t adxl380_get_range(const adxl380_dev_t *dev,
                                   adxl380_range_t *range);

/** @} */ /* end config */

/** @defgroup channels Channel Enable
 *  @{ */

/**
 * @brief Enable or disable measurement channels.
 *
 * @param dev   Device handle.
 * @param x     Enable X-axis.
 * @param y     Enable Y-axis.
 * @param z     Enable Z-axis.
 * @param temp  Enable temperature channel.
 * @return Status code.
 */
adxl380_status_t adxl380_enable_channels(const adxl380_dev_t *dev,
                                         bool x, bool y, bool z, bool temp);

/** @} */ /* end channels */

/** @defgroup data Data Acquisition
 *  @{ */

/**
 * @brief Read raw acceleration data for all three axes.
 *
 * @param dev   Device handle.
 * @param data  Pointer to store the acceleration data.
 * @return Status code.
 */
adxl380_status_t adxl380_get_accel_data(const adxl380_dev_t *dev,
                                        adxl380_accel_data_t *data);

/**
 * @brief Read the raw temperature value.
 *
 * @param dev       Device handle.
 * @param raw_temp  Pointer to store the raw temperature value.
 * @return Status code.
 */
adxl380_status_t adxl380_get_temp_raw(const adxl380_dev_t *dev,
                                      int16_t *raw_temp);

/**
 * @brief Convert a raw temperature reading to degrees Celsius.
 *
 * @param raw_temp  Raw temperature value from @ref adxl380_get_temp_raw.
 * @return Temperature in °C.
 */
float adxl380_convert_temp_c(int16_t raw_temp);

/**
 * @brief Check whether new acceleration data is ready.
 *
 * @param dev    Device handle.
 * @param ready  Pointer to store the ready flag.
 * @return Status code.
 */
adxl380_status_t adxl380_is_data_ready(const adxl380_dev_t *dev, bool *ready);

/** @} */ /* end data */

/** @defgroup helpers Sensitivity Helper
 *  @{ */

/**
 * @brief Get the sensitivity (LSB/g) for the given range.
 *
 * @param range  Measurement range.
 * @return Sensitivity in LSB/g.
 */
uint16_t adxl380_get_sensitivity(adxl380_range_t range);

/** @} */ /* end helpers */

/** @defgroup selftest_api Self-Test
 *  @{ */

/**
 * @brief Execute the built-in self-test and report pass/fail.
 *
 * @param dev     Device handle.
 * @param passed  Pointer to store the self-test result.
 * @return Status code.
 */
adxl380_status_t adxl380_self_test(adxl380_dev_t *dev, bool *passed);

/** @} */ /* end selftest_api */

/** @defgroup fifo FIFO
 *  @{ */

/**
 * @brief Configure the FIFO operating mode.
 *
 * @param dev   Device handle.
 * @param mode  Desired FIFO mode.
 * @return Status code.
 */
adxl380_status_t adxl380_set_fifo_mode(const adxl380_dev_t *dev,
                                       adxl380_fifo_mode_t mode);

/**
 * @brief Get the number of valid FIFO entries.
 *
 * @param dev      Device handle.
 * @param entries  Pointer to store the entry count.
 * @return Status code.
 */
adxl380_status_t adxl380_get_fifo_entries(const adxl380_dev_t *dev,
                                          uint16_t *entries);

/**
 * @brief Read data from the FIFO.
 *
 * @param dev  Device handle.
 * @param buf  Destination buffer.
 * @param len  Number of bytes to read.
 * @return Status code.
 */
adxl380_status_t adxl380_read_fifo(const adxl380_dev_t *dev,
                                   uint8_t *buf, uint16_t len);

/** @} */ /* end fifo */

/** @defgroup interrupts Interrupt Configuration
 *  @{ */

/**
 * @brief Configure interrupt pin mapping.
 *
 * @param dev      Device handle.
 * @param int_num  Interrupt pin number (0 or 1).
 * @param map0     MAP0 register value.
 * @param map1     MAP1 register value.
 * @return Status code.
 */
adxl380_status_t adxl380_set_int_map(const adxl380_dev_t *dev,
                                     uint8_t int_num, uint8_t map0,
                                     uint8_t map1);

/** @} */ /* end interrupts */

/** @defgroup act_inact Activity / Inactivity Detection
 *  @{ */

/**
 * @brief Set the activity threshold.
 *
 * @param dev        Device handle.
 * @param threshold  11-bit threshold value.
 * @return Status code.
 */
adxl380_status_t adxl380_set_act_threshold(const adxl380_dev_t *dev,
                                           uint16_t threshold);

/**
 * @brief Set the inactivity threshold.
 *
 * @param dev        Device handle.
 * @param threshold  11-bit threshold value.
 * @return Status code.
 */
adxl380_status_t adxl380_set_inact_threshold(const adxl380_dev_t *dev,
                                             uint16_t threshold);

/**
 * @brief Set the activity time.
 *
 * @param dev   Device handle.
 * @param time  24-bit activity time value.
 * @return Status code.
 */
adxl380_status_t adxl380_set_act_time(const adxl380_dev_t *dev, uint32_t time);

/**
 * @brief Set the inactivity time.
 *
 * @param dev   Device handle.
 * @param time  24-bit inactivity time value.
 * @return Status code.
 */
adxl380_status_t adxl380_set_inact_time(const adxl380_dev_t *dev,
                                        uint32_t time);

/**
 * @brief Set the activity/inactivity control register.
 *
 * @param dev  Device handle.
 * @param ctl  Control register value.
 * @return Status code.
 */
adxl380_status_t adxl380_set_act_inact_ctl(const adxl380_dev_t *dev,
                                           uint8_t ctl);

/** @} */ /* end act_inact */

/** @defgroup tap Tap Detection
 *  @{ */

/**
 * @brief Configure tap detection parameters.
 *
 * @param dev     Device handle.
 * @param thresh  Tap threshold.
 * @param dur     Tap duration.
 * @param latent  Tap latency.
 * @param window  Tap window.
 * @param cfg     Tap configuration register value.
 * @return Status code.
 */
adxl380_status_t adxl380_set_tap_config(const adxl380_dev_t *dev,
                                        uint8_t thresh, uint8_t dur,
                                        uint8_t latent, uint8_t window,
                                        uint8_t cfg);

/** @} */ /* end tap */

/** @defgroup status Status
 *  @{ */

/**
 * @brief Read all four status registers.
 *
 * @param dev      Device handle.
 * @param status0  Pointer to store STATUS0 value (may be NULL).
 * @param status1  Pointer to store STATUS1 value (may be NULL).
 * @param status2  Pointer to store STATUS2 value (may be NULL).
 * @param status3  Pointer to store STATUS3 value (may be NULL).
 * @return Status code.
 */
adxl380_status_t adxl380_get_status(const adxl380_dev_t *dev,
                                    uint8_t *status0, uint8_t *status1,
                                    uint8_t *status2, uint8_t *status3);

/** @} */ /* end status */

/** @defgroup devid Device Identification
 *  @{ */

/**
 * @brief Read the device identification registers.
 *
 * @param dev       Device handle.
 * @param devid_ad  Pointer to store DEVID_AD  (may be NULL).
 * @param devid_mst Pointer to store DEVID_MST (may be NULL).
 * @param part_id   Pointer to store PART_ID   (may be NULL).
 * @return Status code.
 */
adxl380_status_t adxl380_get_device_id(const adxl380_dev_t *dev,
                                       uint8_t *devid_ad, uint8_t *devid_mst,
                                       uint8_t *part_id);

/** @} */ /* end devid */

#ifdef __cplusplus
}
#endif

#endif /* ADXL380_DRIVER_H */
