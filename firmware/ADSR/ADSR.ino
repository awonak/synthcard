// GPIO Pin mapping.
#define S1 A0  // Slider Potentiometer 1
#define S2 A1  // Slider Potentiometer 2
#define S3 A2  // Slider Potentiometer 3
#define S4 A3  // Slider Potentiometer 4
#define SLIDER_COUNT 4

#define LED1 9  // Slider LED 1
#define LED2 6  // Slider LED 2
#define LED3 5  // Slider LED 3
#define LED4 3  // Slider LED 4

#define ANALOG_IN A7   // Analog Input
#define DIGITAL_IN 11  // Trigger Input to advance step
#define CV_OUT 10      // CV Output for current step

// Slider & LED pin arrays to support looping over led sliders.
const uint8_t SLIDER_PIN[SLIDER_COUNT] = {S1, S2, S3, S4};
const uint8_t LED_PIN[SLIDER_COUNT] = {LED1, LED2, LED3, LED4};

// Conversion calculations: Arduino 5v pin will only output 4.6v when powered by
// USB.
const float VREF = 4.6 / 1023.0;

// Global state variables.
int ain = 0;  // Raw cv read from pot for current step
int old_ain = 0;
uint8_t din = 0;  // Digital input read value.
uint8_t old_din = 0;
int slider[SLIDER_COUNT];  // Hold the current slider value in an array of
                           // sliders.
float R;  // R holds the linear to exponential conversion factor value.

// Script state variables.
const byte top = 255;  // Envelope state max value
byte val = 0;          // Envelope state value
int time = 0;          // Envelope delay time between incremental change
int sustain = 0;       // Sustain value;

enum Stage { ATTACK, DECAY, SUSTAIN, RELEASE };
Stage stage = ATTACK;

void setup() {
  pinMode(ANALOG_IN, INPUT);
  pinMode(DIGITAL_IN, INPUT);
  pinMode(CV_OUT, OUTPUT);
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  // Register setting for high frequency PWM.
  TCCR1A = 0b00100001;
  TCCR1B = 0b00100001;

  // Calculate the R variable (only needs to be done once at setup).
  R = (1023 * log10(2)) / (log10(255));

  delay(100);
}

void loop() {
  // Check gate input.
  old_din = din;
  din = digitalRead(DIGITAL_IN);

  // Detect if gate has just opened and begin envelope attack.
  if (old_din == 0 && din == 1) {
    stage = ATTACK;
    val = 0;
  }

  // Detect if gate has just closed and begin the release envelope.
  if (old_din == 1 && din == 0) {
    stage = RELEASE;
  }

  // Advance the cv value for the current envelope stage according to the
  // related stage knob position.
  switch (stage) {
    case ATTACK:
      // At minimum attack levels, traverse the curve at a faster rate than
      // default.
      val += analogRead(S1) == 0 ? min(top - val, 10) : 1;

      if (val >= top) {
        stage = DECAY;
      }
      break;

    case DECAY:
      // Decrease the envelope value if it's still falling.
      if (val > 0) {
        // At minimum release levels, traverse the curve at a faster rate than
        // default.
        val -= analogRead(S2) == 0 ? min(val, 10) : 1;
      }

      // Check if the falling decay envelope has reached sustain level.
      sustain = min(map(analogRead(S3), 0, 1023, 0, top), val);
      if (val <= sustain) {
        stage = SUSTAIN;
      }
      break;

    case SUSTAIN:
      val = map(analogRead(S3), 0, 1023, 0, top);
      break;

    case RELEASE:
      // Decrease the envelope value if it's still falling.
      if (val > 0) {
        // At minimum release levels, traverse the curve at a faster rate than
        // default.
        val -= analogRead(S4) == 0 ? min(val, 10) : 1;
      }
      break;
  }

  // Attack / Decay / Release incremental delay time.
  switch (stage) {
    case ATTACK:
      time = analogRead(S1);
      break;
    case DECAY:
      time = analogRead(S2);
      break;
    case SUSTAIN:
      time = 1000;  // Default time for Sustain.
      break;
    case RELEASE:
      time = analogRead(S4);
      break;
  }

  // Attack / Decay / Release stage LED indicator.
  for (int i = 0; i < SLIDER_COUNT; i++) {
    analogWrite(LED_PIN[i], 0);
  }
  switch (stage) {
    case ATTACK:
      analogWrite(LED_PIN[0], val);
      break;
    case DECAY:
      analogWrite(LED_PIN[1], val);
      break;
    case SUSTAIN:
      analogWrite(LED_PIN[2], val);
      break;
    case RELEASE:
      analogWrite(LED_PIN[3], val);
      break;
  }

  // Write envelope CV output
  analogWrite(CV_OUT, val);

  // Short sleep duration before advancing to the next step in the curve table.
  delayMicroseconds(time * 10);
}

// Convert a linear value to its exponential compliment for led visual
// correction.
int brightness(int v) { return (v > 32) ? pow(2, (v / R)) : 0; }
