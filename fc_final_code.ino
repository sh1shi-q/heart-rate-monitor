// define blynk configurations
#define BLYNK_TEMPLATE_ID "TMPL661wxiuNY"
#define BLYNK_TEMPLATE_NAME "HeartRateSystem"
#define BLYNK_AUTH_TOKEN "i2EZRgiEUixpyRG2HW-cb6CceU0o-9XM"

// include wifi libraries
#include <WiFi.h>
#include <WiFiClient.h>

// include Blynk app libraries
#include <BlynkSimpleEsp32.h>
#include <SPI.h>

// include OLED display libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// include pulse sensor libraries
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

// define Blynk virtual ports
#define V0  0 // app bpm gauge
#define V1  1 // app status

// define pulse sensor
PulseOximeter pox;
#define REPORTING_PERIOD_MS 1000


uint32_t tsLastReport = 0;
uint32_t lastBuzzerTime = 0;
uint32_t buzzerDuration = 200;

// define OLED screen dimentions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// define outputs

// LEDs
#define LED_Y 15
#define LED_G 2
#define LED_R 4

// buzzer
#define Buzzer 18
String status = " ";

// detect a beat from pulse sensor
void onBeatDetected() {
    Serial.println("Beat detected!");
}

void setup() {
  Serial.begin(115200);

  // Initialize Wi-Fi connection
  WiFi.begin("Redmi Note 13", "wpkgp7785");
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Blynk configuration
  Blynk.config(BLYNK_AUTH_TOKEN);
  Blynk.connect();

  // define pin modes for LEDs and buzzer
  pinMode(LED_Y, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(Buzzer, OUTPUT);

  // check if OLED display is connected
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println(F("SSD1306 allocation failed"));
      for (;;);
  }
  
  // reset display as a good practice
  display.clearDisplay();
  display.display();

  Serial.print("Initializing pulse oximeter..");

  // check if pulse sensor is starting
  if (!pox.begin()) {
      Serial.println("FAILED");
      for(;;);
  } else {
      Serial.println("SUCCESS");
  }

  // set the current for the pulse sensor LED
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
}

void loop() {

  // run the mobile app
  Blynk.run();

  // get the pulse o=from the sensor
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {

    // assign heart rate value to a variable
    float bpm = pox.getHeartRate();

    // print heart rate in the serial monitor
    Serial.print("Heart rate: ");
    Serial.print(bpm);
    Serial.println(" bpm");

    tsLastReport = millis();

    // check if the heart rate status (low, normal, high)
    if (bpm >= 0 && bpm < 60.0) {
      digitalWrite(LED_Y, HIGH);
      digitalWrite(LED_G, LOW);
      digitalWrite(LED_R, LOW);
      status = "Below average";
    } 
    else if (bpm >= 60.0 && bpm <= 100.0) {
      digitalWrite(LED_Y, LOW);
      digitalWrite(LED_G, HIGH);
      digitalWrite(LED_R, LOW);
      status = "Normal";
    } 
    else if (bpm > 100.0) {
      digitalWrite(LED_Y, LOW);
      digitalWrite(LED_G, LOW);
      digitalWrite(LED_R, HIGH);
      status = "Above average";

      // turn on the buzzer
      if (millis() - lastBuzzerTime > buzzerDuration) {
        digitalWrite(Buzzer, HIGH);
        delay(200);
        digitalWrite(Buzzer, LOW);
        lastBuzzerTime = millis();
      }
    }

    // display heart rate and status in display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print("Heart Rate: ");
    display.println(bpm, 1);
    display.println("Heart rate status: ");
    display.println(status);
    display.display();

    // print same outputs to mobile app
    Blynk.virtualWrite(V0, bpm);
    Blynk.virtualWrite(V1, status);
  }
}
