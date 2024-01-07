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
uint8_t old_clk = 0;
int slider[SLIDER_COUNT];  // Hold the current slider value in an array of
                           // sliders.
float R;  // R holds the linear to exponential conversion factor value.

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
  // Serial.begin(115200);
}

void loop() {
  // Read all inputs.
  ain = analogRead(ANALOG_IN);
  din = digitalRead(DIGITAL_IN);
  for (int i = 0; i < SLIDER_COUNT; i++) {
    slider[i] = analogRead(SLIDER_PIN[i]);
  }

  // Process state and write outputs.
  int out = ain >> 2;  // Bit shift 10 bit input to 8 bit output.
  analogWrite(CV_OUT, out);
  digitalWrite(LED_BUILTIN, din);  // Echo digital input on Arduino LED.
  for (int i = 0; i < SLIDER_COUNT; i++) {
    analogWrite(LED_PIN[i], brightness(slider[i]));
  }

  // Log input/output values.
  Serial.println("AIN: " + String(ain) + "\tDIN: " + String(din) +
                 "\tS1: " + String(slider[0]) + "\tS2: " + String(slider[1]) +
                 "\tS3: " + String(slider[2]) + "\tS4: " + String(slider[3]) +
                 "\tOUT: " + String(out));
}

// Convert a linear value to its exponential compliment for led visual correction.
int brightness(int v) { return (v > 32) ? pow(2, (v / R)) : 0; }

// Convert analog input read value to voltage value.
float voltage(int val) { return ain * VREF; }