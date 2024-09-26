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

struct actuation {
  int level; // Level conversion of actuation distance (mm to levels)
  int current_read = 0; // Raw input reading from HE sensor
  int current_level = 0; // Current switch depression level
  int local_max_level = 0; // Used for rapid trigger
  int actuated = 0; // 1 means actuated, 0 means not actuated

  int samples[3] = {0, 0, 0};
  int average_read = 0;
};

struct rt {
  bool enable = false;
  bool interrupt_flag = false;
  int interrupt_time = 0;
};

struct calibration {
  bool enable = false;
  bool interrupt_flag = false;
  int interrupt_time = 0; // Used for checking button debounce
  int read_max;
  int read_min;
  int adc_range = 1023;
};

actuation actuation;
rt rt;
calibration calibration;

void setup() {
  Serial.begin(115200);
  actuation.level = 10 * ACTUATION_POINT;
  
  attachInterrupt(digitalPinToInterrupt(RT_PIN), rt_toggle_isr, FALLING);
  pinMode(RT_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(CALIBRATION_PIN), calibration_isr, FALLING);
  pinMode(CALIBRATION_PIN, INPUT_PULLUP);
}

void loop() {
  actuation.current_read = analogRead(ADC_PIN);
  actuation.average_read = average_samples();
  actuation.current_level = (actuation.average_read - calibration.read_min) * LEVELS / calibration.adc_range;

  // Handle rapid trigger toggle
  if (rt.interrupt_flag) {
    if (millis() + DEBOUNCE_TIME > rt.interrupt_time) {
      if(digitalRead(RT_PIN) == LOW) {
        rt.interrupt_time = millis(); // Debounce
  
        rt.enable = !rt.enable;
      }
    }
    rt.interrupt_flag = false;
  }
  
  // Handle calibration mode toggle
  if (calibration.interrupt_flag) {
    if (millis() + DEBOUNCE_TIME > calibration.interrupt_time) {
      if(digitalRead(CALIBRATION_PIN) == LOW) {
        calibration.interrupt_time = millis(); // Debounce
  
        if (!calibration.enable) {
          calibration.enable = true;
          // Reset max and min readings
          calibration.read_max = 0;
          calibration.read_min = actuation.current_read;
        } else if (calibration.enable){
          calibration.enable = false;
          calibration.adc_range = calibration.read_max - calibration.read_min; // Calculate and set new range
        }
      }
    }
    calibration.interrupt_flag = false;
  }
 
  // Main process
  if (!calibration.enable) {
    // Device mode
    if (rt.enable) {
      true_rapid_trigger();
    } else {
      false_rapid_trigger();
    }

    Serial.print("DEVICE:    ");
    Serial.print(actuation.actuated);
    Serial.print("    ");
    Serial.println(actuation.current_level);
  } else {
    // Calibration mode
    calibrate();
    Serial.print("CALIBR:    ");
    Serial.print(calibration.read_min);
    Serial.print("    ");
    Serial.println(calibration.read_max);
  }

  delay(1); // this speeds up the simulation
}

// Average the last three ADC readings
int average_samples() {
  int sum;

  // Update samples array with most recent reading
  actuation.samples[0] = actuation.samples[1];
  actuation.samples[1] = actuation.samples[2];
  actuation.samples[2] = actuation.current_read;

  // Compute average
  for (int i = 0; i < 3; i++) {
    sum += actuation.samples[i];
  }

  return sum / 3;
}

// ISR when rapid trigger toggle button is pressed
void rt_toggle_isr() {
  rt.interrupt_flag = true;
}

// ISR when calibration mode toggle button is pressed
void calibration_isr() {
  calibration.interrupt_flag = true;
}

// Find maximum and minimum sensor readings
void calibrate() {
  if (actuation.current_read > calibration.read_max) {
    calibration.read_max = actuation.current_read;
  }
  if (actuation.current_read < calibration.read_min) {
    calibration.read_min = actuation.current_read;
  }
}

void true_rapid_trigger() {
  if (actuation.current_level == 0 || actuation.current_level + RT_RELEASE * 10 < actuation.local_max_level) {
    actuation.local_max_level = actuation.current_level;
    actuation.actuated = 0;
  }
  if (actuation.current_level > actuation.local_max_level) {
    actuation.local_max_level = actuation.current_level;
    actuation.actuated = 1;
  }
}

void false_rapid_trigger() {
  actuation.actuated = actuation.current_level >= actuation.level;
}
