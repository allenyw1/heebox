// Wooting Lekker switches have 4mm total travel;
// 0.1mm highest sensitivty = 40 max sensitivity levels.
#define LEVELS 40
#define ADC_PIN 27
#define BUTTON_PIN 21
#define DEBOUNCE_TIME 100

// USER CONFIGURABLE SETTINGS
#define ACTUATION_POINT 2 // Distance from top for actuation (in mm)
#define RAPID_TRIGGER true // Enable/disable rapid trigger
#define RT_RELEASE 1 // Minimum release distance to reset rapid trigger (in mm)


// GLOBAL VARIABLES
int actuation_level; // Level conversion of actuation distance (mm to levels)
int current_read = 0; // Raw input reading from HE sensor
int current_level = 0; // Current switch depression level
int local_max_level = 0; // Used for rapid trigger
int actuated = 0; // 1 means actuated, 0 means not actuated

int calibration_toggle = 0; // 1 for calibration mode
int calibration_time = 0; // Used for checking button debounce
int calibration_trigger = 0;
int read_max;
int read_min;
int adc_range = 1023;


void setup() {
  Serial.begin(115200);
  actuation_level = 10 * ACTUATION_POINT;

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), calibration_isr, FALLING);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  current_read = analogRead(ADC_PIN);
  current_level = (current_read - read_min) * LEVELS / adc_range;

  // Handle calibration mode toggle
  if (calibration_trigger == 1) {
    if (millis() + DEBOUNCE_TIME > calibration_time) {
      calibration_time = millis(); // Debounce

      if (calibration_toggle == 0) {
        calibration_toggle = 1;
        // Reset max and min readings
        read_max = 0;
        read_min = current_read;
      } else if (calibration_toggle == 1){
        calibration_toggle = 0;
        adc_range = read_max - read_min; // Calculate and set new range
      }
    }
    calibration_trigger = 0;
  }
 
 
  if (calibration_toggle == 0) {
    // Device mode
    if (RAPID_TRIGGER) {
      true_rapid_trigger();
    } else {
      false_rapid_trigger();
    }

    Serial.print("DEVICE:    ");
    Serial.print(actuated);
    Serial.print("    ");
    Serial.println(current_level);
  } else {
    // Calibration mode
    calibrate();
    Serial.print("CALIBR:    ");
    Serial.print(read_min);
    Serial.print("    ");
    Serial.println(read_max);
  }

  delay(1); // this speeds up the simulation
}

// ISR when button is pressed
void calibration_isr() {
  calibration_trigger = 1;
}

// Find maximum and minimum sensor readings
void calibrate() {
  if (analogRead(ADC_PIN) > read_max) {
    read_max = analogRead(ADC_PIN);
  }
  if (analogRead(ADC_PIN) < read_min) {
    read_min = analogRead(ADC_PIN);
  }
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
