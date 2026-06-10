"""ADXL380 accelerometer driver."""

import time
from typing import Callable, NamedTuple, Optional, Tuple

try:
    from . import adxl380_driver_constants as const
except ImportError:
    import adxl380_driver_constants as const


class AccelData(NamedTuple):
    x: int
    y: int
    z: int


class ADXL380Error(Exception):
    """Exception raised for ADXL380 driver errors."""
    pass


class ADXL380:
    """ADXL380 accelerometer driver.

    Args:
        spi_xfer: Callable that performs SPI transfer.
                  Signature: spi_xfer(tx_data: bytes) -> bytes
                  Sends tx_data and returns received bytes of same length.
        delay_us: Optional callable for microsecond delays.
                  Signature: delay_us(microseconds: int) -> None
                  If None, uses time.sleep().
    """

    def __init__(self, spi_xfer: Callable[[bytes], bytes],
                 delay_us: Optional[Callable[[int], None]] = None):
        self._spi_xfer = spi_xfer
        self._delay_us = delay_us or self._default_delay_us
        self._range = const.Range.RANGE_4G
        self._op_mode = const.OpMode.STANDBY

    @staticmethod
    def _default_delay_us(us: int) -> None:
        time.sleep(us / 1_000_000)

    # --- Register Access ---

    def read_reg(self, reg: int) -> int:
        """Read a single register. Returns the byte value."""
        tx = bytes([(reg << const.SPI_ADDR_SHIFT) | const.SPI_READ_BIT, 0x00])
        rx = self._spi_xfer(tx)
        return rx[1]

    def write_reg(self, reg: int, val: int) -> None:
        """Write a single register."""
        tx = bytes([(reg << const.SPI_ADDR_SHIFT) | const.SPI_WRITE_BIT, val & 0xFF])
        self._spi_xfer(tx)

    def read_regs(self, reg: int, length: int) -> bytes:
        """Read multiple consecutive registers."""
        result = bytearray(length)
        for i in range(length):
            result[i] = self.read_reg(reg + i)
        return bytes(result)

    def write_regs(self, reg: int, data: bytes) -> None:
        """Write multiple consecutive registers."""
        for i, val in enumerate(data):
            self.write_reg(reg + i, val)

    # --- Init & Reset ---

    def init(self) -> None:
        """Initialize the ADXL380. Performs soft reset and verifies device ID."""
        self.soft_reset()
        self._delay_us(const.STARTUP_DELAY_US)

        devid_ad = self.read_reg(const.REG_DEVID_AD)
        devid_mst = self.read_reg(const.REG_DEVID_MST)
        part_id = self.read_reg(const.REG_PART_ID)

        if devid_ad != const.RESET_DEVID_AD:
            raise ADXL380Error(
                f"DEVID_AD mismatch: expected 0x{const.RESET_DEVID_AD:02X}, "
                f"got 0x{devid_ad:02X}"
            )
        if devid_mst != const.RESET_DEVID_MST:
            raise ADXL380Error(
                f"DEVID_MST mismatch: expected 0x{const.RESET_DEVID_MST:02X}, "
                f"got 0x{devid_mst:02X}"
            )
        if part_id != const.RESET_PART_ID:
            raise ADXL380Error(
                f"PART_ID mismatch: expected 0x{const.RESET_PART_ID:02X}, "
                f"got 0x{part_id:02X}"
            )

        self._range = const.Range.RANGE_4G
        self._op_mode = const.OpMode.STANDBY

    def soft_reset(self) -> None:
        """Perform a software reset."""
        self.write_reg(const.REG_REG_RESET, const.REG_RESET_CODE)

    # --- Configuration ---

    def set_op_mode(self, mode: const.OpMode) -> None:
        """Set operating mode."""
        val = self.read_reg(const.REG_OP_MODE)
        val = (val & ~const.OP_MODE_MODE_MSK) | (int(mode) & const.OP_MODE_MODE_MSK)
        self.write_reg(const.REG_OP_MODE, val)
        self._op_mode = mode

    def get_op_mode(self) -> const.OpMode:
        """Get current operating mode from device."""
        val = self.read_reg(const.REG_OP_MODE)
        return const.OpMode(val & const.OP_MODE_MODE_MSK)

    def set_range(self, range_val: const.Range) -> None:
        """Set acceleration range (±4g, ±8g, or ±16g)."""
        val = self.read_reg(const.REG_OP_MODE)
        val = (val & ~const.OP_MODE_RANGE_MSK) | (int(range_val) << const.OP_MODE_RANGE_POS)
        self.write_reg(const.REG_OP_MODE, val)
        self._range = range_val

    def get_range(self) -> const.Range:
        """Get current range setting from device."""
        val = self.read_reg(const.REG_OP_MODE)
        return const.Range((val & const.OP_MODE_RANGE_MSK) >> const.OP_MODE_RANGE_POS)

    # --- Channel Enable ---

    def enable_channels(self, x: bool = True, y: bool = True,
                        z: bool = True, temp: bool = False) -> None:
        """Enable/disable measurement channels."""
        val = self.read_reg(const.REG_DIG_EN)
        val &= 0x0F  # Clear channel bits [7:4]
        if x:
            val |= const.DIG_EN_X_EN
        if y:
            val |= const.DIG_EN_Y_EN
        if z:
            val |= const.DIG_EN_Z_EN
        if temp:
            val |= const.DIG_EN_TEMP_EN
        self.write_reg(const.REG_DIG_EN, val)

    # --- Data Acquisition ---

    def get_accel_data(self) -> AccelData:
        """Read acceleration data (raw 16-bit signed values)."""
        buf = self.read_regs(const.REG_XDATA_H, 6)
        x = _to_int16((buf[0] << 8) | buf[1])
        y = _to_int16((buf[2] << 8) | buf[3])
        z = _to_int16((buf[4] << 8) | buf[5])
        return AccelData(x, y, z)

    def get_temp_raw(self) -> int:
        """Read raw 12-bit temperature value (twos complement)."""
        buf = self.read_regs(const.REG_TDATA_H, 2)
        raw = (buf[0] << 4) | (buf[1] >> 4)
        if raw & 0x800:  # Sign-extend from 12-bit
            raw |= 0xF000
        return _to_int16(raw)

    def get_temp_celsius(self) -> float:
        """Read temperature in degrees Celsius."""
        raw = self.get_temp_raw()
        return 25.0 + (raw - const.TEMP_OFFSET_AT_25C) / const.TEMP_SENSITIVITY

    def is_data_ready(self) -> bool:
        """Check if new acceleration data is ready."""
        val = self.read_reg(const.REG_STATUS3)
        return bool(val & const.STATUS3_DATA_READY)

    # --- Sensitivity ---

    def get_sensitivity(self) -> int:
        """Get sensitivity for current range in LSB/g."""
        return const.SENSITIVITY_MAP.get(self._range, 0)

    # --- Self-Test ---

    def self_test(self) -> bool:
        """Run self-test procedure per datasheet. Returns True if passed."""
        # Save state
        saved_mode = self._op_mode
        saved_range = self._range

        try:
            # Setup: standby, ±8g, enable XYZ, HP mode
            self.set_op_mode(const.OpMode.STANDBY)
            self.set_range(const.Range.RANGE_8G)
            self.enable_channels(x=True, y=True, z=True, temp=False)
            self.set_op_mode(const.OpMode.HP)

            # Wait for data and read baseline (discard first sample)
            self._wait_data_ready()
            _ = self.get_accel_data()  # discard first

            # Positive self-test
            reg_val = self.read_reg(const.REG_SNSR_AXIS_EN)
            reg_val = ((reg_val & 0x1F)
                       | const.SNSR_AXIS_EN_ST_MODE
                       | const.SNSR_AXIS_EN_ST_FORCE)
            self.write_reg(const.REG_SNSR_AXIS_EN, reg_val)
            self._delay_us(50000)  # 50ms settling

            pos = self._average_samples(4)

            # Negative self-test
            reg_val |= const.SNSR_AXIS_EN_ST_DIR
            self.write_reg(const.REG_SNSR_AXIS_EN, reg_val)
            self._delay_us(50000)

            neg = self._average_samples(4)

            # Disable self-test
            reg_val = self.read_reg(const.REG_SNSR_AXIS_EN)
            reg_val &= ~(const.SNSR_AXIS_EN_ST_MODE
                         | const.SNSR_AXIS_EN_ST_FORCE
                         | const.SNSR_AXIS_EN_ST_DIR)
            self.write_reg(const.REG_SNSR_AXIS_EN, reg_val)

            # Compute deltas in mg
            sensitivity = const.SENSITIVITY_8G
            dx_mg = abs(pos.x - neg.x) * 1000 // sensitivity
            dy_mg = abs(pos.y - neg.y) * 1000 // sensitivity
            dz_mg = abs(pos.z - neg.z) * 1000 // sensitivity

            x_pass = const.SELF_TEST_XY_MIN_MG <= dx_mg <= const.SELF_TEST_XY_MAX_MG
            y_pass = const.SELF_TEST_XY_MIN_MG <= dy_mg <= const.SELF_TEST_XY_MAX_MG
            z_pass = const.SELF_TEST_Z_MIN_MG <= dz_mg <= const.SELF_TEST_Z_MAX_MG

            return x_pass and y_pass and z_pass
        finally:
            # Restore original settings
            self.set_op_mode(const.OpMode.STANDBY)
            self.set_range(saved_range)
            self.set_op_mode(saved_mode)

    # --- FIFO ---

    def set_fifo_mode(self, mode: const.FifoMode) -> None:
        """Set FIFO operating mode."""
        val = self.read_reg(const.REG_FIFO_CFG0)
        val = (val & 0xFC) | (int(mode) & 0x03)
        self.write_reg(const.REG_FIFO_CFG0, val)

    def get_fifo_entries(self) -> int:
        """Get current number of FIFO entries."""
        s0 = self.read_reg(const.REG_FIFO_STATUS0)
        s1 = self.read_reg(const.REG_FIFO_STATUS1)
        return ((s1 & 0x01) << 8) | s0

    def read_fifo(self, count: int) -> bytes:
        """Read count bytes from FIFO data register."""
        result = bytearray(count)
        for i in range(count):
            result[i] = self.read_reg(const.REG_FIFO_DATA)
        return bytes(result)

    # --- Interrupts ---

    def set_int_map(self, int_num: int, map0: int, map1: int) -> None:
        """Configure interrupt mapping registers for INT0 or INT1."""
        if int_num == 0:
            self.write_reg(const.REG_INT0_MAP0, map0)
            self.write_reg(const.REG_INT0_MAP1, map1)
        elif int_num == 1:
            self.write_reg(const.REG_INT1_MAP0, map0)
            self.write_reg(const.REG_INT1_MAP1, map1)
        else:
            raise ValueError(f"Invalid interrupt number: {int_num}")

    # --- Activity/Inactivity ---

    def set_act_threshold(self, threshold: int) -> None:
        """Set activity detection threshold (16-bit)."""
        self.write_reg(const.REG_THRESH_ACT_H, (threshold >> 8) & 0xFF)
        self.write_reg(const.REG_THRESH_ACT_L, threshold & 0xFF)

    def set_inact_threshold(self, threshold: int) -> None:
        """Set inactivity detection threshold (16-bit)."""
        self.write_reg(const.REG_THRESH_INACT_H, (threshold >> 8) & 0xFF)
        self.write_reg(const.REG_THRESH_INACT_L, threshold & 0xFF)

    def set_act_time(self, time_val: int) -> None:
        """Set activity detection time (24-bit)."""
        self.write_reg(const.REG_TIME_ACT_H, (time_val >> 16) & 0xFF)
        self.write_reg(const.REG_TIME_ACT_M, (time_val >> 8) & 0xFF)
        self.write_reg(const.REG_TIME_ACT_L, time_val & 0xFF)

    def set_inact_time(self, time_val: int) -> None:
        """Set inactivity detection time (24-bit)."""
        self.write_reg(const.REG_TIME_INACT_H, (time_val >> 16) & 0xFF)
        self.write_reg(const.REG_TIME_INACT_M, (time_val >> 8) & 0xFF)
        self.write_reg(const.REG_TIME_INACT_L, time_val & 0xFF)

    def set_act_inact_ctl(self, ctl: int) -> None:
        """Set activity/inactivity control register."""
        self.write_reg(const.REG_ACT_INACT_CTL, ctl)

    # --- Tap Detection ---

    def set_tap_config(self, thresh: int, dur: int, latent: int,
                       window: int, cfg: int) -> None:
        """Configure tap detection parameters."""
        self.write_reg(const.REG_TAP_THRESH, thresh)
        self.write_reg(const.REG_TAP_DUR, dur)
        self.write_reg(const.REG_TAP_LATENT, latent)
        self.write_reg(const.REG_TAP_WINDOW, window)
        self.write_reg(const.REG_TAP_CFG, cfg)

    # --- Status ---

    def get_status(self) -> Tuple[int, int, int, int]:
        """Read all four status registers. Returns (status0, status1, status2, status3)."""
        s0 = self.read_reg(const.REG_STATUS0)
        s1 = self.read_reg(const.REG_STATUS1)
        s2 = self.read_reg(const.REG_STATUS2)
        s3 = self.read_reg(const.REG_STATUS3)
        return (s0, s1, s2, s3)

    # --- Device ID ---

    def get_device_id(self) -> Tuple[int, int, int]:
        """Read device IDs. Returns (devid_ad, devid_mst, part_id)."""
        return (
            self.read_reg(const.REG_DEVID_AD),
            self.read_reg(const.REG_DEVID_MST),
            self.read_reg(const.REG_PART_ID),
        )

    # --- Private Helpers ---

    def _wait_data_ready(self, timeout_us: int = 100000) -> None:
        """Wait for data ready with timeout."""
        elapsed = 0
        interval = 1000  # 1ms poll interval
        while elapsed < timeout_us:
            if self.is_data_ready():
                return
            self._delay_us(interval)
            elapsed += interval
        raise ADXL380Error("Timeout waiting for data ready")

    def _average_samples(self, count: int) -> AccelData:
        """Read and average multiple acceleration samples."""
        sx, sy, sz = 0, 0, 0
        for _ in range(count):
            self._wait_data_ready()
            d = self.get_accel_data()
            sx += d.x
            sy += d.y
            sz += d.z
        return AccelData(sx // count, sy // count, sz // count)


def _to_int16(val: int) -> int:
    """Convert unsigned 16-bit value to signed."""
    val &= 0xFFFF
    if val >= 0x8000:
        val -= 0x10000
    return val
