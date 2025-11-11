#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>

// === Pin Definitions ===
#define LED1 16
#define LED2 4
#define BUZZER 2
#define BUTTON1 5
#define BUTTON2 36
#define BUTTON3 37
#define POT_PIN 35
#define SERVO_PIN 13
#define STEP_PIN 14
#define DIR_PIN 12
#define CLK 18
#define DT 17
#define SW 11
#define SDA_PIN 21
#define SCL_PIN 20

// === Global Variables ===
volatile int encoderPos = 0;
int lastCLKState = 0;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;

// === Task Handles ===
TaskHandle_t TaskLED1;
TaskHandle_t TaskLED2;
TaskHandle_t TaskBuzzer;
TaskHandle_t TaskStepper;
TaskHandle_t TaskServo;
TaskHandle_t TaskLCD;
TaskHandle_t TaskEncoder;
TaskHandle_t TaskPot;
TaskHandle_t TaskButton;

// === Encoder ISR ===
void IRAM_ATTR readEncoder() {
  int clkState = digitalRead(CLK);
  int dtState = digitalRead(DT);
  if (clkState != lastCLKState) {
    if (dtState != clkState)
      encoderPos++;
    else
      encoderPos--;
  }
  lastCLKState = clkState;
}

// === TASK: LED1 Blink ===
void TaskLED1Blink(void *pvParameters) {
  while (true) {
    digitalWrite(LED1, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    digitalWrite(LED1, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// === TASK: LED2 Blink ===
void TaskLED2Blink(void *pvParameters) {
  while (true) {
    digitalWrite(LED2, HIGH);
    vTaskDelay(300 / portTICK_PERIOD_MS);
    digitalWrite(LED2, LOW);
    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}

// === TASK: Buzzer Beep ===
void TaskBuzzerBeep(void *pvParameters) {
  while (true) {
    tone(BUZZER, 1000);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    noTone(BUZZER);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// === TASK: Stepper Motor ===
void TaskStepperRun(void *pvParameters) {
  bool dirState = false;
  while (true) {
    digitalWrite(DIR_PIN, dirState);
    for (int i = 0; i < 200; i++) {
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(700);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(700);
    }
    dirState = !dirState;
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// === TASK: Servo Sweep ===
void TaskServoSweep(void *pvParameters) {
  int angle = 0;
  bool increasing = true;
  while (true) {
    myServo.write(angle);
    if (increasing)
      angle += 5;
    else
      angle -= 5;

    if (angle >= 180)
      increasing = false;
    if (angle <= 0)
      increasing = true;

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// === TASK: LCD Display ===
void TaskLCDDisplay(void *pvParameters) {
  while (true) {
    int potValue = analogRead(POT_PIN);
    float voltage = (potValue / 4095.0) * 3.3;

    lcd.setCursor(0, 0);
    lcd.print("Volt: ");
    lcd.print(voltage, 2);
    lcd.print(" V   ");

    lcd.setCursor(0, 1);
    lcd.print("Enc: ");
    lcd.print(encoderPos);
    lcd.print("    ");

    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}

// === TASK: Potentiometer Reader ===
void TaskPotReader(void *pvParameters) {
  while (true) {
    int potValue = analogRead(POT_PIN);
    float voltage = (potValue / 4095.0) * 3.3;
    Serial.print("Pot: ");
    Serial.print(potValue);
    Serial.print(" (");
    Serial.print(voltage, 2);
    Serial.println("V)");
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

// === TASK: Encoder Monitor ===
void TaskEncoderMonitor(void *pvParameters) {
  while (true) {
    Serial.print("Encoder: ");
    Serial.println(encoderPos);
    vTaskDelay(700 / portTICK_PERIOD_MS);
  }
}

// === TASK: Button Handler ===
void TaskButtonCheck(void *pvParameters) {
  while (true) {
    if (!digitalRead(BUTTON1))
      Serial.println("BUTTON1 ditekan!");
    if (!digitalRead(BUTTON2))
      Serial.println("BUTTON2 ditekan!");
    if (!digitalRead(BUTTON3))
      Serial.println("BUTTON3 ditekan!");
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// === SETUP ===
void setup() {
  Serial.begin(115200);

  // === Pin Setup ===
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // === LCD & Servo Setup ===
  lcd.init();
  lcd.backlight();
  myServo.attach(SERVO_PIN);

  // === Encoder Setup ===
  lastCLKState = digitalRead(CLK);
  attachInterrupt(digitalPinToInterrupt(CLK), readEncoder, CHANGE);

  // === TASK CREATION ===
  xTaskCreatePinnedToCore(TaskLED1Blink, "LED1", 2048, NULL, 1, &TaskLED1, 0);
  xTaskCreatePinnedToCore(TaskLED2Blink, "LED2", 2048, NULL, 1, &TaskLED2, 1);
  xTaskCreatePinnedToCore(TaskBuzzerBeep, "Buzzer", 2048, NULL, 1, &TaskBuzzer, 0);
  xTaskCreatePinnedToCore(TaskStepperRun, "Stepper", 4096, NULL, 1, &TaskStepper, 0);
  xTaskCreatePinnedToCore(TaskServoSweep, "Servo", 4096, NULL, 1, &TaskServo, 1);
  xTaskCreatePinnedToCore(TaskLCDDisplay, "LCD", 4096, NULL, 1, &TaskLCD, 1);
  xTaskCreatePinnedToCore(TaskPotReader, "Pot", 2048, NULL, 1, &TaskPot, 1);
  xTaskCreatePinnedToCore(TaskEncoderMonitor, "Encoder", 2048, NULL, 1, &TaskEncoder, 1);
  xTaskCreatePinnedToCore(TaskButtonCheck, "Button", 2048, NULL, 1, &TaskButton, 0);

  lcd.setCursor(0, 0);
  lcd.print("All Task Running");
  delay(1000);
  lcd.clear();
}

void loop() {
  // Semua dikendalikan oleh FreeRTOS Task
}
