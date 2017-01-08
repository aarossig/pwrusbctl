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

#include <cinttypes>
#include <cstdio>
#include <unistd.h>
#include <tclap/CmdLine.h>

#include "power_usb_device.h"

using namespace pwrusbctl;

//! A description of this tool.
constexpr char kToolDescription[] =
    "a tool for interacting with PowerUSB USB-controlled power strips";

//! The current version of this tool. Defined according to the rules of
//! semantic versioning.
constexpr char kVersionString[] = "0.1.0";

//! The default logging interval of 200ms.
constexpr useconds_t kDefaultLoggingIntervalUs(200000);

//! The default line voltage when computing energy consumption.
constexpr float kDefaultLineVoltage(115.0f);

/**
 * A configuration for how to log data from the PowerUsb device.
 */
struct LoggingConfig {
  //! Whether or not current should be logged.
  bool log_current;

  //! Whether or not power should be logged.
  bool log_power;

  //! Whether or not the total energy should be logged.
  bool log_energy;

  //! The line voltage used for energy computation.
  float line_voltage;

  //! Whether or not logs should be emitted indefinitely.
  bool log_indefinitely;

  //! The number of times to log (if log_indefinitely is set to false).
  size_t log_count;

  //! The interval between logs (if logging more than once).
  int interval_us;

  /**
   * Determines whether or not any log statements will be printed given the
   * configuration.
   *
   * @return Returns true if a log message will be emitted in the current
   * configuration.
   */
  bool LogsEnabled() const {
    return log_current|| log_power || log_energy;
  }
};

/**
 * Releases any global resources and quits with an error.
 */
void CleanupAndAbort() {
  hid_exit();
  exit(-1);
}

/**
 * Prints the device type and logs any errors.
 *
 * @param device The device to print the type of.
 */
void PrintDeviceType(const PowerUsbDevice& device) {
  const char *deviceType = device.GetDeviceType();
  if (deviceType) {
    fprintf(stdout, "Found PowerUSB device type: %s\n", deviceType);
  } else {
    fprintf(stderr, "Error getting device info\n");
    CleanupAndAbort();
  }
}

/**
 * Resets the charge accumulator and logs any errors.
 *
 * @param device The PowerUsbDevice to be reset.
 */
void ResetChargeAccumulator(const PowerUsbDevice& device) {
  if (!device.ResetChargeAccumulator()) {
    fprintf(stderr, "Error resetting charge accumulator\n");
    CleanupAndAbort();
  }
}

/**
 * Sets the state of a socket and logs any errors.
 *
 * @param device The PowerUsbDevice to manipulate.
 * @param outlet_index The index of the outlet to set.
 * @param state The state to set the outlet to.
 */
void SetSocketState(const PowerUsbDevice& device, size_t outlet_index,
                    SocketState socket_state) {
  if (!device.SetSocketState(outlet_index, socket_state)) {
    fprintf(stderr, "Error setting socket state\n");
    CleanupAndAbort();
  }
}

/**
 * Sets the state of a socket and logs any errors.
 *
 * @param device The PowerUsbDevice to manipulate.
 * @param outlet_index The index of the outlet to set.
 * @param state The state to set the outlet to.
 */
void SetDefaultSocketState(const PowerUsbDevice& device, size_t outlet_index,
                           SocketState socket_state) {
  if (!device.SetDefaultSocketState(outlet_index, socket_state)) {
    fprintf(stderr, "Error setting socket state\n");
    CleanupAndAbort();
  }
}

/**
 * Log information about the power strip based on configurable arguments.
 *
 * @param device The device to print info about.
 * @param config The configuration of the logs.
 */
void LogStats(const PowerUsbDevice& device, const LoggingConfig& config) {
  for (size_t i = 0; config.log_indefinitely || i < config.log_count; i++) {
    if (config.log_current || config.log_power) {
      int16_t current;
      if (device.GetInstantaneousCurrent(&current)) {
        if (config.log_current) {
          fprintf(stdout, "Current: %" PRId16 "mA\n", current);
        }

        if (config.log_power) {
          float power = (current / 1000.0f) * config.line_voltage;
          fprintf(stdout, "Power: %fW\n", power);
        }
      } else {
        fprintf(stderr, "Error reading device current\n");
        CleanupAndAbort();
      }
    }

    if (config.log_energy) {
      int32_t milliamp_minutes;
      if (device.GetAccumulatedCharge(&milliamp_minutes)) {
        float energy = PowerUsbDevice::ConvertChargeToKilowattHours(
            milliamp_minutes, config.line_voltage);
        fprintf(stdout, "Energy: %fkWh\n", energy);
      } else {
        CleanupAndAbort();
      }
    }

    // Sleep if logs will be printed more than once.
    if (config.log_indefinitely || config.log_count != 1) {
      usleep(config.interval_us);
    }
  }
}

int main(int argc, char **argv) {
  using TCLAP::Arg;
  using TCLAP::CmdLine;
  using TCLAP::SwitchArg;
  using TCLAP::ValueArg;

  // Define the command line object with the description of this tool.
  CmdLine cmd(kToolDescription, ' ', kVersionString);

  // Logging args.
  SwitchArg print_device_info_arg("", "device_info",
      "Print device information", cmd, false);
  SwitchArg reset_charge_accumulator_arg("", "reset_charge_accumulator",
      "Resets the charge accumulator", cmd, false);
  SwitchArg log_current_arg("", "current",
      "Print the current used by attached devices", cmd, false);
  SwitchArg log_power_arg("", "power",
      "Print the power used by attached devices", cmd, false);
  SwitchArg log_energy_arg("", "energy",
      "Print energy (in kWh) used by attached devices since the last reset",
      cmd, false);
  ValueArg<float> line_voltage_arg("", "line_voltage",
      "Specify the line voltage used in energy estimation",
      false, kDefaultLineVoltage, "volts", cmd);
  SwitchArg log_indefinitely_arg("l", "log_indefinitely",
      "Requests stats to be logged indefinitely", cmd, false);
  ValueArg<size_t> log_count_arg("c", "log_count",
      "Requests stats to be logged n times, ignored if log_indefinitely",
      false, 1, "count", cmd);
  ValueArg<useconds_t> interval_us_arg("", "interval",
      "The interval between logs, ignored for just one log",
      false, kDefaultLoggingIntervalUs, "microseconds", cmd);

  // Default outlet state args.
  ValueArg<size_t> outlet_default_enable_arg("", "outlet_default_enable",
      "The index of the outlet to set enabled by default",
      false, 0, "index", cmd);
  ValueArg<size_t> outlet_default_disable_arg("", "outlet_default_disable",
      "The index of the outlet to set disabled by default",
      false, 0, "index", cmd);

  // Outlet state args.
  ValueArg<size_t> outlet_enable_arg("", "outlet_enable",
      "The index of the outlet to enable", false, 0, "index", cmd);
  ValueArg<size_t> outlet_disable_arg("", "outlet_disable",
      "The index of the outlet to disable", false, 0, "index", cmd);

  // Parse arguments.
  cmd.parse(argc, argv);

  if (outlet_enable_arg.isSet() && outlet_disable_arg.isSet()) {
    fprintf(stderr, "Error: outlet state must only be manipulated once\n");
    CleanupAndAbort();
  }

  if (outlet_default_enable_arg.isSet() && outlet_default_disable_arg.isSet()) {
    fprintf(stderr, "Error: outlet default state must only be manipulated once\n");
    CleanupAndAbort();
  }

  PowerUsbDevice device;
  if (!device.IsInitialized()) {
    fprintf(stderr, "Error opening the Power USB device: not found\n");
    CleanupAndAbort();
  }

  if (print_device_info_arg.getValue()) {
    PrintDeviceType(device);
  }

  if (reset_charge_accumulator_arg.getValue()) {
    ResetChargeAccumulator(device);
  }

  if (outlet_default_enable_arg.isSet()) {
    size_t outlet_index = outlet_enable_arg.getValue();
    SetDefaultSocketState(device, outlet_index, SocketState::On);
  }

  if (outlet_default_disable_arg.isSet()) {
    size_t outlet_index = outlet_disable_arg.getValue();
    SetDefaultSocketState(device, outlet_index, SocketState::Off);
  }

  if (outlet_enable_arg.isSet()) {
    size_t outlet_index = outlet_enable_arg.getValue();
    SetSocketState(device, outlet_index, SocketState::On);
  }

  if (outlet_disable_arg.isSet()) {
    size_t outlet_index = outlet_disable_arg.getValue();
    SetSocketState(device, outlet_index, SocketState::Off);
  }

  // Build a logging config and log device information as requested.
  LoggingConfig logging_config;
  logging_config.log_current = log_current_arg.getValue();
  logging_config.log_power = log_power_arg.getValue();
  logging_config.log_energy = log_energy_arg.isSet();
  logging_config.line_voltage = line_voltage_arg.getValue();
  logging_config.log_indefinitely = log_indefinitely_arg.getValue();
  logging_config.log_count = log_count_arg.getValue();
  logging_config.interval_us = interval_us_arg.getValue();
  if (logging_config.LogsEnabled()) {
    LogStats(device, logging_config);
  }

  // Cleanup after the hidapi library and exit cleanly.
  hid_exit();
  return 0;
}
