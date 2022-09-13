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

#include <cassert>

#include "power_usb_device.h"

namespace pwrusbctl {

//! The vendor ID of the Power USB product line.
constexpr uint16_t kVendorId(0x04d8);

//! The product ID of the Power USB device.
constexpr uint16_t kProductId(0x003f);

//! The number of sockets attached to the PowerUsb device.
constexpr size_t kSocketCount(3);

//! The device types as described by the http://pwrusb.com/products.html
//! webpage. Note that this does not include the full name of the device and
//! only the variant string.
constexpr const char * const kDeviceTypes[] = {
  "Basic",
  "Digital IO",
  "Watchdog",
  "Smart"
};

//! The command used to obtain the type of the device.
constexpr uint8_t kGetDeviceTypeCommand(0xAA);

//! The command used to obtain the instantaneous current of the device.
constexpr uint8_t kGetInstantaneousCurrentCommand(0xB1);

//! The command used to obtain the accummulated charge of the device.
constexpr uint8_t kGetAccumulatedEnergyCommand(0xB2);

//! The command used to reset the charge accumulator in the device.
constexpr uint8_t kResetChargeAccumulatorCommand(0xB3);

//! The power off command values.
constexpr uint8_t kSetPowerOffCommands[] = {
  0x42,
  0x44,
  0x50,
};

//! The power on command values.
constexpr uint8_t kSetPowerOnCommands[] = {
  0x41,
  0x43,
  0x45,
};

//! The default power off command values.
constexpr uint8_t kSetDefaultPowerOffCommands[] = {
  0x46,
  0x51,
  0x48,
};

//! The default power on command values.
constexpr uint8_t kSetDefaultPowerOnCommands[] = {
  0x4E,
  0x47,
  0x4F,
};

float PowerUsbDevice::ConvertChargeToKilowattHours(int32_t milliamp_minutes,
                                                   float line_voltage) {
  float amp_hours = milliamp_minutes / 60.0f / 1000.0f;
  return (amp_hours * line_voltage) / 1000.0f;
}

PowerUsbDevice::PowerUsbDevice() {
  device_ = hid_open(kVendorId, kProductId, nullptr);
}

PowerUsbDevice::~PowerUsbDevice() {
  if (IsInitialized()) {
    hid_close(device_);
  }
}

bool PowerUsbDevice::IsInitialized() const {
  return (device_ != nullptr);
}

size_t PowerUsbDevice::GetSocketCount() const {
  // All of the PowerUSB devices currently available have 3 switchable outlets
  // so we simply return a constant here.
  return kSocketCount;
}

const char *PowerUsbDevice::GetDeviceType() const {
  uint8_t get_device_type = kGetDeviceTypeCommand;
  if (!DeviceWrite(&get_device_type, 1)) {
    return nullptr;
  }

  uint8_t device_type;
  if (!DeviceRead(&device_type, 1)) {
    return nullptr;
  }

  device_type--;
  if (device_type > 3) {
    return nullptr;
  }

  return kDeviceTypes[device_type];
}

bool PowerUsbDevice::SetSocketState(size_t index, SocketState state) const {
  // The index supplied must be less than 3. If the strip is indexed out of
  // bounds no operation is performed.
  assert(index < kSocketCount);
  if (index >= kSocketCount) {
    return false;
  }

  // Set the state of the socket.
  uint8_t command_value;
  if (state == SocketState::On) {
    command_value = kSetPowerOnCommands[index];
  } else if (state == SocketState::Off) {
    command_value = kSetPowerOffCommands[index];
  } else {
    // Handle the case of an unhandled socket state.
    assert(false);
  }

  return DeviceWrite(&command_value, 1);
}

bool PowerUsbDevice::SetDefaultSocketState(size_t index, SocketState state) const {
  // The index supplied must be less than 3. If the strip is indexed out of
  // bounds no operation is performed.
  assert(index < kSocketCount);
  if (index >= kSocketCount) {
    return false;
  }

  // Set the state of the socket.
  uint8_t command_value;
  if (state == SocketState::On) {
    command_value = kSetDefaultPowerOnCommands[index];
  } else if (state == SocketState::Off) {
    command_value = kSetDefaultPowerOffCommands[index];
  } else {
    // Handle the case of an unhandled socket state.
    assert(false);
  }

  return DeviceWrite(&command_value, 1);
}

bool PowerUsbDevice::GetInstantaneousCurrent(int16_t *current) const {
  assert(current);
  if (current == nullptr) {
    return false;
  }

  uint8_t get_instantaneous_current = kGetInstantaneousCurrentCommand;
  if (!DeviceWrite(&get_instantaneous_current, 1)) {
    return false;
  }

  uint8_t current_buffer[2];
  if (!DeviceRead(current_buffer, sizeof(current_buffer))) {
    return false;
  }

  *current = (current_buffer[0] << 8) | current_buffer[1];
  return true;
}

bool PowerUsbDevice::GetAccumulatedCharge(int32_t *accumulated_charge) const {
  assert(accumulated_charge);
  if (accumulated_charge == nullptr) {
    return false;
  }

  uint8_t get_accumulated_energy = kGetAccumulatedEnergyCommand;
  if (!DeviceWrite(&get_accumulated_energy, 1)) {
    return false;
  }

  uint8_t energy_buffer[4];
  if (!DeviceRead(energy_buffer, sizeof(energy_buffer))) {
    return false;
  }

  *accumulated_charge = (energy_buffer[0] << 24)
      | (energy_buffer[1] << 16)
      | (energy_buffer[2] << 8)
      | (energy_buffer[3]);
  return true;
}

bool PowerUsbDevice::ResetChargeAccumulator() const {
  uint8_t reset_charge_accumulator = kResetChargeAccumulatorCommand;
  return DeviceWrite(&reset_charge_accumulator, 1);
}

bool PowerUsbDevice::DeviceWrite(const uint8_t *buffer, size_t length) const {
  int state = hid_write(device_, buffer, length);
  return (state != -1);
}

bool PowerUsbDevice::DeviceRead(uint8_t *buffer, size_t length) const {
  int state = hid_read(device_, buffer, length);
  return (state != -1);
}

}  // namespace pwrusbctl
