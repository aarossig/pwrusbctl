/*
 * Copyright 2017 Andrew Rossignol andrew.rossignol@gmail.com
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hidapi.h>

#include <cstdint>

#include "util/noncopyable.h"

namespace pwrusbctl {

/**
 * Models the state of a socket.
 */
enum class SocketState {
  //! Notates that a socket is powered off.
  Off,

  //! Notates that a socket is powered on.
  On,
};

/**
 * A class to model and control the state of a PowerUSB-branded power bar. This
 * device is interfaced with via the USB HID protocol. The HIDAPI is used as
 * a platform abstraction over the HID implementation.
 */
class PowerUsbDevice : public NonCopyable {
 public:
  /**
   * A small helper function to convert charge and a line voltage into energy.
   * Charge can be obtained from the GetAccumulatedCharge method and is in
   * milliamp-minute form. The return is expressed in kilowatt-hours.
   *
   * @param miliamp_seconds The amount of charge in milliamp-seconds.
   * @param line_voltage The approximate AC voltage of the line.
   * @return The amount of energy expressed in kWh.
   */
  static float ConvertChargeToKilowattHours(int32_t milliamp_minutes,
                                            float line_voltage);

  /**
   * Constructs a PowerUsbDevice by opening the first (or only) PowerUSB
   * device attached to the system. This library currently only supports
   * interfacing with one device at a time (and defaults to the first
   * available).
   *
   * The return value of IsInitialized() must be checked prior to invoking any
   * of the methods that read or write the state of the device.
   */
  PowerUsbDevice();

  /**
   * Release the PowerUsbDevice by closing the device.
   */
  ~PowerUsbDevice();

  /**
   * Determines whether or not the device has been initialized. This method must
   * be invoked before any other API provided by this class. If this method
   * returns false, no PowerUsbDevice was available to be opened.
   *
   * @return Returns true if the PowerUsbDevice was opened correctly.
   */
  bool IsInitialized() const;

  /**
   * Obtains the number of sockets that the PowerUsbDevice has that can be
   * controlled via USB. This is helpful to know the maximum index that can
   * be used to set/get socket state.
   *
   * @return The number of switchable sockets.
   */
  size_t GetSocketCount() const;

  /**
   * Returns a string describing the type of the device. If an error occurs,
   * nullptr is returned.
   *
   * @return The type of the device.
   */
  const char *GetDeviceType() const;

  /**
   * Sets the state of a power outlet given an index. If the index is out of
   * range for the connected device no operation is performed. For debug builds
   * (those compiled without NDEBUG), an assertion will catch this error. For
   * NDEBUG builds, false will be returned. False is also returned if an error
   * in HIDAPI occurs.
   *
   * @param index The index of the socket to set.
   * @param state The state to set the index of the socket to.
   * @return Returns false if an error occurs.
   */
  bool SetSocketState(size_t index, SocketState state) const;

  /**
   * Sets the default state of a power outlet given an index. If the index is
   * out of range for the connected device no operation is performed. For debug
   * builds (those compiled without NDEBUG), an assertion will catch this error.
   * For NDEBUG builds, false will be returned False is also returned if an
   * error in HIDAPI occurs.
   *
   * @param index The index of the socket to set.
   * @param state The default state to set the index of the socket to.
   * @return Returns false if an error occurs.
   */
  bool SetDefaultSocketState(size_t index, SocketState state) const;

  /**
   * Obtains the total instantaneous current as measured by the power strip.
   * This includes all outlets including the unswitched 15A outlet. False is
   * returned if an error communicating with the device occurs.
   *
   * @param current A pointer to a fixed-size int to populate with the current.
   * @return Returns false if an error occurs.
   */
  bool GetInstantaneousCurrent(int16_t *current) const;

  /**
   * Obtains the total accumulated charge as measured by the power strip. The
   * power strip has a built-in function to integrate charge iminutesn
   * milliamp-minutes. You must convert this to Wh or kWh using a known voltage
   * that the device does not measure. A static helper function is provided
   * in this class to convert this to power.
   *
   * @param accumulated_charge A pointer to a fixed-size integer to populate
   *                           with the accumulated charge.
   * @return Returns false if an eccur occurs.
   */
  bool GetAccumulatedCharge(int32_t *accumulated_charge) const;

  /**
   * Resets the charge accumulator.
   *
   * @return Returns false if an error occurs.
   */
  bool ResetChargeAccumulator() const;

 private:
  //! The underlying HID device used to communicate with the PowerUSB device.
  hid_device *device_;

  /**
   * Writes a buffer to the underlying device. If an error occurs, false is
   * returned.
   *
   * @param buffer The buffer of data to write.
   * @param length The length of the buffer to write.
   * @return Returns false if an error occurs.
   */
  bool DeviceWrite(const uint8_t *buffer, size_t length) const;

  /**
   * Reads from the underlying device into a buffer. If an error occurs,
   * false is returned.
   *
   * @param buffer The buffer to read into.
   * @param length The size of the buffer.
   * @return Returns false if an error occurs.
   */
  bool DeviceRead(uint8_t *buffer, size_t length) const;
};

}  // namespace pwrusbctl
