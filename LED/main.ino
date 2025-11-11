#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ================= PIN DEFINISI =================
#define LED1 4

// ================= TASK (CORE 0) =================
void Task_LED_Core0(void *parameter) {
  for (;;) {
    // LED OFF when Core 0 is active
    digitalWrite(LED1, LOW);
    Serial.println("LED OFF | Core 0");
    vTaskDelay(300 / portTICK_PERIOD_MS);  // Simulate task delay for Core 0
  }
}

// ================= TASK (CORE 1) =================
void Task_LED_Core1(void *parameter) {
  for (;;) {
    // LED ON when Core 1 is active
    digitalWrite(LED1, HIGH);
    Serial.println("LED ON | Core 1");
    vTaskDelay(500 / portTICK_PERIOD_MS);  // Simulate task delay for Core 1
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);

  // Buat Task
  xTaskCreatePinnedToCore(Task_LED_Core0, "LED_Core0", 2048, NULL, 1, NULL, 0);  // Core 0
  xTaskCreatePinnedToCore(Task_LED_Core1, "LED_Core1", 2048, NULL, 1, NULL, 1);  // Core 1
}

void loop() {
  // Tidak dipakai, semua berjalan di Task
}
