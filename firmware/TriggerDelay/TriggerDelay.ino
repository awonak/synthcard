// GPIO Pin mapping.
#define P1 A0  // Trigger Delay
#define P2 A1  // Attack Slope
#define P3 A2  // Gate Duration
#define P4 A3  // Release Slope
#define SLIDER_COUNT 4

#define LED1 9  // Slider LED 1
#define LED2 6  // Slider LED 2
#define LED3 5  // Slider LED 3
#define LED4 3  // Slider LED 4

#define ANALOG_IN 21   // Analog Input
#define DIGITAL_IN 11  // Trigger Input to advance step
#define CV_OUT 10      // CV Output for current step

// Slider & LED pin arrays to support looping over led sliders.
const uint8_t SLIDER_PIN[SLIDER_COUNT] = {P1, P2, P3, P4};
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
float R;                   // R holds the linear to exponential conversion factor value.

enum Stage {
    DELAY,
    ATTACK,
    GATE,
    RELEASE,
    WAIT,
};
Stage stage = WAIT;

// Script state.
unsigned long trig_start;
int trig_delay;
int trig_duration;
int attack_slope;
int release_slope;
int attack_count;
int release_count;
int i;
int output;

void setup() {
    pinMode(ANALOG_IN, INPUT);
    pinMode(DIGITAL_IN, INPUT);
    pinMode(CV_OUT, OUTPUT);
    pinMode(P1, INPUT);
    pinMode(P2, INPUT);
    pinMode(P3, INPUT);
    pinMode(P4, INPUT);
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);

    // Register setting for high frequency PWM.
    TCCR1A = 0b00100001;
    TCCR1B = 0b00100001;

    // Calculate the linear to exponential R value (only needs to be done once at setup).
    R = (1023 * log10(2)) / (log10(255));
}

void loop() {
    // Read all inputs.
    old_ain = ain;
    old_din = din;
    ain = analogRead(ANALOG_IN);
    din = digitalRead(DIGITAL_IN);
    for (int i = 0; i < SLIDER_COUNT; i++) {
        slider[i] = analogRead(SLIDER_PIN[i]);
    }

    // Set LED Brightness.
    for (int i = 0; i < SLIDER_COUNT; i++) {
        analogWrite(LED_PIN[i], brightness(slider[i]));
    }

    // Detect if a new trigger has been received.
    if (old_din == 0 && din == 1) {
        // Read the trigger delay and duration.
        trig_delay = map(analogRead(P1), 0, 1023, 0, 2000);
        trig_duration = map(analogRead(P3), 0, 1023, 20, 2000);
        trig_start = millis();

        // Calculate the exponential slope value.
        attack_count = analogRead(P2);
        release_count = analogRead(P4);
        attack_slope = (attack_count * log10(2)) / log10(255);
        release_slope = (release_count * log10(2)) / log10(255);

        stage = DELAY;
        output = 0;
        i = 1;
    }

    switch (stage) {
        case DELAY:
            if (millis() > trig_start + trig_delay) {
                stage = ATTACK;
            }
            break;
        case ATTACK:
            if (i >= attack_count) {
                stage = GATE;
                trig_start = millis();
                i = release_count;
                output = 255;
            } else {
                output = min(pow(2, (float(i) / float(attack_slope))), 255);
                i = min(i++, attack_count);
            }
            break;
        case GATE:
            if (millis() > trig_start + trig_duration) {
                stage = RELEASE;
            }
            break;
        case RELEASE:
            if (i == 0) {
                stage = WAIT;
                output = 0;
            } else {
                output = min(pow(2, float(i) / float(release_slope)), 255);
                if (i > 0) i--;
            }
            break;
        case WAIT:
            break;
    }

    // Set output voltage.
    analogWrite(CV_OUT, output);
}

// Convert a linear value to its exponential compliment for led visual correction.
int brightness(int v) { return (v > 32) ? pow(2, (v / R)) : 0; }
