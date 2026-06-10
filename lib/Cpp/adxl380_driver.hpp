/**
 * @file adxl380_driver.hpp
 * @brief ADXL380 3-Axis Digital Accelerometer C++ RAII Wrapper
 *
 * Provides an exception-safe, RAII C++ interface around the ADXL380 C driver.
 * Uses std::function for SPI/delay callbacks and static trampolines to bridge
 * with the C function-pointer API.
 */

#ifndef ADXL380_DRIVER_HPP
#define ADXL380_DRIVER_HPP

#include <cstdint>
#include <functional>
#include <stdexcept>

#include "../C/adxl380_driver.h"

namespace adxl380 {

/**
 * @brief Exception thrown when an ADXL380 C driver call fails.
 */
class DriverError : public std::runtime_error {
public:
    /**
     * @brief Construct a DriverError.
     * @param status  The failing status code from the C driver.
     * @param msg     Human-readable context string.
     */
    explicit DriverError(adxl380_status_t status,
                         const char* msg = "ADXL380 driver error");

    /** @brief Return the underlying C driver status code. */
    adxl380_status_t status() const noexcept;

private:
    adxl380_status_t status_;
};

/**
 * @brief Raw 16-bit acceleration sample for all three axes.
 */
struct AccelData {
    int16_t x; /**< X-axis acceleration (raw) */
    int16_t y; /**< Y-axis acceleration (raw) */
    int16_t z; /**< Z-axis acceleration (raw) */
};

/**
 * @brief RAII wrapper around the ADXL380 C driver.
 *
 * Owns an @c adxl380_dev_t handle and forwards every call through the C API,
 * translating non-OK status codes into @ref DriverError exceptions.
 */
class Accelerometer {
public:
    /** @brief SPI transfer callback type. */
    using SpiTransfer = std::function<int(uint8_t*, uint8_t*, uint16_t, void*)>;

    /** @brief Microsecond delay callback type. */
    using DelayUs = std::function<void(uint32_t, void*)>;

    /**
     * @brief Construct an Accelerometer.
     * @param spi_xfer   SPI full-duplex transfer function.
     * @param delay_us   Microsecond delay function.
     * @param user_data  Opaque pointer forwarded to callbacks.
     */
    Accelerometer(SpiTransfer spi_xfer, DelayUs delay_us,
                  void* user_data = nullptr);

    /**
     * @brief Destructor — sets the device to standby (best-effort).
     */
    ~Accelerometer();

    /** @brief Deleted copy constructor. */
    Accelerometer(const Accelerometer&) = delete;

    /** @brief Deleted copy-assignment operator. */
    Accelerometer& operator=(const Accelerometer&) = delete;

    /** @brief Move constructor. */
    Accelerometer(Accelerometer&& other) noexcept;

    /** @brief Move-assignment operator. */
    Accelerometer& operator=(Accelerometer&& other) noexcept;

    /* ── Initialisation & Reset ────────────────────────────────────────── */

    /** @brief Initialise the device (calls adxl380_init). */
    void init();

    /** @brief Issue a software reset. */
    void softReset();

    /* ── Configuration ─────────────────────────────────────────────────── */

    /** @brief Set the operating mode. */
    void setOpMode(adxl380_op_mode_t mode);

    /** @brief Get the current operating mode. */
    adxl380_op_mode_t getOpMode() const;

    /** @brief Set the measurement range. */
    void setRange(adxl380_range_t range);

    /** @brief Get the current measurement range. */
    adxl380_range_t getRange() const;

    /* ── Channel Enable ────────────────────────────────────────────────── */

    /**
     * @brief Enable or disable measurement channels.
     * @param x     Enable X-axis.
     * @param y     Enable Y-axis.
     * @param z     Enable Z-axis.
     * @param temp  Enable temperature channel.
     */
    void enableChannels(bool x, bool y, bool z, bool temp);

    /* ── Data Acquisition ──────────────────────────────────────────────── */

    /** @brief Read raw acceleration data for all three axes. */
    AccelData getAccelData() const;

    /** @brief Read the raw temperature value. */
    int16_t getTempRaw() const;

    /** @brief Read the temperature in degrees Celsius. */
    float getTempCelsius() const;

    /** @brief Check whether new data is ready. */
    bool isDataReady() const;

    /* ── Sensitivity ───────────────────────────────────────────────────── */

    /** @brief Get the sensitivity (LSB/g) for the current range. */
    uint16_t getSensitivity() const;

    /* ── Self-Test ─────────────────────────────────────────────────────── */

    /** @brief Run the built-in self-test. @return true if passed. */
    bool selfTest();

    /* ── FIFO ──────────────────────────────────────────────────────────── */

    /** @brief Configure the FIFO operating mode. */
    void setFifoMode(adxl380_fifo_mode_t mode);

    /** @brief Get the number of valid FIFO entries. */
    uint16_t getFifoEntries() const;

    /**
     * @brief Read data from the FIFO.
     * @param buf  Destination buffer.
     * @param len  Number of bytes to read.
     */
    void readFifo(uint8_t* buf, uint16_t len) const;

    /* ── Interrupts ────────────────────────────────────────────────────── */

    /**
     * @brief Configure interrupt pin mapping.
     * @param int_num  Interrupt pin number (0 or 1).
     * @param map0     MAP0 register value.
     * @param map1     MAP1 register value.
     */
    void setIntMap(uint8_t int_num, uint8_t map0, uint8_t map1);

    /* ── Activity / Inactivity ─────────────────────────────────────────── */

    /** @brief Set the activity threshold (11-bit). */
    void setActThreshold(uint16_t threshold);

    /** @brief Set the inactivity threshold (11-bit). */
    void setInactThreshold(uint16_t threshold);

    /** @brief Set the activity time (24-bit). */
    void setActTime(uint32_t time);

    /** @brief Set the inactivity time (24-bit). */
    void setInactTime(uint32_t time);

    /** @brief Set the activity/inactivity control register. */
    void setActInactCtl(uint8_t ctl);

    /* ── Tap Detection ─────────────────────────────────────────────────── */

    /**
     * @brief Configure tap detection parameters.
     * @param thresh  Tap threshold.
     * @param dur     Tap duration.
     * @param latent  Tap latency.
     * @param window  Tap window.
     * @param cfg     Tap configuration register value.
     */
    void setTapConfig(uint8_t thresh, uint8_t dur, uint8_t latent,
                      uint8_t window, uint8_t cfg);

    /* ── Status ────────────────────────────────────────────────────────── */

    /**
     * @brief Read all four status registers.
     * @param s0  Pointer to store STATUS0 (may be nullptr).
     * @param s1  Pointer to store STATUS1 (may be nullptr).
     * @param s2  Pointer to store STATUS2 (may be nullptr).
     * @param s3  Pointer to store STATUS3 (may be nullptr).
     */
    void getStatus(uint8_t* s0, uint8_t* s1, uint8_t* s2,
                   uint8_t* s3) const;

    /* ── Device ID ─────────────────────────────────────────────────────── */

    /**
     * @brief Read device identification registers.
     * @param devid_ad   Pointer to store DEVID_AD  (may be nullptr).
     * @param devid_mst  Pointer to store DEVID_MST (may be nullptr).
     * @param part_id    Pointer to store PART_ID   (may be nullptr).
     */
    void getDeviceId(uint8_t* devid_ad, uint8_t* devid_mst,
                     uint8_t* part_id) const;

    /* ── Raw Register Access ───────────────────────────────────────────── */

    /** @brief Read a single register. */
    uint8_t readReg(uint8_t reg) const;

    /** @brief Write a single register. */
    void writeReg(uint8_t reg, uint8_t val);

private:
    /**
     * @brief Throw DriverError if @p status is not ADXL380_OK.
     * @param status   Return value from a C driver call.
     * @param context  Human-readable context for the error message.
     */
    void checkStatus(adxl380_status_t status,
                     const char* context = "") const;

    /** @brief Static trampoline forwarding SPI calls to std::function. */
    static int spiTrampoline(uint8_t* tx, uint8_t* rx, uint16_t len,
                             void* ctx);

    /** @brief Static trampoline forwarding delay calls to std::function. */
    static void delayTrampoline(uint32_t us, void* ctx);

    adxl380_dev_t dev_;            /**< C driver device handle          */
    SpiTransfer   spi_xfer_;       /**< Stored SPI transfer callback    */
    DelayUs       delay_us_;       /**< Stored delay callback           */
    void*         original_user_data_; /**< User-supplied opaque pointer */
    bool          initialized_;    /**< Whether init() has succeeded    */
};

} /* namespace adxl380 */

#endif /* ADXL380_DRIVER_HPP */
