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
const byte max_val = 255;  // CV step max value
const byte max_step =
    4;  // Max steps in the CV sequence (corresponding to number of knobs)
byte val = 0;   // Current CV value
byte step = 0;  // Current CV step (0,1,2,3)
int read = 0;   // Raw cv read from pot for current step

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
  old_din = din;
  din = digitalRead(DIGITAL_IN);

  // Detect if new trigger received and advance step.
  if (old_din == 0 && din == 1) {
    // Turn off current step LED.
    analogWrite(LED_PIN[step], LOW);

    // Advance step count and turn on LED.
    step = (step + 1) % max_step;
    digitalWrite(LED_PIN[step], HIGH);
  }

  // Write current step CV output.
  val = analogRead(SLIDER_PIN[step]) >> 2;
  analogWrite(CV_OUT, val);
}