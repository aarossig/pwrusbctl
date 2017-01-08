# PwrUsbCtl

A tool for interacting with the Power USB line of USB-connected power strips.

More information about this hardware is available here: http://www.pwrusb.com/

This project was not created in affiliation with the vendor so do not contact
them for support.

## Usage Instructions

Run the binary with ``--help`` to obtain usage information.

    USAGE: 
    
       ./pwrusbctl  [--outlet_disable <index>] [--outlet_enable <index>]
                    [--outlet_default_disable <index>] [--outlet_default_enable
                    <index>] [--interval <microseconds>] [-c <count>] [-l]
                    [--line_voltage <volts>] [--energy] [--current]
                    [--reset_charge_accumulator] [--device_info] [--]
                    [--version] [-h]
    
    
    Where: 
    
       --outlet_disable <index>
         The index of the outlet to disable
    
       --outlet_enable <index>
         The index of the outlet to enable
    
       --outlet_default_disable <index>
         The index of the outlet to set disabled by default
    
       --outlet_default_enable <index>
         The index of the outlet to set enabled by default
    
       --interval <microseconds>
         The interval between logs, ignored for just one log
    
       -c <count>,  --log_count <count>
         Requests stats to be logged n times, ignored if log_indefinitely
    
       -l,  --log_indefinitely
         Requests stats to be logged indefinitely
    
       --line_voltage <volts>
         Specify the line voltage used in energy estimation
    
       --energy
         Print energy (in kWh) used by attached devices since the last reset
    
       --current
         Print the current used by attached devices
    
       --reset_charge_accumulator
         Resets the charge accumulator
    
       --device_info
         Print device information
    
       --,  --ignore_rest
         Ignores the rest of the labeled arguments following this flag.
    
       --version
         Displays version information and exits.
    
       -h,  --help
         Displays usage information and exits.
    
    
       a tool for interacting with PowerUSB USB-controlled power strip

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
