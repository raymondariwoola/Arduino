# Evil Portal

Evil Portal is an Arduino project designed to create a captive portal using an ESP32. The portal can capture user credentials and other information through a web interface.

## Features

- Captive portal to intercept and capture user credentials.
- Configurable HTML content for the portal page.
- Ability to set the Access Point (AP) name.
- LED indicators for different states (waiting, good, bad).

## Hardware Requirements

- ESP32 board
- LEDs connected to pins 4, 5, and 6 for blue, green, and red indicators respectively.

## Software Requirements

- Arduino IDE
- ESPAsyncWebServer library
- AsyncTCP library
- DNSServer library

## Setup Instructions

1. **Install Libraries**: Ensure you have the required libraries installed in your Arduino IDE.
   - ESPAsyncWebServer
   - AsyncTCP
   - DNSServer

2. **Connect LEDs**: Connect LEDs to the following pins on your ESP32:
   - Blue LED to pin 4
   - Green LED to pin 5
   - Red LED to pin 6

3. **Upload Code**: Open `evilportal.ino` in the Arduino IDE and upload it to your ESP32 board.

4. **Serial Monitor**: Open the Serial Monitor at a baud rate of 115200 to view debug messages and interact with the device.

## Usage

1. **Initial Configuration**: Upon startup, the device will wait for initial configuration commands via the Serial Monitor.
   - `sethtml=<HTML_CONTENT>`: Set the HTML content for the portal page.
   - `setap=<AP_NAME>`: Set the Access Point name.
   - `reset`: Reset the device.

2. **Start Portal**: Once configured, the device will start the Access Point and serve the portal page.

3. **Capturing Data**: The portal page will capture user credentials and other information, which will be displayed in the Serial Monitor.

## Commands

- **sethtml=<HTML_CONTENT>**: Update the HTML content of the portal page.
- **setap=<AP_NAME>**: Update the Access Point name.
- **reset**: Reset the device.
- **start**: Start the portal.

## LED Indicators

- **Blue LED**: Waiting for configuration.
- **Green LED**: Portal is running and ready.
- **Red LED**: Error state.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please open an issue or submit a pull request for any improvements or bug fixes.

## Disclaimer

This project is for educational purposes only. Use it responsibly and do not use it for any malicious activities.