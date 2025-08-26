/* Teensy 4.1 — LED blink + PWM x1_000_000 + JSON status
 *  - LED: pin 13 (built-in LED), blinking frequency set by user (default 1 Hz)
 *  - PWM: pin 2 (hardware PWM), frequency = LED_HZ * 1_000_000 (e.g., 2 -> 2 MHz), 50% duty
 *  - Serial: accepts a single-line number (Hz for LED) and periodic JSON status
 */

const int LED_PIN = 13;      // built-in LED on Teensy 4.1
const int PWM_PIN = 2;       // PWM pin (Teensy 4.x: e.g., 2,3,4,5,... support PWM)
const float DEFAULT_LED_HZ = 1.0f;
const float MULTIPLIER = 1'000'000.0f;  // x1e6
const float MAX_PWM_HZ = 20'000'000.0f; // safety limit (20 MHz); adjust as needed
const float MIN_LED_HZ = 0.05f;         // 0.05 Hz ~ 20 s per full cycle - avoids division by zero

// Non-blocking blink (uses micros)
uint32_t lastToggleUs = 0;
bool ledState = false;
float ledHz = DEFAULT_LED_HZ;

// Serial line buffer
char lineBuf[32];
size_t lineLen = 0;

// Timers for JSON status
uint32_t lastStatusMs = 0;
const uint32_t STATUS_EVERY_MS = 1000;

// Helper: set PWM to 50% duty for a given frequency
void setPwmFrequency(float hz) {
  if (hz < 1.0f) hz = 1.0f;               // minimum 1 Hz for PWM
  if (hz > MAX_PWM_HZ) hz = MAX_PWM_HZ;   // upper limit
  analogWriteFrequency(PWM_PIN, hz);
  analogWrite(PWM_PIN, 128);              // 50% duty at 8-bit resolution
}

void applyNewFrequency(float newLedHz) {
  // sanity check & range
  if (newLedHz < MIN_LED_HZ) newLedHz = MIN_LED_HZ;
  ledHz = newLedHz;

  float pwmHz = ledHz * MULTIPLIER;
  if (pwmHz > MAX_PWM_HZ) pwmHz = MAX_PWM_HZ;

  setPwmFrequency(pwmHz);

  // quick feedback after change
  Serial.print(F("{\"event\":\"update\",\"led_hz\":"));
  Serial.print(ledHz, 6);
  Serial.print(F(",\"pwm_pin\":"));
  Serial.print(PWM_PIN);
  Serial.print(F(",\"pwm_hz\":"));
  Serial.print(pwmHz, 0);
  Serial.println(F("}"));
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(PWM_PIN, OUTPUT);
  analogWriteResolution(8);  // 0..255
  setPwmFrequency(DEFAULT_LED_HZ * MULTIPLIER);

  Serial.begin(115200);
  while (!Serial && millis() < 2000) { /* wait briefly for USB serial */ }

  Serial.println(F("{\"event\":\"boot\",\"msg\":\"Teensy4.1 ready\",\"led_hz\":1.0,\"pwm_pin\":2,\"pwm_hz\":1000000}"));
}

void loop() {
  // --- 1) LED blinking (no delay) ---
  // half-period in microseconds = 0.5 / ledHz seconds
  float halfPeriodSec = 0.5f / ledHz;
  uint32_t halfPeriodUs = (uint32_t)(halfPeriodSec * 1e6f);
  uint32_t nowUs = micros();
  if ((uint32_t)(nowUs - lastToggleUs) >= halfPeriodUs) {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    lastToggleUs = nowUs;
  }

  // --- 2) Read from Serial: single line as LED frequency (Hz) ---
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\r') continue;           // skip CR
    if (c == '\n') {                   // end of line => parse
      lineBuf[lineLen] = '\0';
      if (lineLen > 0) {
        float f = atof(lineBuf);
        if (f > 0.0f) {
          applyNewFrequency(f);
        } else {
          Serial.print(F("{\"event\":\"error\",\"reason\":\"non_positive_input\",\"raw\":\""));
          Serial.print(lineBuf);
          Serial.println(F("\"}"));
        }
      }
      lineLen = 0;                     // reset buffer
    } else if (lineLen + 1 < sizeof(lineBuf)) {
      lineBuf[lineLen++] = c;          // accumulate
    } else {
      // overflow - reset and report error
      lineLen = 0;
      Serial.println(F("{\"event\":\"error\",\"reason\":\"line_overflow\"}"));
    }
  }

  // --- 3) Periodic JSON status ---
  uint32_t nowMs = millis();
  if (nowMs - lastStatusMs >= STATUS_EVERY_MS) {
    float pwmHz = ledHz * MULTIPLIER;
    if (pwmHz > MAX_PWM_HZ) pwmHz = MAX_PWM_HZ;

    Serial.print(F("{\"event\":\"status\",\"led_hz\":"));
    Serial.print(ledHz, 6);
    Serial.print(F(",\"pwm_pin\":"));
    Serial.print(PWM_PIN);
    Serial.print(F(",\"pwm_hz\":"));
    Serial.print(pwmHz, 0);
    Serial.println(F("}"));

    lastStatusMs = nowMs;
  }
}
