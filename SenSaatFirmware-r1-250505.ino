/*
 * OLED Date, Time, and Temperature Display using DS1302 RTC and DHT11
 *
 * This sketch displays the current date, time, and temperature on a Deneyap OLED display
 * using the DS1302 RTC module and a DHT11 temperature sensor without flickering
 * With added check for DHT11 sensor connectivity
 *
 * Libraries used:
 * - Deneyap OLED Display: https://github.com/deneyapkart/deneyap-oled-ekran-arduino-library
 * - DS1302: https://github.com/Treboada/Ds1302
 * - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
 */
#include <Deneyap_OLED.h>
#include <Ds1302.h>
#include <Wire.h>
#include <DHT.h>

// OLED display object
// Default I2C Address is 0x7A
OLED OLED;
const uint8_t OLED_ADDRESS = 0x7A;

// DS1302 RTC pin configuration
// Change these pins according to your wiring
const uint8_t PIN_RST = D14;
const uint8_t PIN_DAT = D15;
const uint8_t PIN_CLK = D13;

// DHT11 sensor configuration
#define DHTPIN D0     // DHT11 connected to pin D0
#define DHTTYPE DHT11 // DHT 11 sensor type
DHT dht(DHTPIN, DHTTYPE);

// DS1302 RTC object
Ds1302 rtc(PIN_RST, PIN_CLK, PIN_DAT);

// Uncomment this line to set the time once, then comment it again after uploading
// #define SET_TIME

// Variables to store previous values to avoid unnecessary updates
char prevTimeStr[16] = "";
char prevDateStr[16] = "";
char prevDowStr[12] = "";
char prevTempStr[16] = "";
char prevHumStr[16] = "";
char prevDHTStatusStr[16] = "";

// Flag to track if DHT11 is connected
bool isDHTConnected = false;
bool wasDHTConnected = false;
unsigned long connectionMessageStartTime = 0;
const unsigned long connectionMessageDuration = 3000; // Show message for 3 seconds

void setup() {
  Serial.begin(115200);
 
  // Initialize I2C
  Wire.begin();
 
  // Initialize OLED display
  if (!OLED.begin(OLED_ADDRESS)) {
    Serial.println("OLED display initialization failed!");
    while (1) {
      delay(100);
    }
  }
 
  // Clear display once at startup
  OLED.clearDisplay();
 
  // Initialize RTC module
  rtc.init();
 
  // Initialize DHT sensor
  dht.begin();
  
  // Check if DHT11 is connected
  checkDHTConnection();
 
  // Set initial time if defined
  #ifdef SET_TIME
    Ds1302::DateTime dt;
    dt.year = 25;       // Year since 2000, so 2025 is 25
    dt.month = 5;       // Month (1-12)
    dt.day = 8;         // Day of month (1-31)
    dt.hour = 22;       // Hour (0-23)
    dt.minute = 15;      // Minute (0-59)
    dt.second = 00;      // Second (0-59)
    dt.dow = 4;         // Day of week (1-7, where 1 is Monday, 7 is Sunday)
   
    rtc.setDateTime(&dt);
  #endif
}

// Function to check if DHT11 sensor is connected
void checkDHTConnection() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // If either reading is not NaN, the sensor is connected
  if (!isnan(humidity) || !isnan(temperature)) {
    isDHTConnected = true;
    Serial.println("DHT11 sensor connected!");
  } else {
    isDHTConnected = false;
    Serial.println("DHT11 sensor not connected or not functioning properly!");
  }
}

// Function to read temperature from DHT11 sensor
float readTemperature() {
  // Read temperature from DHT11 (returns NaN if reading fails)
  float temperature = dht.readTemperature();
  
  // Print to serial for debugging
  Serial.print("Temperature reading: ");
  
  // Check if reading failed
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return 0; // Return 0 if failed (could return last valid reading instead)
  }
  
  Serial.print(temperature);
  Serial.println(" °C");
  
  return temperature;
}

// Function to read humidity from DHT11 sensor
float readHumidity() {
  // Read humidity from DHT11 (returns NaN if reading fails)
  float humidity = dht.readHumidity();
  
  // Print to serial for debugging
  Serial.print("Humidity reading: ");
  
  // Check if reading failed
  if (isnan(humidity)) {
    Serial.println("Failed to read humidity from DHT sensor!");
    return 0; // Return 0 if failed
  }
  
  Serial.print(humidity);
  Serial.println(" %");
  
  return humidity;
}

void loop() {
  // Check DHT connection status every cycle (every 2 seconds)
  checkDHTConnection();
  
  // Track when the sensor is newly connected
  if (isDHTConnected && !wasDHTConnected) {
    connectionMessageStartTime = millis();
    wasDHTConnected = true;
  } else if (!isDHTConnected) {
    wasDHTConnected = false;
  }
  
  // Get current time from RTC
  Ds1302::DateTime now;
  rtc.getDateTime(&now);
 
  // Format date and time strings
  char dateStr[16];
  char timeStr[16];
  char dowStr[12];
  char tempStr[16];
  char humStr[16];
  char dhtStatusStr[16];
 
  // Convert day of week to string
  const char* daysOfWeek[] = {"", "Ptesi", " Sali", "Crsmb", "Prsmb", " Cuma", "Ctesi", "Pazar"};
 
  // Format the date, time and day strings
  sprintf(dateStr, "%02d/%02d/%04d", now.day, now.month, 2000 + now.year);
  sprintf(timeStr, "%02d:%02d:%02d", now.hour, now.minute, now.second);
 
  if(now.dow >= 1 && now.dow <= 7) {
    sprintf(dowStr, "%s", daysOfWeek[now.dow]);
  } else {
    sprintf(dowStr, "Unknown");
  }
  
  // Only show "DHT11 connected" message for 3 seconds after connection
  bool showConnectionMessage = isDHTConnected && 
                              (millis() - connectionMessageStartTime < connectionMessageDuration);
  
  if (showConnectionMessage) {
    sprintf(dhtStatusStr, "DHT11 algilandi");
  } else {
    dhtStatusStr[0] = '\0'; // Empty string
  }
  
  // Only update the day of week text if it has changed
  if (strcmp(prevDowStr, dowStr) != 0) {
    // Clear the first line completely before writing new day of week
    OLED.setTextXY(0, 0);
    OLED.putString("                "); // 16 spaces to clear the line
   
    OLED.setTextXY(0, 5);
    OLED.putString(dowStr);
    strcpy(prevDowStr, dowStr);
  }
 
  // Only update the date text if it has changed
  if (strcmp(prevDateStr, dateStr) != 0) {
    OLED.setTextXY(1, 0);
    OLED.putString("                "); // Clear line
    OLED.setTextXY(1, 3);
    OLED.putString(dateStr);
    strcpy(prevDateStr, dateStr);
  }
 
  // Only update the time text if it has changed
  if (strcmp(prevTimeStr, timeStr) != 0) {
    OLED.setTextXY(2, 0);
    OLED.putString("                "); // Clear line
    OLED.setTextXY(2, 4);
    OLED.putString(timeStr);
    strcpy(prevTimeStr, timeStr);
  }
  
  // Update DHT status line
  if (strcmp(prevDHTStatusStr, dhtStatusStr) != 0) {
    OLED.setTextXY(4, 0);
    OLED.putString("                "); // Clear line
    
    if (dhtStatusStr[0] != '\0') {
      OLED.setTextXY(4, 0);
      OLED.putString(dhtStatusStr);
    }
    
    strcpy(prevDHTStatusStr, dhtStatusStr);
  }
  
  // Only display temperature and humidity if DHT11 is connected
  if (isDHTConnected) {
    // Read temperature and humidity
    float temperature = readTemperature();
    float humidity = readHumidity();
    
    sprintf(tempStr, "Sicaklik: %.1f C", temperature);
    sprintf(humStr, "Nem: %.1f%%", humidity);
    
    // Update temperature display
    if (strcmp(prevTempStr, tempStr) != 0) {
      OLED.setTextXY(5, 0);
      OLED.putString("                "); // Clear line
      OLED.setTextXY(5, 0);
      OLED.putString(tempStr);
      strcpy(prevTempStr, tempStr);
    }

    // Update humidity display
    if (strcmp(prevHumStr, humStr) != 0) {
      OLED.setTextXY(6, 0);
      OLED.putString("                "); // Clear line
      OLED.setTextXY(6, 0);
      OLED.putString(humStr);
      strcpy(prevHumStr, humStr);
    }
  } else {
    // Clear DHT status, temperature and humidity lines if DHT is not connected
    if (prevDHTStatusStr[0] != '\0') {
      OLED.setTextXY(4, 0);
      OLED.putString("                ");
      prevDHTStatusStr[0] = '\0';
    }
    
    if (prevTempStr[0] != '\0') {
      OLED.setTextXY(5, 0);
      OLED.putString("                ");
      prevTempStr[0] = '\0';
    }
    
    if (prevHumStr[0] != '\0') {
      OLED.setTextXY(6, 0);
      OLED.putString("                ");
      prevHumStr[0] = '\0';
    }
  }
  
  // Display SenSaat text with scrolling effect
  OLED.setTextXY(7, 0);
  OLED.putString("  SenSaat  ");
  OLED.setHorizontalScrollProperties(Scroll_Right, 7, 7, Scroll_4Frames);
  OLED.activateScroll();
 
  // DHT11 can't be read more often than about once every 2 seconds
  // for reliable readings, so we'll set a slightly longer delay
  delay(2000);
}