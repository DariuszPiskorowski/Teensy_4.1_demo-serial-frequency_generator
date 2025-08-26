# Teensy 4.1 Serial Frequency Generator Demo

A simple demo for Teensy 4.1 that demonstrates:
- User-controlled LED blinking
- High-frequency hardware PWM output
- Serial communication for real-time control
- Periodic JSON status reporting

---

## Features

- **LED Blink Control:** Blinks the built-in LED (pin 13) at a frequency set by the user (default: 1 Hz).
- **User-Defined Blink Frequency:** Enter a number via serial to update the LED blink frequency.
- **High-Frequency PWM Output:** Pin 2 outputs a digital signal with frequency = LED_HZ × 1,000,000 (e.g., 2 Hz LED → 2 MHz PWM, 50% duty cycle).
- **JSON Status Reporting:** Sends structured JSON messages to the serial output with both the LED frequency and the PWM output frequency.
- **Event Feedback:** On each update, an additional event is sent: `"event": "update"`.

---

## Getting Started

### Requirements
- Teensy 4.1 board
- Arduino IDE with Teensyduino or PlatformIO
- USB cable for programming and serial communication

### Installation
1. Clone or download this repository.
2. Open the `demo.ino` file in Arduino IDE or copy its contents into a new sketch.
3. Select the correct board (Teensy 4.1) and port.
4. Upload the code to your Teensy.

### Serial Communication
- Baud rate: **115200**
- Send a number (e.g., `2.5`) followed by Enter to set the LED blink frequency (in Hz).
- The device will adjust the PWM output and reply with a JSON status.

---

## Example Serial Session

```
> 2
{"event":"update","led_hz":2.000000,"pwm_pin":2,"pwm_hz":2000000}

{"event":"status","led_hz":2.000000,"pwm_pin":2,"pwm_hz":2000000}
```

If you send an invalid or non-positive value:
```
> -1
{"event":"error","reason":"non_positive_input","raw":"-1"}
```

---

## Practical Notes

- Teensy 4.1 supports very high PWM frequencies, but at MHz ranges the duty cycle resolution and jitter depend on internal clocks and timers.
- For clean 2–10 MHz signals, use pins with native FlexPWM support and avoid conflicts with other timing functions.
- The maximum frequency limit is currently 20 MHz (defined by `MAX_PWM_HZ`). You can adjust this constant if needed.
- To use another PWM pin, modify the `PWM_PIN` definition to a Teensy 4.1 pin that supports PWM.

---

## File Overview

- `demo.ino` — Main source file with all logic and comments in English.
- `README.md` — This documentation.

---

## License

MIT License. See [LICENSE](LICENSE) for details.

---

## Author

[Dariusz Piskorowski](https://github.com/DariuszPiskorowski)