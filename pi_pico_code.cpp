#define ADC_RANGE 1024

// Wooting Lekker switches have 4mm total travel;
// 0.1mm highest sensitivty = 40 max sensitivity levels.
#define LEVELS 40

// The travel distance at which the key should register 
// as being pressed (in mm, measured from top of travel)
#define ACTUATION_POINT 1

// Enable/disable rapid trigger
#define RAPID_TRIGGER false

int depression;

void setup() {
  Serial1.begin(115200);
  depression = ACTUATION_POINT / (LEVELS * 0.1) *  ADC_RANGE;
}

void loop() {
  int value = analogRead(27); // Value read by ADC

  if (RAPID_TRIGGER) {
    Serial1.println("bocchi raiden");
  } else {
    Serial1.println(false_rapid_trigger(value));
  }

  delay(1); // this speeds up the simulation
}

int true_rapid_trigger(int value) {

}

int false_rapid_trigger(int value) {
  return value > depression;
}
