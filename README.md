# ADXL380 Accelerometer Driver

Multi-language driver for the Analog Devices ADXL380 low-noise, low-drift, wide-bandwidth, 3-axis MEMS accelerometer.

## Features

- SPI interface (16-bit half-duplex, Mode 0)
- ±4g, ±8g, ±16g selectable range
- 13 operating modes including concurrent modes
- 12-bit integrated temperature sensor
- FIFO support (Oldest Saved, Stream, Triggered)
- Activity/Inactivity detection
- Single/Double tap detection
- Built-in self-test
- C, C++, and Python implementations

## Repository Structure

```
ADXL380/
├── lib/
│   ├── C/
│   │   ├── adxl380_driver.h    # C driver header (register map, types, API)
│   │   └── adxl380_driver.c    # C driver implementation
│   ├── Cpp/
│   │   ├── adxl380_driver.hpp  # C++ RAII wrapper
│   │   └── adxl380_driver.cpp  # C++ implementation
│   └── Python/
│       ├── adxl380_driver_constants.py  # Constants and register map
│       └── adxl380.py                   # Python driver class
├── examples/
│   ├── C/main.c
│   ├── Cpp/main.cpp
│   └── Python/main.py
├── docs/
│   └── adxl380.pdf             # Datasheet
└── utilities/
    ├── format.cfg
    └── format.sh
```

## Hardware Overview

The ADXL380 is a 3-axis MEMS accelerometer from Analog Devices.

- **SPI communication**: 7-bit address + R/W bit + 8-bit data (16-bit frame)
- **SPI Mode**: CPOL=0, CPHA=0 (SPI Mode 0)
- **R/W bit**: 1 = read, 0 = write
- **Device IDs**: DEVID_AD = `0xAD`, DEVID_MST = `0x1D`, PART_ID = `0x17`

## Quick Start — C

```c
#include "adxl380_driver.h"

/* Platform-specific SPI and delay must be provided */
static int my_spi_xfer(uint8_t *tx, uint8_t *rx, uint16_t len, void *ctx);
static void my_delay_us(uint32_t us, void *ctx);

int main(void) {
    adxl380_dev_t dev = {0};
    dev.spi_xfer  = my_spi_xfer;
    dev.delay_us  = my_delay_us;
    dev.user_data = NULL;

    adxl380_init(&dev);
    adxl380_set_range(&dev, ADXL380_RANGE_8G);
    adxl380_enable_channels(&dev, true, true, true, false);
    adxl380_set_op_mode(&dev, ADXL380_MODE_HP);

    adxl380_accel_data_t data;
    adxl380_get_accel_data(&dev, &data);
    printf("X=%d Y=%d Z=%d\n", data.x, data.y, data.z);

    return 0;
}
```

## Quick Start — C++

```cpp
#include "adxl380_driver.hpp"

int main() {
    adxl380::Accelerometer accel(my_spi_xfer, my_delay_us);
    accel.init();
    accel.setRange(ADXL380_RANGE_8G);
    accel.enableChannels(true, true, true, false);
    accel.setOpMode(ADXL380_MODE_HP);

    auto data = accel.getAccelData();
    printf("X=%d Y=%d Z=%d\n", data.x, data.y, data.z);

    return 0;
}
```

## Quick Start — Python

```python
from adxl380 import ADXL380
import adxl380_driver_constants as const

accel = ADXL380(spi_xfer=my_spi_xfer)
accel.init()
accel.set_range(const.Range.RANGE_8G)
accel.enable_channels(x=True, y=True, z=True)
accel.set_op_mode(const.OpMode.HP)

data = accel.get_accel_data()
print(f"X={data.x} Y={data.y} Z={data.z}")
```

## API Reference — C

### Initialization

| Function | Description |
|----------|-------------|
| `adxl380_init()` | Initialize device: soft reset, verify IDs, set defaults |
| `adxl380_soft_reset()` | Write reset code (`0x52`) to `REG_RESET` register |

### Configuration

| Function | Description |
|----------|-------------|
| `adxl380_set_op_mode()` | Set operating mode (standby, HP, LP, ULP, etc.) |
| `adxl380_get_op_mode()` | Read current operating mode |
| `adxl380_set_range()` | Set acceleration range (±4g, ±8g, ±16g) |
| `adxl380_get_range()` | Read current range setting |
| `adxl380_enable_channels()` | Enable/disable X, Y, Z, Temp channels |

### Data Acquisition

| Function | Description |
|----------|-------------|
| `adxl380_get_accel_data()` | Read X, Y, Z acceleration (16-bit signed) |
| `adxl380_get_temp_raw()` | Read raw 12-bit temperature |
| `adxl380_convert_temp_c()` | Convert raw temp to °C |
| `adxl380_is_data_ready()` | Check `DATA_READY` bit in `STATUS3` |
| `adxl380_get_sensitivity()` | Get sensitivity in LSB/g for given range |

### Self-Test

| Function | Description |
|----------|-------------|
| `adxl380_self_test()` | Full self-test procedure per datasheet |

### FIFO

| Function | Description |
|----------|-------------|
| `adxl380_set_fifo_mode()` | Set FIFO mode (disabled, oldest, stream, triggered) |
| `adxl380_get_fifo_entries()` | Read number of FIFO entries available |
| `adxl380_read_fifo()` | Read data from FIFO |

### Interrupts & Detection

| Function | Description |
|----------|-------------|
| `adxl380_set_int_map()` | Configure INT0/INT1 mapping |
| `adxl380_set_act_threshold()` | Set activity detection threshold |
| `adxl380_set_inact_threshold()` | Set inactivity detection threshold |
| `adxl380_set_act_time()` | Set activity time (24-bit) |
| `adxl380_set_inact_time()` | Set inactivity time (24-bit) |
| `adxl380_set_act_inact_ctl()` | Configure activity/inactivity control |
| `adxl380_set_tap_config()` | Configure tap detection parameters |

### Status & ID

| Function | Description |
|----------|-------------|
| `adxl380_get_status()` | Read all four status registers |
| `adxl380_get_device_id()` | Read DEVID_AD, DEVID_MST, PART_ID |

### Register Access

| Function | Description |
|----------|-------------|
| `adxl380_read_reg()` | Read single register |
| `adxl380_write_reg()` | Write single register |
| `adxl380_read_regs()` | Read consecutive registers |
| `adxl380_write_regs()` | Write consecutive registers |

## Operating Modes

| Mode | Value | Description |
|------|-------|-------------|
| Standby | `0x00` | No measurements, lowest power |
| Heart Sound | `0x01` | Heart sound detection mode |
| Ultra Low Power | `0x02` | Minimum power consumption |
| Very Low Power | `0x03` | Very low power |
| Low Power | `0x04` | Low power |
| LP + ULP | `0x06` | Concurrent LP and ULP |
| LP + VLP | `0x07` | Concurrent LP and VLP |
| Reduced Bandwidth | `0x08` | Reduced bandwidth |
| RBW + ULP | `0x0A` | Concurrent RBW and ULP |
| RBW + VLP | `0x0B` | Concurrent RBW and VLP |
| High Performance | `0x0C` | Maximum performance |
| HP + ULP | `0x0E` | Concurrent HP and ULP |
| HP + VLP | `0x0F` | Concurrent HP and VLP |

## Range and Sensitivity

| Range | Sensitivity | Resolution |
|-------|-------------|------------|
| ±4g | 7500 LSB/g | 133.3 µg/LSB |
| ±8g | 3750 LSB/g | 266.7 µg/LSB |
| ±16g | 1875 LSB/g | 533.3 µg/LSB |

## Temperature Sensor

- 12-bit integrated sensor (twos complement)
- Output at 25°C: 550 LSB
- Default sensitivity: 10.2 LSB/°C
- High-gain sensitivity: 16.5 LSB/°C

## Porting Guide

To use this driver on a new platform, provide two callbacks:

1. **SPI transfer function**

   ```c
   int spi_xfer(uint8_t *tx, uint8_t *rx, uint16_t len, void *user_data);
   ```

   Must perform a full-duplex SPI transfer of `len` bytes. Returns 0 on success, non-zero on failure.

2. **Delay function**

   ```c
   void delay_us(uint32_t us, void *user_data);
   ```

   Must block for at least `us` microseconds.

3. **Optional `user_data` pointer**

   An opaque pointer stored in the device handle and forwarded to both callbacks. Use it to pass platform-specific context (e.g., SPI peripheral handle, GPIO chip-select).

### C

```c
adxl380_dev_t dev = {0};
dev.spi_xfer  = my_spi_xfer;
dev.delay_us  = my_delay_us;
dev.user_data = &my_platform_ctx;
```

### C++

```cpp
adxl380::Accelerometer accel(my_spi_xfer, my_delay_us, &my_platform_ctx);
```

### Python

```python
accel = ADXL380(spi_xfer=my_spi_xfer, delay_us=my_delay_us)
```

The Python driver's `spi_xfer` has a simpler signature: `spi_xfer(tx_data: bytes) -> bytes`. It sends `tx_data` and returns the received bytes of the same length. If `delay_us` is omitted, `time.sleep()` is used as a fallback.

## License

See the [LICENSE](LICENSE) file for details.
