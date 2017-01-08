# PwrUsbCtl

A tool for interacting with the Power USB line of USB-connected power strips.

More information about this hardware is available here: http://www.pwrusb.com/

## Build Instructions

This codebase has two dependencies: HIDAPI for communicating via USB and TCLAP
for command-line argument parsing.

### Linux (Arch)

The instructions should be similar for your Linux distribution of choice, just
substitute your package manager and tweak the package names if necessary.

#### Install dependencies

    sudo pacman -S tclap
    sudo pacman -S hidapi

#### Build

    make

### Mac OS

These instructions require Homebrew to be installed. If you use MacPorts, the
instructions will be different.

#### Install dependencies

    brew install tclap
    brew install hidapi

#### Build

    make

## Usage Instructions

Run the binary with ``--help`` to obtain usage information.

    Brief USAGE:
       ./pwrusbctl  [--outlet_disable <index>] [--outlet_enable <index>]
                    [--outlet_default_disable <index>] [--outlet_default_enable
                    <index>] [--interval <microseconds>] [-c <count>] [-l]
                    [--line_voltage <volts>] [--energy] [--current]
                    [--reset_charge_accumulator] [--device_info] [--]
                    [--version] [-h]

    For complete USAGE and HELP type:
       ./pwrusbctl --help
