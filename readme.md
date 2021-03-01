# DMT Biofuel Engine Control System

This is the repository for the software used for the DMT Biofuel Engine Sensors & Control Team. The Sensors & Control team are developing a control system that will allow a motorcycle engine to run on E85 fuel using an Arduino Micro to control the fuel injection and spark timings of the engine.

This is based off camshaft and crankshaft encoders from the engine to determine the precise angle of the shafts. By modelling the engine using 1D analysis, the optimal shaft angles for charging/discharging the ignition coils or opening/closing the fuel injectors can be found. When the shaft positions reach these optimal angles, the program will open or close the appropriate circuit controlling one of the ignition coils or fuel injectors.

## Pre-requisites

You must have an Arduino Micro and `arduino-cli` installed in order to upload the program to the microcontroller.

You can install `arduino-cli` from [here](https://www.arduino.cc/pro/cli). Also make sure you have installed the core libraries, which can be done using the following command:

```bash
arduino-cli core install arduino:samd
```

## Installation

This repository can be downloaded using:

```bash
git clone git@github.com:Loumstar/DMT-Control-System.git
```

Before uploading the program to the Arduino, ensure all the tests have passed to make sure it will function correctly.

To install the program onto the Micro, run the following command:

```bash
arduino-cli upload ...
```

## Testing

Tests will be added shortly.

## Usage

The arduino will start the program when powered on. Therefore, once the control has a power supply the program will run on its own. For the moment, the only way to power-off the Arduino is to remove the power supply.

Control through a computer and serial communication will be added shortly.
