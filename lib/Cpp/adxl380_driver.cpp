/**
 * @file adxl380_driver.cpp
 * @brief ADXL380 C++ RAII Wrapper — Implementation
 *
 * Implements the adxl380::Accelerometer class declared in adxl380_driver.hpp.
 * Static trampoline functions bridge std::function callbacks to the C driver's
 * plain function-pointer API.
 */

#include "adxl380_driver.hpp"

#include <cstring>

namespace adxl380 {

/* ────────────────────────────────────────────────────────────────────────── */
/*  DriverError                                                              */
/* ────────────────────────────────────────────────────────────────────────── */

DriverError::DriverError(adxl380_status_t status, const char* msg)
    : std::runtime_error(msg), status_(status) {}

adxl380_status_t DriverError::status() const noexcept {
    return status_;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Static trampolines                                                       */
/* ────────────────────────────────────────────────────────────────────────── */

int Accelerometer::spiTrampoline(uint8_t* tx, uint8_t* rx, uint16_t len,
                                 void* ctx) {
    auto* self = static_cast<Accelerometer*>(ctx);
    return self->spi_xfer_(tx, rx, len, self->original_user_data_);
}

void Accelerometer::delayTrampoline(uint32_t us, void* ctx) {
    auto* self = static_cast<Accelerometer*>(ctx);
    self->delay_us_(us, self->original_user_data_);
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  checkStatus helper                                                       */
/* ────────────────────────────────────────────────────────────────────────── */

void Accelerometer::checkStatus(adxl380_status_t status,
                                const char* context) const {
    if (status != ADXL380_OK) {
        throw DriverError(status, context);
    }
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Constructor / Destructor / Move                                          */
/* ────────────────────────────────────────────────────────────────────────── */

Accelerometer::Accelerometer(SpiTransfer spi_xfer, DelayUs delay_us,
                             void* user_data)
    : spi_xfer_(std::move(spi_xfer)),
      delay_us_(std::move(delay_us)),
      original_user_data_(user_data),
      initialized_(false) {
    std::memset(&dev_, 0, sizeof(dev_));
    dev_.spi_xfer  = &spiTrampoline;
    dev_.delay_us  = &delayTrampoline;
    dev_.user_data = this;
}

Accelerometer::~Accelerometer() {
    if (initialized_) {
        // Best-effort standby — ignore errors in the destructor.
        adxl380_set_op_mode(&dev_, ADXL380_MODE_STANDBY);
    }
}

Accelerometer::Accelerometer(Accelerometer&& other) noexcept
    : dev_(other.dev_),
      spi_xfer_(std::move(other.spi_xfer_)),
      delay_us_(std::move(other.delay_us_)),
      original_user_data_(other.original_user_data_),
      initialized_(other.initialized_) {
    // Point the C handle at *this* instance.
    dev_.user_data = this;

    // Invalidate the source so its destructor is a no-op.
    other.initialized_ = false;
    std::memset(&other.dev_, 0, sizeof(other.dev_));
    other.original_user_data_ = nullptr;
}

Accelerometer& Accelerometer::operator=(Accelerometer&& other) noexcept {
    if (this != &other) {
        // Clean up current state.
        if (initialized_) {
            adxl380_set_op_mode(&dev_, ADXL380_MODE_STANDBY);
        }

        dev_                = other.dev_;
        spi_xfer_           = std::move(other.spi_xfer_);
        delay_us_           = std::move(other.delay_us_);
        original_user_data_ = other.original_user_data_;
        initialized_        = other.initialized_;

        dev_.user_data = this;

        other.initialized_ = false;
        std::memset(&other.dev_, 0, sizeof(other.dev_));
        other.original_user_data_ = nullptr;
    }
    return *this;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Initialisation & Reset                                                   */
/* ────────────────────────────────────────────────────────────────────────── */

void Accelerometer::init() {
    checkStatus(adxl380_init(&dev_), "init failed");
    initialized_ = true;
}

void Accelerometer::softReset() {
    checkStatus(adxl380_soft_reset(&dev_), "soft reset failed");
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Configuration                                                            */
/* ────────────────────────────────────────────────────────────────────────── */

void Accelerometer::setOpMode(adxl380_op_mode_t mode) {
    checkStatus(adxl380_set_op_mode(&dev_, mode), "set op mode failed");
}

adxl380_op_mode_t Accelerometer::getOpMode() const {
    adxl380_op_mode_t mode;
    checkStatus(adxl380_get_op_mode(&dev_, &mode), "get op mode failed");
    return mode;
}

void Accelerometer::setRange(adxl380_range_t range) {
    checkStatus(adxl380_set_range(&dev_, range), "set range failed");
}

adxl380_range_t Accelerometer::getRange() const {
    adxl380_range_t range;
    checkStatus(adxl380_get_range(&dev_, &range), "get range failed");
    return range;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Channel Enable                                                           */
/* ────────────────────────────────────────────────────────────────────────── */

void Accelerometer::enableChannels(bool x, bool y, bool z, bool temp) {
    checkStatus(adxl380_enable_channels(&dev_, x, y, z, temp),
                "enable channels failed");
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Data Acquisition                                                         */
/* ────────────────────────────────────────────────────────────────────────── */

AccelData Accelerometer::getAccelData() const {
    adxl380_accel_data_t raw;
    checkStatus(adxl380_get_accel_data(&dev_, &raw), "get accel data failed");
    return {raw.x, raw.y, raw.z};
}

int16_t Accelerometer::getTempRaw() const {
    int16_t raw;
    checkStatus(adxl380_get_temp_raw(&dev_, &raw), "get temp raw failed");
    return raw;
}

float Accelerometer::getTempCelsius() const {
    int16_t raw = getTempRaw();
    return adxl380_convert_temp_c(raw);
}

bool Accelerometer::isDataReady() const {
    bool ready = false;
    checkStatus(adxl380_is_data_ready(&dev_, &ready), "is data ready failed");
    return ready;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Sensitivity                                                              */
/* ────────────────────────────────────────────────────────────────────────── */

uint16_t Accelerometer::getSensitivity() const {
    return adxl380_get_sensitivity(dev_.range);
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Self-Test                                                                */
/* ────────────────────────────────────────────────────────────────────────── */

bool Accelerometer::selfTest() {
    bool passed = false;
    checkStatus(adxl380_self_test(&dev_, &passed), "self test failed");
    return passed;
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  FIFO                                                                     */
/* ────────────────────────────────────────────────────────────────────────── */

void Accelerometer::setFifoMode(adxl380_fifo_mode_t mode) {
    checkStatus(adxl380_set_fifo_mode(&dev_, mode), "set fifo mode failed");
}

uint16_t Accelerometer::getFifoEntries() const {
    uint16_t entries = 0;
    checkStatus(adxl380_get_fifo_entries(&dev_, &entries),
                "get fifo entries failed");
    return entries;
}

void Accelerometer::readFifo(uint8_t* buf, uint16_t len) const {
    checkStatus(adxl380_read_fifo(&dev_, buf, len), "read fifo failed");
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Interrupts                                                               */
/* ────────────────────────────────────────────────────────────────────────── */

void Accelerometer::setIntMap(uint8_t int_num, uint8_t map0, uint8_t map1) {
    checkStatus(adxl380_set_int_map(&dev_, int_num, map0, map1),
                "set int map failed");
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Activity / Inactivity                                                    */
/* ────────────────────────────────────────────────────────────────────────── */

void Accelerometer::setActThreshold(uint16_t threshold) {
    checkStatus(adxl380_set_act_threshold(&dev_, threshold),
                "set act threshold failed");
}

void Accelerometer::setInactThreshold(uint16_t threshold) {
    checkStatus(adxl380_set_inact_threshold(&dev_, threshold),
                "set inact threshold failed");
}

void Accelerometer::setActTime(uint32_t time) {
    checkStatus(adxl380_set_act_time(&dev_, time), "set act time failed");
}

void Accelerometer::setInactTime(uint32_t time) {
    checkStatus(adxl380_set_inact_time(&dev_, time), "set inact time failed");
}

void Accelerometer::setActInactCtl(uint8_t ctl) {
    checkStatus(adxl380_set_act_inact_ctl(&dev_, ctl),
                "set act inact ctl failed");
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Tap Detection                                                            */
/* ────────────────────────────────────────────────────────────────────────── */

void Accelerometer::setTapConfig(uint8_t thresh, uint8_t dur, uint8_t latent,
                                 uint8_t window, uint8_t cfg) {
    checkStatus(adxl380_set_tap_config(&dev_, thresh, dur, latent, window, cfg),
                "set tap config failed");
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Status                                                                   */
/* ────────────────────────────────────────────────────────────────────────── */

void Accelerometer::getStatus(uint8_t* s0, uint8_t* s1, uint8_t* s2,
                               uint8_t* s3) const {
    checkStatus(adxl380_get_status(&dev_, s0, s1, s2, s3),
                "get status failed");
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Device ID                                                                */
/* ────────────────────────────────────────────────────────────────────────── */

void Accelerometer::getDeviceId(uint8_t* devid_ad, uint8_t* devid_mst,
                                uint8_t* part_id) const {
    checkStatus(adxl380_get_device_id(&dev_, devid_ad, devid_mst, part_id),
                "get device id failed");
}

/* ────────────────────────────────────────────────────────────────────────── */
/*  Raw Register Access                                                      */
/* ────────────────────────────────────────────────────────────────────────── */

uint8_t Accelerometer::readReg(uint8_t reg) const {
    uint8_t val = 0;
    checkStatus(adxl380_read_reg(&dev_, reg, &val), "read reg failed");
    return val;
}

void Accelerometer::writeReg(uint8_t reg, uint8_t val) {
    checkStatus(adxl380_write_reg(&dev_, reg, val), "write reg failed");
}

} /* namespace adxl380 */
