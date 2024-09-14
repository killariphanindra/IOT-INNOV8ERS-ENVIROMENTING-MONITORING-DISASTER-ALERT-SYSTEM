#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MQ135.h>
#include <SoftwareSerial.h>

#define DHTPIN 2        // DHT11 data pin connected to Arduino pin 2
#define DHTTYPE DHT11   // DHT 11 sensor type
#define SOIL_MOISTURE_PIN A0  // Soil moisture sensor connected to analog pin A0
#define RAIN_SENSOR_PIN A1    // Rain sensor connected to analog pin A1
#define VIBRATION_SENSOR_PIN A2 // Vibration sensor connected to analog pin A2
#define MQ135_PIN A3    // MQ135 sensor connected to analog pin A3

#define GSM_RX 10  // SIM800L RX pin connected to Arduino pin 10
#define GSM_TX 11  // SIM800L TX pin connected to Arduino pin 11

#define RAIN_THRESHOLD 100     // Adjust based on your rain sensor's range
#define VIBRATION_THRESHOLD 500 // Adjust based on your vibration sensor's range



DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 20, 4);  // I2C address 0x27, 20 column and 4 rows
MQ135 mq135_sensor(MQ135_PIN);
SoftwareSerial gsmSerial(GSM_RX, GSM_TX);

const String PHONE_NUMBER = "+919900948408";  // Replace with your phone number

void setup() {
  Serial.begin(9600);
 
  gsmSerial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();
  
  Serial.println("Multi-Sensor System with GSM Alerts");
  lcd.setCursor(0, 0);
  lcd.print("Sensor System");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  
  initGSM();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(2000);
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  int soilMoisturePercent = map(soilMoisture, 0, 1023, 0, 100);
  int rainValue = analogRead(RAIN_SENSOR_PIN);
  int vibrationValue = analogRead(VIBRATION_SENSOR_PIN);
  float correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);

  // Check for sensor errors
  if (isnan(humidity) || isnan(temperature) || isnan(correctedPPM)) {
    Serial.println("Failed to read from a sensor!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    return;
  }

  // Print to Serial Monitor
  printSerialData(temperature, humidity, soilMoisturePercent, rainValue, vibrationValue, correctedPPM);

  // Update LCD
  updateLCD(temperature, humidity, soilMoisturePercent, rainValue, vibrationValue, correctedPPM);


  // Check for alerts
  checkAlerts(rainValue, vibrationValue);

  delay(2000);  // Wait for 2 seconds between measurements
}

void initGSM() {
  Serial.println("Initializing GSM module...");
  gsmSerial.println("AT");
  delay(1000);
  gsmSerial.println("AT+CMGF=1"); // Set SMS text mode
  delay(1000);
}

void sendSMS(String message) {
  gsmSerial.println("AT+CMGS=\""+ PHONE_NUMBER +"\"");
  delay(1000);
  gsmSerial.print(message);
  delay(100);
  gsmSerial.write(26); // End SMS with Ctrl+Z
  delay(1000);
  Serial.println("SMS sent: " + message);
}

void printSerialData(float temp, float hum, int soil, int rain, int vib, float co2) {
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print("°C, Humidity: ");
  Serial.print(hum);
  Serial.println("%");
  Serial.print("Soil Moisture: ");
  Serial.print(soil);
  Serial.println("%");
  Serial.print("Rain: ");
  Serial.print(rain);
  Serial.print(", Vibration: ");
  Serial.println(vib);
  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.println(" ppm");
  Serial.println();
}

void updateLCD(float temp, float hum, int soil, int rain, int vib, float co2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print("C H:");
  lcd.print(hum, 1);
  lcd.print("%");
  
  lcd.setCursor(0, 1);
  lcd.print("Soil:");
  lcd.print(soil);
  lcd.print("% R:");
  lcd.print(rain);

  lcd.setCursor(0, 2);
  lcd.print("Vib:");
  lcd.print(vib);
  
  lcd.setCursor(0, 3);
  lcd.print("CO2:");
  lcd.print(co2, 1);
  lcd.print(" ppm");
}

void checkAlerts(int rainValue, int vibrationValue) {
  if (rainValue < RAIN_THRESHOLD) {
    String alert = "ALERT: Heavy rain detected!";
    sendSMS(alert);
    Serial.println(alert);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERT: Heavy Rain");
    delay(2000);
  }
  
  if (vibrationValue > VIBRATION_THRESHOLD) {
    String message = "ALERT: High vibration detected! Possible earthquake.";
    sendSMS(message);
    Serial.println(message);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ALERT: High Vibration");
    lcd.setCursor(0, 1);
    lcd.print("Possible Earthquake");
    delay(2000);
  }

}
