#include <cstdio>

#include <unistd.h>

#include "power_usb_device.h"

using namespace pwrusbctl;

int main(int argc, char **argv) {
  PowerUsbDevice device;
  if (!device.IsInitialized()) {
    fprintf(stderr, "Error opening the Power USB device: not found\n");
    return -1;
  }

  fprintf(stderr, "Found device type: %s\n", device.GetDeviceType());

  device.ResetChargeAccumulator();

  while(1) {
    int16_t current;
    device.GetInstantaneousCurrent(&current);
    fprintf(stderr, "Current %dmA\n", current);

    int32_t charge;
    device.GetAccumulatedCharge(&charge);
    fprintf(stderr, "Charge %dmA\n", charge);

    int line_voltage = 110;
    float energy = PowerUsbDevice::ConvertChargeToKilowattHours(charge, line_voltage);
    fprintf(stderr, "Estimated energy: %fkWh\n", energy);

    usleep(10000000);
  }

  // Cleanup after the hidapi library.
  hid_exit();

  // Exit cleanly.
  return 0;
}
