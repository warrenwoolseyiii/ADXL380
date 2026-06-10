"""ADXL380 register map and constants."""

from enum import IntEnum

# =============================================================================
# Register Map
# =============================================================================

REG_DEVID_AD        = 0x00
REG_DEVID_MST       = 0x01
REG_PART_ID         = 0x02
REG_PART_ID_REV_ID  = 0x03
REG_SN0             = 0x04
REG_SN1             = 0x05
REG_SN2             = 0x06
REG_SN3             = 0x07
REG_SN4             = 0x08
REG_SN5             = 0x09
REG_SN6             = 0x0A
REG_DEV_DELTA_Q_X   = 0x0B
REG_DEV_DELTA_Q_Y   = 0x0C
REG_DEV_DELTA_Q_Z   = 0x0D
REG_DEV_DELTA_F0_X  = 0x0E
REG_DEV_DELTA_F0_Y  = 0x0F
REG_DEV_DELTA_F0_Z  = 0x10
REG_STATUS0         = 0x11
REG_STATUS1         = 0x12
REG_STATUS2         = 0x13
REG_STATUS3         = 0x14
REG_XDATA_H         = 0x15
REG_XDATA_L         = 0x16
REG_YDATA_H         = 0x17
REG_YDATA_L         = 0x18
REG_ZDATA_H         = 0x19
REG_ZDATA_L         = 0x1A
REG_TDATA_H         = 0x1B
REG_TDATA_L         = 0x1C
REG_FIFO_DATA       = 0x1D
REG_FIFO_STATUS0    = 0x1E
REG_FIFO_STATUS1    = 0x1F
REG_MISC0           = 0x20
REG_MISC1           = 0x21
REG_SENS_DSM        = 0x24
REG_CLK_CTRL        = 0x25
REG_OP_MODE         = 0x26
REG_DIG_EN          = 0x27
REG_SAR_I2C         = 0x28
REG_NVM_CTL         = 0x29
REG_REG_RESET       = 0x2A
REG_INT0_MAP0       = 0x2B
REG_INT0_MAP1       = 0x2C
REG_INT1_MAP0       = 0x2D
REG_INT1_MAP1       = 0x2E
REG_FIFO_CFG0       = 0x30
REG_FIFO_CFG1       = 0x31
REG_SPT_CFG0        = 0x32
REG_SPT_CFG1        = 0x33
REG_SPT_CFG2        = 0x34
REG_SYNC_CFG        = 0x35
REG_PDM_CFG         = 0x36
REG_ACT_INACT_CTL   = 0x37
REG_SNSR_AXIS_EN    = 0x38
REG_THRESH_ACT_H    = 0x39
REG_THRESH_ACT_L    = 0x3A
REG_TIME_ACT_H      = 0x3B
REG_TIME_ACT_M      = 0x3C
REG_TIME_ACT_L      = 0x3D
REG_THRESH_INACT_H  = 0x3E
REG_THRESH_INACT_L  = 0x3F
REG_TIME_INACT_H    = 0x40
REG_TIME_INACT_M    = 0x41
REG_TIME_INACT_L    = 0x42
REG_TAP_THRESH      = 0x43
REG_TAP_DUR         = 0x44
REG_TAP_LATENT      = 0x45
REG_TAP_WINDOW      = 0x46
REG_TAP_CFG         = 0x47
REG_OR_CFG          = 0x48
REG_TRIG_CFG        = 0x49
REG_X_SAR_OFFSET    = 0x4A
REG_Y_SAR_OFFSET    = 0x4B
REG_Z_SAR_OFFSET    = 0x4C
REG_X_DSM_OFFSET    = 0x4D
REG_Y_DSM_OFFSET    = 0x4E
REG_Z_DSM_OFFSET    = 0x4F
REG_FILTER          = 0x50
REG_USER_TEMP_SENS_0 = 0x55
REG_USER_TEMP_SENS_1 = 0x56
REG_MISO            = 0x58
REG_SOUT0_PAD_CTRL  = 0x59
REG_MCLK_PAD_CTRL   = 0x5A
REG_BCLK_PAD_CTRL   = 0x5B
REG_FSYNC_PAD_CTRL  = 0x5C
REG_INT0_PAD_CTRL   = 0x5D
REG_INT1_PAD_CTRL   = 0x5E

# =============================================================================
# Reset Values
# =============================================================================

RESET_DEVID_AD    = 0xAD
RESET_DEVID_MST   = 0x1D
RESET_PART_ID     = 0x17
RESET_PART_ID_REV = 0xC3
RESET_STATUS0     = 0x80
RESET_STATUS2     = 0x04

# =============================================================================
# Status Bit Masks
# =============================================================================

# STATUS0
STATUS0_NVM_BUSY           = (1 << 7)
STATUS0_FIFO_FULL          = (1 << 6)
STATUS0_FIFO_OVR           = (1 << 5)
STATUS0_FIFO_WM            = (1 << 4)
STATUS0_FIFO_RDY           = (1 << 3)
STATUS0_PARITY_ERR_STICKY  = (1 << 1)

# STATUS1
STATUS1_NVM_IRQ      = (1 << 7)
STATUS1_ACT          = (1 << 4)
STATUS1_INACT        = (1 << 3)
STATUS1_OR           = (1 << 2)
STATUS1_DOUBLE_TAP   = (1 << 1)
STATUS1_SINGLE_TAP   = (1 << 0)

# STATUS2
STATUS2_NVM_ECC_ERR     = (1 << 3)
STATUS2_NVM_CRC_ERR     = (1 << 2)
STATUS2_UV_FLAG_STICKY  = (1 << 0)

# STATUS3
STATUS3_DATA_READY = (1 << 0)

# =============================================================================
# OP_MODE Register Bit Fields
# =============================================================================

OP_MODE_RANGE_MSK   = 0xC0
OP_MODE_RANGE_POS   = 6
OP_MODE_PDM_MODE    = (1 << 5)
OP_MODE_AUDIO_MODE  = (1 << 4)
OP_MODE_MODE_MSK    = 0x0F

# =============================================================================
# DIG_EN Register Bit Fields
# =============================================================================

DIG_EN_TEMP_EN      = (1 << 7)
DIG_EN_Z_EN         = (1 << 6)
DIG_EN_Y_EN         = (1 << 5)
DIG_EN_X_EN         = (1 << 4)
DIG_EN_FIFO_EN      = (1 << 3)
DIG_EN_DOUBLE_SPEED = (1 << 2)
DIG_EN_INT01_EVENT  = (1 << 1)
DIG_EN_PARITY_EN    = (1 << 0)

# =============================================================================
# REG_RESET, SNSR_AXIS_EN, FILTER, SPI
# =============================================================================

REG_RESET_CODE = 0x52

SNSR_AXIS_EN_ST_MODE  = (1 << 7)
SNSR_AXIS_EN_ST_FORCE = (1 << 6)
SNSR_AXIS_EN_ST_DIR   = (1 << 5)

FILTER_DCF_BYPASS     = (1 << 7)
FILTER_EQ_BYPASS      = (1 << 6)
FILTER_LPF_MODE_MSK   = 0x30
FILTER_LPF_MODE_POS   = 4
FILTER_HPF_PATH       = (1 << 3)
FILTER_HPF_CORNER_MSK = 0x07

SPI_READ_BIT   = 0x01
SPI_WRITE_BIT  = 0x00
SPI_ADDR_SHIFT = 1

# =============================================================================
# Operating Modes
# =============================================================================


class OpMode(IntEnum):
    STANDBY  = 0x00
    HS       = 0x01
    ULP      = 0x02
    VLP      = 0x03
    LP       = 0x04
    LP_ULP   = 0x06
    LP_VLP   = 0x07
    RBW      = 0x08
    RBW_ULP  = 0x0A
    RBW_VLP  = 0x0B
    HP       = 0x0C
    HP_ULP   = 0x0E
    HP_VLP   = 0x0F


class Range(IntEnum):
    RANGE_4G  = 0x00
    RANGE_8G  = 0x01
    RANGE_16G = 0x02


class FifoMode(IntEnum):
    DISABLED  = 0x00
    OLDEST    = 0x01
    STREAM    = 0x02
    TRIGGERED = 0x03

# =============================================================================
# Sensitivity, Temperature, Self-Test, Timing Constants
# =============================================================================

SENSITIVITY_4G  = 7500   # LSB/g
SENSITIVITY_8G  = 3750
SENSITIVITY_16G = 1875

SENSITIVITY_MAP = {
    Range.RANGE_4G:  SENSITIVITY_4G,
    Range.RANGE_8G:  SENSITIVITY_8G,
    Range.RANGE_16G: SENSITIVITY_16G,
}

TEMP_OFFSET_AT_25C    = 550     # LSB at 25°C
TEMP_SENSITIVITY      = 10.2    # LSB/°C
TEMP_SENSITIVITY_HG   = 16.5    # LSB/°C high-gain

SELF_TEST_XY_MIN_MG = 2500
SELF_TEST_XY_MAX_MG = 5500
SELF_TEST_Z_MIN_MG  = 2000
SELF_TEST_Z_MAX_MG  = 3800

RESET_LATENCY_US   = 500
NVM_REFRESH_US     = 2200
STARTUP_DELAY_US   = RESET_LATENCY_US + NVM_REFRESH_US
