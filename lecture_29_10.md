# Wireless Communication


## History

Started:
- 1873
- Maxwell equations

1980 - First commercial network cellphone

1990 - GSM 

2000 - 3G, bluetooth, WiFi

2010 - 4G/LTE, LTE-A

2020 - 5G


## Why wireless?

- Freedom from wires
- Global coverage
- Stay connected
- Flexibility (can be connected to several devices simultaneously)

## What is wireless?

Transmitting data using electromagnetic waves

Electro waves:
- Speed of light


## Wireless Link Characteristics
- Decreased signal strength
- Interference from other sources
- Multipath propagation

These characteristics make wireless communication much more difficult.

## Modulation

### Analogue modulation

- AM
- FM

### Digital modulation
- ASK: Amplitude shift keyring
- FSK: Frequency shift keyring
- PSK: Phase shift keyring

## Multiplexing - Multiple access

Methods
- TDMA (time slotting, simple) 1G 2G
- FDMA (frequency slotting, complicated) 
- CDMA (unique code, code set divisions) 3G
- OFDMA 5G

**Orthogonality Principle**
*efficiency*

## Wireless impact on higher layers

Logically it should be minimal. But because of packet loss TCP has issues regarding congestion windows.

- delay
- limited bandwidth

## Wireless LAN architecture

- AP (center point)
- BBS

### IEEE 802.11 Wireless LAN

802.11x versioning

## Wireless network characteristics

- Hidden terminal problem
- Exposed terminal problem

## Carrier Sense Multiple Access (CSMA)
- Listen before talk (LBT)


### Avoiding collision

Allow sender to reserve channel

- senders send (RTS) packets to use base station using CSMA
- base station broadcasts a clear to send message


## Advanced

### MIMO
Exploits the space dimension to improve wireless systems capacity
- Multiple antennas


##### Beamforming
*directional antennas*


### MU-MIMO

MIMO + Beamforming

- Not free, costs in processing power/time

### mmWave

100-1000 times faster than existing tech

- Much higher frequencies
- Drawback: shorter range

Solution: Beamforming

Problem: Where should the beam point?
*What if the direct path is blocked?*

**Solution**
- Sector Level Sweep
- Beam Refinement Process

## Bluetooth

Wireless Personal Area Networks (WPAN)

Design goal
- Cable replacement
- Low cost
- Low power
- Small size
- For mobile devices

Standard: IEEE 802.15.1

### Bluetooth

Short distance networking

Developed in 1994 by Ericsson


### How it works?

1 master, up-to 7 slaves

devices can switch roles

called piconets

two or more piconets for a scatternet

#### Pairing

A bonding procedure between two devices

#### Communication
devices automatically detect and connect to one another

Hopping between frequency during the session

79 different channels (frequencies)

