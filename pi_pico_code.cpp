// Wooting Lekker switches have 4mm total travel;
// 0.1mm highest sensitivty = 40 max sensitivity levels.
#define LEVELS 40
#define ADC_PIN 27
#define CALIBRATION_PIN 20
#define RT_PIN 16
#define DEBOUNCE_TIME 100

// USER CONFIGURABLE SETTINGS
#define ACTUATION_POINT 2 // Distance from top for actuation (in mm)
#define RT_RELEASE 1 // Minimum release distance to reset rapid trigger (in mm)

// GLOBAL VARIABLES
int actuation_level; // Level conversion of actuation distance (mm to levels)
int current_read = 0; // Raw input reading from HE sensor
int current_level = 0; // Current switch depression level
int local_max_level = 0; // Used for rapid trigger
int actuated = 0; // 1 means actuated, 0 means not actuated

bool rapid_trigger_enable = false;
bool rt_toggle_flag = false;
int rt_toggle_time = 0;

bool calibration_toggle = false;
bool calibration_flag = false;
int calibration_time = 0; // Used for checking button debounce
int read_max;
int read_min;
int adc_range = 1023;


void setup() {
  Serial1.begin(115200);
  actuation_level = 10 * ACTUATION_POINT;
  
  attachInterrupt(digitalPinToInterrupt(RT_PIN), rt_toggle_isr, FALLING);
  pinMode(RT_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(CALIBRATION_PIN), calibration_isr, FALLING);
  pinMode(CALIBRATION_PIN, INPUT_PULLUP);
}

void loop() {
  current_read = analogRead(ADC_PIN);
  current_level = (current_read - read_min) * LEVELS / adc_range;

  // Handle rapid trigger toggle
  if (rt_toggle_flag) {
    if (millis() + DEBOUNCE_TIME > rt_toggle_time) {
      if(digitalRead(RT_PIN) == LOW) {
        rt_toggle_time = millis(); // Debounce
  
        rapid_trigger_enable = !rapid_trigger_enable;
      }
    }
    rt_toggle_flag = false;
  }
  
  // Handle calibration mode toggle
  if (calibration_flag) {
    if (millis() + DEBOUNCE_TIME > calibration_time) {
      if(digitalRead(CALIBRATION_PIN) == LOW) {
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
    }
    calibration_flag = false;
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

// ISR when rapid trigger toggle button is pressed
void rt_toggle_isr() {
  rt_toggle_flag = true;
}

// ISR when calibration mode toggle button is pressed
void calibration_isr() {
  calibration_flag = true;
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
