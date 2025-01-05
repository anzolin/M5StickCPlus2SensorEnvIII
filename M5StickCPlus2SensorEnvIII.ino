/**
 * @file M5StickCPlus2SensorEnvIII.ino
 * @author Diego Anzolin (diego@anzolin.com.br)
 * @brief Example of how you can pull temperature, humidity and pressure data from the ENV III sensor paired with the M5StickCPlus2.
 * @version 0.2
 * @date 2025-01-02
 * @Hardwares: M5StickCPlus2, Unit ENV-III (https://docs.m5stack.com/en/unit/envIII)
 */

#include "M5StickCPlus2.h"
#include "M5Unified.h"
#include "M5UnitENV.h"

SHT3X sht30;      // temperature and humidity sensor
QMP6988 qmp6988;  // pressure sensor

float temp = 0.0;
float hum = 0.0;
float pressure = 0.0;
float altitude = 0.0;

int brightness = 25;

void GetMeasurements() {
  StickCP2.Power.setLed(255);
  StickCP2.Display.setTextColor(WHITE);

  // Get new updated values from our sensor
  if (qmp6988.update()) {
    pressure = qmp6988.calcPressure();
    altitude = qmp6988.altitude;
  }

  if (sht30.update()) {    // Obtain the data of sht30.
    temp = sht30.cTemp;    // Store the temperature obtained from sht30.
    hum = sht30.humidity;  // Store the humidity obtained from the sht30.
  } else {
    temp = 0, hum = 0;
  }

  // Sometimes this sensor fails, and if we get an invalid reading it's best to just restart the controller to clear it out
  if (pressure < 950) {
    ESP.restart();
  }

  // Sometimes this sensor fails, and if we get an invalid reading it's best to just restart the controller to clear it out
  if (pressure / 100 > 1200) {
    ESP.restart();
  }

  Serial.printf("Temp: %2.1f °C \r\nHumidity: %2.0f%%  \r\nPressure: %2.0f hPa \r\nAltitude: %2.0f m\r\n\n", temp, hum, pressure / 100, altitude);

  StickCP2.Display.setTextSize(2);

  StickCP2.Display.setCursor(10, 30);
  StickCP2.Display.printf("Temperature: %2.1f*C \n", temp);

  StickCP2.Display.setCursor(10, 50);
  StickCP2.Display.printf("Humidity: %2.0f%% \n", hum);

  StickCP2.Display.setCursor(10, 70);
  StickCP2.Display.printf("Pressure: %2.0fhPa \n", pressure / 100);

  StickCP2.Display.setCursor(10, 90);
  StickCP2.Display.printf("Altitude: %2.0fm \n", altitude);

  StickCP2.Display.setTextSize(1);

  // Gather some internal data as well, about battery states, voltages, charge rates and so on
  int bat_volt = StickCP2.Power.getBatteryVoltage();
  Serial.printf("Battery Volt: %dmv \n", bat_volt);

  int bat_current = StickCP2.Power.getBatteryCurrent();
  Serial.printf("Battery Current: %dmv \n", bat_current);

  int bat_level = StickCP2.Power.getBatteryLevel();
  Serial.printf("Battery Level: %d%% \n\n", bat_level);

  StickCP2.Power.setLed(0);
}

void UpdateBatteryStatus() {
  StickCP2.Display.setCursor(210, 120);
  StickCP2.Display.setTextColor(DARKGREY, BLACK);
  StickCP2.Display.println("    ");

  int batteryLevel = StickCP2.Power.getBatteryLevel();
  int batteryLevelColor = batteryLevel < 30 ? RED : DARKGREY;

  StickCP2.Display.setTextColor(batteryLevelColor);
  StickCP2.Display.setCursor(210, 120);
  StickCP2.Display.printf("%d%%", batteryLevel);
}

void UpdateBrightnessStatus() {
  StickCP2.Display.setTextColor(DARKGREY, BLACK);
  StickCP2.Display.setCursor(170, 120);
  StickCP2.Display.println("   ");
  StickCP2.Display.setCursor(170, 120);
  StickCP2.Display.printf("%d% ", brightness);
}

void SetBrightness() {
  switch (brightness) {
    case 1:
      brightness = 25;
      break;
    case 25:
      brightness = 50;
      break;
    case 50:
      brightness = 75;
      break;
    case 75:
      brightness = 100;
      break;
    case 100:
      brightness = 1;
      break;
  }

  StickCP2.Display.setBrightness(brightness);

  Serial.printf("Brightness: %d%% \n\n", brightness);

  UpdateBrightnessStatus();
  UpdateBatteryStatus();
}

void setup() {
  // Initialize the M5StickCPlus2
  auto cfg = M5.config();

  StickCP2.begin(cfg);
  StickCP2.Display.setBrightness(25);

  // Setup the serial port for debugging
  Serial.begin(115200);
  Serial.println();
  Serial.println("Initializing...");

  Wire.begin();

  if (!qmp6988.begin(&Wire, QMP6988_SLAVE_ADDRESS_L, 32, 33, 400000U)) {
    Serial.println("Couldn't find QMP6988");
    while (1) delay(1);
  }

  if (!sht30.begin(&Wire, SHT3X_I2C_ADDR, 32, 33, 400000U)) {
    Serial.println("Couldn't find SHT30");
    while (1) delay(1);
  }

  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setRotation(3);
  StickCP2.Display.setTextSize(1);

  Serial.println("Initialized.");

  DisplayScreen();
}

void DisplayScreen() {
  StickCP2.Display.fillRect(0, 0, 240, 25, GREENYELLOW);

  StickCP2.Display.setTextColor(BLACK);
  StickCP2.Display.setCursor(10, 10);
  StickCP2.Display.println("M5StickCPlus2 with Env III sensor");

  StickCP2.Display.drawFastVLine(160, 113, 22, DARKGREY);
  StickCP2.Display.drawFastVLine(200, 113, 22, DARKGREY);
  StickCP2.Display.drawFastHLine(0, 113, 240, DARKGREY);

  StickCP2.Display.setTextColor(LIGHTGREY);
  StickCP2.Display.setCursor(10, 120);
  StickCP2.Display.println("(a) Measure (b) Light");

  UpdateBrightnessStatus();
  UpdateBatteryStatus();
}

void loop() {
  StickCP2.update();

  if (StickCP2.BtnA.wasReleased()) {
    Serial.println("Button A was released.");

    StickCP2.Speaker.tone(8000, 400);
    StickCP2.Display.fillScreen(BLACK);

    DisplayScreen();
    GetMeasurements();
  }

  if (StickCP2.BtnB.wasReleased()) {
    Serial.println("Button B was released.");

    StickCP2.Speaker.tone(6000, 400);
    SetBrightness();
  }
}
