# DMT Biofuel Engine PCB Test Script

This is a test script to test the Control System electronics from the DMT Biofuel Engine project. The point of this experiment is to ensure the transistors open and close as expected, by pulsing the transistors open and closed repeatedly. 

In addition to checking whether the transistors open and close, the speed can be changed to see at what point the transistors switching speed becomes too slow to control the engine effectively.

For more information about the testing process, read the __Test Specification__.

## Pre-requisites

In order to use this software, you must have:

- Arduino Micro.
- Arduino IDE.
- GitHub Desktop (optional).

To install the Arduino IDE, follow this [link](https://www.arduino.cc/en/software). The IDE provides an easy way to verify and upload the program to the board, as well as a serial monitor to communicate with the Arduino. 

GitHub Desktop also provides a quick way of downloading the repository, which can be installed from [here](https://desktop.github.com/).

## Installation & Board Uploading

Download this repository by clicking the __code__ button on the repository homepage and selecting either __Open with GitHub Desktop__ or __Download ZIP__.

Go to where the folder has been downloaded, and navigate to `pcb_test/` and open `pcb_test.ino` in the Arduino IDE.

Click on the __Tools__ dropdown menu and select the board type. for the PCB test this will be __Arduino Micro__. Once the Arduino has been plugged into your computer, check the __Port__ from the same menu. This should be done automatically, however if no board is found, you can select the Micro from the list of available devices.

Click __Upload__ to install the program on the Arduino. If the build fails please [email me](mailto:louis.manestar18@imperial.ac.uk).


## Usage

When the program has been uploaded, Click on the __Serial Monitor__ from the __Tools__ dropdown menu. This will allow you to communicate and instruct the Arduino to open/close different circuits.

__Ensure that the Serial messages are sent with a newline at the end.__ This is how the Arduino knows a message is available. This can be selected from the dropdown menu at the bottom of the serial monitor.

The instructions passed to the Arduino have a bash-style syntax:

```bash
command [--TARGET target_name | --SPEED speed_value]
```

There are four commands available:

- `START`, which starts the pulses.
- `STOP`, which stops the pulses.
- `SET`, which allows you to set the target circuit to pulse and/or the speed at which it is pulsing.
- `GET`, which allows you to get the reading of a particular circuit.

`--TARGET` and `--SPEED` are optional arguments where you can include the target circuit and the speed in the instruction. 

Note: All target names must be given in __uppercase__. You can select:

- `THERMISTOR` to get the value of temperature reading of the internal thermistor.
- `CRANKSHAFT` to get the pin state of the Ignition Pulse Generator (IPG).
- `CAMSHAFT` to get the pin state of the CAM Pulse Generator (CPG).
- `INJECTOR [1-4]`.
- `COIL [1-4]`.

The speed value is given in RPM, and will cause the circuit to pulse at the same rate as if the engine had that RPM.

### Example Instructions

```bash
SET --TARGET INJECTOR 3 --SPEED 2300
```

This will cause the Arduino to pulse the injector in cylinder 3 at the same rate as if the engine was running at 2300 RPM.

```bash
GET --TARGET THERMISTOR
```

This will return the temperature reading given by the internal thermistor.

```bash
START
```

Start the pulses. This will only have an effect if both the target and speed have been defined beforehand.

```bash
STOP
```

Stop the pulses. You should call this before unplugging the Arduino to be safe.

```bash
SET --SPEED 4000
```

If the target has already been set, you can change the speed by omitting the `--TARGET` flag. The reverse is also possible if you wish to change the target, but want to keep the speed the same.

## Contact

If you need any information or help, please email me at [louis.manestar18@imperial.ac.uk](mailto:louis.manestar18@imperial.ac.uk).
