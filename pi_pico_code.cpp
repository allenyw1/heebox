// Wooting Lekker switches have 4mm total travel;
// 0.1mm highest sensitivty = 40 max sensitivity levels.
#define LEVELS 40
#define ADC_PIN 27
#define BUTTON_PIN 21
#define DEBOUNCE_TIME 100

// USER CONFIGURABLE SETTINGS
#define ACTUATION_POINT 2 // Distance from top for actuation (in mm)
#define RT_RELEASE 1 // Minimum release distance to reset rapid trigger (in mm)

// GLOBAL VARIABLES
int actuation_level; // Level conversion of actuation distance (mm to levels)
int current_read = 0; // Raw input reading from HE sensor
int current_level = 0; // Current switch depression level
bool rapid_trigger_enable = false;
int local_max_level = 0; // Used for rapid trigger
int actuated = 0; // 1 means actuated, 0 means not actuated

bool calibration_toggle = false;
int calibration_time = 0; // Used for checking button debounce
bool calibration_trigger = false;
int read_max;
int read_min;
int adc_range = 1023;


void setup() {
  Serial1.begin(115200);
  actuation_level = 10 * ACTUATION_POINT;
  
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), calibration_isr, FALLING);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  current_read = analogRead(ADC_PIN);
  current_level = (current_read - read_min) * LEVELS / adc_range;

  // Handle calibration mode toggle
  if (calibration_trigger) {
    if (millis() + DEBOUNCE_TIME > calibration_time) {
      calibration_time = millis(); // Debounce

      if (!calibration_toggle) {
        calibration_toggle = true;
        // Reset max and min readings
        read_max = 0;
        read_min = current_read;
      } else if (calibration_toggle){
        calibration_toggle = false;
        adc_range = read_max - read_min; // Calculate and set new range
      }
    }
    calibration_trigger = false;
  }
 
  // Main process
  if (!calibration_toggle) {
    // Device mode
    if (rapid_trigger_enable) {
      true_rapid_trigger();
    } else {
      false_rapid_trigger();
    }

    Serial1.print("DEVICE:    ");
    Serial1.print(actuated);
    Serial1.print("    ");
    Serial1.println(current_level);
  } else {
    // Calibration mode
    calibrate();
    Serial1.print("CALIBR:    ");
    Serial1.print(read_min);
    Serial1.print("    ");
    Serial1.println(read_max);
  }

  delay(1); // this speeds up the simulation
}

// ISR when button is pressed
void calibration_isr() {
  calibration_trigger = true;
}

// Find maximum and minimum sensor readings
void calibrate() {
  if (current_read > read_max) {
    read_max = current_read;
  }
  if (current_read < read_min) {
    read_min = current_read;
  }
}

void true_rapid_trigger() {
  if (current_level == 0 || current_level + RT_RELEASE * 10 < local_max_level) {
    local_max_level = current_level;
    actuated = 0;
  }
  if (current_level > local_max_level) {
    local_max_level = current_level;
    actuated = 1;
  }
}

void false_rapid_trigger() {
  actuated = current_level >= actuation_level;
}
