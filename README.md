#Reach
This document outlines the progress on the Gossip-over-BLE implementation.

##Flashing the softdevice
First grab the s110 softdevice folder and put it in the top level directory of this repo.
Use a JLink-speaking programmer and connect one of the devboards.
Run `make flash-softdevice` to get the softdevice onto the nrf51822.

##Building the project

###Requirements

**Install JLink GDB server.** scroll down to "J-Link software & documentation pack for MAC") https://www.segger.com/jlink-software.html

**Install Noric packages.** Try the following handy script.

```
mkdir -p nRF51_SDK_7.1.0
curl http://developer.nordicsemi.com/nRF51_SDK/nRF51_SDK_7.1.0_372d17a.zip | tar xvf - -C nRF51_SDK_7.1.0

mkdir -p s110_nrf51822_7.1.0
curl http://www.nordicsemi.com/eng/nordic/download_resource/30082/12/50064729 | tar xvf - -C s110_nrf51822_7.1.0
```

**Install gcc-arm.** `brew tap tessel/tools` and `brew install gcc-arm`. (If your gcc-arm location is different, set `make GNU_INSTALL_ROOT=/usr/local/wherever`.)

###Running code

Use `make` to compile and link the application binary.

Use `make flash-softdevice` to load the softdevice on the nrf51822.

Use `make flash` to load the application on the nrf51822 after the softdevice.

##Gossip BLE Profile
A preliminary draft of the GATT Profile for communicating with Gossip over BLE is in the `profiles.json` file in this repo. It currently defines 3 proprietary services: `"Gossip IO Protocol Service"`, `"Reach Configuration Protocol Service"`, and `"Reach OTA Update Protocol Service"`. The former has 2 characteristics used separately for upstream and downstream message passing, mimicking a duplex stream. The latter two services are in place as an example for UUID assignment conventions should more services be deemed necessary.

##nrf51822 Client
The nrf51822 client code is mostly contained in `/src/main.c`, and outlines all of the GAP (connection) and GATT (profile) set up.

##Tessel Client
The Tessel client can be found in the `client.js` file. The majority of this code is dedicated to establishing a connection with a host over BLE. This file also contains a `Gossip` object used for parsing incoming packets, executing the appropriate function, and sending the appropriate response. Current implementation is a port of the Rust parser to JavaScript, and only has the SPI functionality.

##Host
The current host implementation uses `noble` to interface with the BLE adapter on a computer, source located in the `host.js` file. Again, most of this file is dedicated to establishing a BLE connection with a Reach client. Lacks most Gossip logic.

##Road map
Priorities

1. ~~Get firmware build & deploy set up for nrf51822~~ w/open source tools
* Port current client code to C using nrf51822 SDK
  * ~~Connection logic~~
  * Gossip logic
  * SPI
  * UART
  * I2C
  * GPIO
* Expand host capabilities to include Gossip logic
* Neatly package demo closely resembling final product

##Running Current Code
`npm install` to install necessary dependencies.

`tessel run client.js` to run the client on a Tessel. Must have BLE module on port A.

`node host.js` to run the host. Must be run on a computer with BLE hardware.
