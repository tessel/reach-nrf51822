#Reach
This document outlines the progress on the Gossip-over-BLE implementation.

##Gossip BLE Profile
A preliminary draft of the GATT Profile for communicating with Gossip over BLE is in the `profiles.json` file in this repo. It currently defines 3 proprietary services: `"Gossip IO Protocol Service"`, `"Reach Configuration Protocol Service"`, and `"Reach OTA Update Protocol Service"`. The former has 2 characteristics used separately for upstream and downstream message passing, mimicking a duplex stream. The latter two services are in place as an example for UUID assignment conventions should more services be deemed necessary.

##Client
The current implementation of the Reach client is meant to be run on a Tessel and as such makes use of it's hardware interface. The source for the client can be found in the `client.js` file. The majority of this code is dedicated to establishing a connection with a host over BLE. This file also contains a `Gossip` object used for parsing incoming packets, executing the appropriate function, and sending the appropriate response. Current implementation is a port of the Rust parser to JavaScript, and only has the SPI functionality.

##Host
The current host implementation uses `noble` to interface with the BLE adapter on a computer, source located in the `host.js` file. Again, most of this file is dedicated to establishing a BLE connection with a Reach client. Lacks most Gossip logic.

##Road map
Priorities
1. Get firmware build & deploy set up for nrf51822 w/open source tools
* Port current client code to C using nrf51822 SDK
  * Include UART/I2C/etc. interfaces
* Expand host capabilities to include Gossip logic
* Neatly package demo closely resembling final product

##Running Current Code
`npm install` to install necessary dependencies.

`tessel run client.js` to run the client on a Tessel. Must have BLE module on port A.

`node host.js` to run the host. Must be run on a computer with BLE hardware.
