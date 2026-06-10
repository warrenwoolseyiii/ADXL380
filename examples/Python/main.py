#!/usr/bin/env python3
"""ADXL380 Python driver usage example.

Demonstrates initializing the ADXL380, configuring it, and reading
acceleration and temperature data using a stub SPI function.
"""

import sys
import os

# Add lib directory to path for imports
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..', 'lib', 'Python'))

from adxl380 import ADXL380, ADXL380Error, AccelData
import adxl380_driver_constants as const


def stub_spi_xfer(tx_data: bytes) -> bytes:
    """Stub SPI transfer function.

    In a real application, replace this with actual SPI hardware communication.
    The ADXL380 uses 16-bit half-duplex SPI (Mode 0, CPOL=0, CPHA=0).

    Args:
        tx_data: Bytes to transmit.

    Returns:
        Received bytes (same length as tx_data).
    """
    return bytes(len(tx_data))


def main():
    print("ADXL380 Python Driver Example")
    print("=============================\n")

    # Create driver instance with stub SPI
    accel = ADXL380(spi_xfer=stub_spi_xfer)

    try:
        # Initialize
        accel.init()
    except ADXL380Error as e:
        print(f"Init error: {e}")
        print("(Expected with stub SPI - replace with real HW)\n")
        # Continue to demonstrate API even with stubs

    # Print device IDs
    try:
        devid_ad, devid_mst, part_id = accel.get_device_id()
        print(f"Device ID:  AD=0x{devid_ad:02X}  MST=0x{devid_mst:02X}  PART=0x{part_id:02X}")
    except ADXL380Error as e:
        print(f"Device ID read error: {e}")

    # Configure range
    accel.set_range(const.Range.RANGE_8G)
    print(f"Range: +/-8g (sensitivity: {accel.get_sensitivity()} LSB/g)")

    # Enable channels
    accel.enable_channels(x=True, y=True, z=True, temp=True)
    print("Channels: X, Y, Z, Temp enabled")

    # Set operating mode
    accel.set_op_mode(const.OpMode.HP)
    print("Mode: High Performance\n")

    # Read acceleration data
    print("Reading acceleration data...")
    for i in range(5):
        try:
            data = accel.get_accel_data()
            print(f"  Sample {i}: X={data.x:6d}  Y={data.y:6d}  Z={data.z:6d}")
        except ADXL380Error as e:
            print(f"  Sample {i}: Error - {e}")

    # Read temperature
    try:
        temp_c = accel.get_temp_celsius()
        print(f"\nTemperature: {temp_c:.1f} °C")
    except ADXL380Error as e:
        print(f"\nTemperature error: {e}")

    # Check data ready
    ready = accel.is_data_ready()
    print(f"Data ready: {ready}")

    # Read status
    try:
        s0, s1, s2, s3 = accel.get_status()
        print(f"\nStatus: S0=0x{s0:02X} S1=0x{s1:02X} S2=0x{s2:02X} S3=0x{s3:02X}")
    except ADXL380Error as e:
        print(f"\nStatus error: {e}")

    # Self-test
    print("\nRunning self-test...")
    try:
        passed = accel.self_test()
        print(f"Self-test: {'PASSED' if passed else 'FAILED'}")
    except ADXL380Error as e:
        print(f"Self-test error: {e}")

    # Return to standby
    accel.set_op_mode(const.OpMode.STANDBY)
    print("\nDevice set to standby. Done.")


if __name__ == "__main__":
    main()
