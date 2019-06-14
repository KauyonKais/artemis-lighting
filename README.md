# Lighting System Implementation for [Artemis Spaceship Bridge Simulator](https://artemisspaceshipbridge.com)
Lighting system to be used with the Artemis Spaceship Bridge Simulator

## Requirements
### Hardware
- Arduino Duemilanove
- LED strips (currently WS2811 for colour and single colour PWM driven for light)
### Software
- XML file for DMX codes
- Artemis Spaceship Bridge Simulator (duh)

## Setup

Connect WS2811 strips to pins 6 and 7 for right and left side respectively, hang them as ceiling lights. Use a Duemilanove, since most modern Arduinos won't be recognized as DMX device by Artemis.
