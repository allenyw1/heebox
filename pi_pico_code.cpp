// Wooting Lekker switches have 4mm total travel;
// 0.1mm highest sensitivty = 40 max sensitivity levels.
#define LEVELS 40
#define ADC_RANGE 1024
#define ADC_PIN 27

// USER CONFIGURABLE SETTINGS
#define ACTUATION_POINT 2 // Distance from top for actuation (in mm)
#define RAPID_TRIGGER false // Enable/disable rapid trigger
#define RT_RELEASE 1 // Minimum release distance to reset rapid trigger (in mm)


// GLOBAL VARIABLES
int actuation_level; // Level conversion of actuation distance (mm to levels)
int current_level = 0; // Current switch depression level
int local_max_level = 0; // Used for rapid trigger
int actuated = 0; // 1 means actuated, 0 means not actuated

void setup() {
  Serial1.begin(115200);
  actuation_level = 10 * ACTUATION_POINT;
}

void loop() {
  current_level = analogRead(ADC_PIN) * LEVELS / ADC_RANGE; // Value read by ADC

  if (RAPID_TRIGGER) {
    true_rapid_trigger();
  } else {
    false_rapid_trigger();
  }

  Serial1.print(actuated);
  Serial1.print("    ");
  Serial1.println(current_level);

  delay(1); // this speeds up the simulation
}

int true_rapid_trigger() {
  if (current_level == 0 || current_level + RT_RELEASE * 10 < local_max_level) {
    local_max_level = current_level;
    actuated = 0;
  }
  if (current_level > local_max_level) {
    local_max_level = current_level;
    actuated = 1;
  }
}

int false_rapid_trigger() {
  if (current_level < actuation_level) {
    actuated = 0;
  } else {
    actuated = 1;
  }
}
