#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>

#define DEBUG

#define ANALOG_MOISTURE_SENSOR_PIN A0
#define ANALOG_SUNLIGHT_SENSOR_PIN A1
#define REF_3V3 A2 //3.3V power on the Arduino board
#define ANALOG_PH_SENSOR_PIN A3
#define PH_OFF_SET 0
#define DHTPIN                      4 //pin for the digital temprature and humidity sensor
#define DHTTYPE                     DHT11 // type of dhtxx series to be used

#define WIRE_PROTOCOL_ADDRESS 0xB1

int moisture_sensor_analog_value;
float relative_moisture_level;
float ultraviolet_intensity_value;
float pH_sensor_value;
float humidity;
float dht11temp; // temprature value from the dht11 sensor

/**
 * declaring objects for the sensors
 */
DHT dht11(DHTPIN,DHTTYPE );

void setup() {

  #ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Field Parameters Measurement System");
  #endif
  
  Wire.begin(WIRE_PROTOCOL_ADDRESS);
  Wire.onRequest(requestEvent);

  pinMode(ANALOG_MOISTURE_SENSOR_PIN, INPUT);
  pinMode(ANALOG_SUNLIGHT_SENSOR_PIN, INPUT);
  pinMode(REF_3V3, INPUT);
  pinMode(ANALOG_PH_SENSOR_PIN, INPUT);
  
}

void loop() {

  dht11temp = readDHT11Temp(dht11);
  humidity = readDHT11Humidity(dht11);
  moisture_sensor_analog_value = readSoilMoisture(ANALOG_MOISTURE_SENSOR_PIN);
  relative_moisture_level = mapFloat(moisture_sensor_analog_value, 0, 1023, 0, 10);
  ultraviolet_intensity_value = readUVlight(ANALOG_SUNLIGHT_SENSOR_PIN, REF_3V3);
  pH_sensor_value = readpHValue(ANALOG_PH_SENSOR_PIN);

  #ifdef DEBUG
    Serial.print("Moisture : ");
    Serial.println(moisture_sensor_analog_value);
    Serial.print("pH value : ");
    Serial.println(pH_sensor_value);
    Serial.print(" UV Intensity (mW/cm^2) : ");
    Serial.print(ultraviolet_intensity_value);
    Serial.print(" Temperature : ");
    Serial.print(dht11temp);
    Serial.print("Humidity : ");
    Serial.print(humidity);
  #endif

}

float readDHT11Temp (DHT dht){
  return dht.readTemperature();
}

float readDHT11Humidity(DHT dht){
  return dht.readHumidity();
}

int readSoilMoisture(int moisture_pin) {
  return analogRead(moisture_pin);
}

float mapFloat(float x, float in_min, float in_max, float out_max,
    float out_min) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min);
}

float readUVlight(int uv_sensor_pin, int ref_3v3_pin) {
  int uvLevel = analogRead(uv_sensor_pin);
  int reference = analogRead(ref_3v3_pin);

  float outputVoltage = 3.3 / reference * uvLevel;

  #ifdef DEBUG
    Serial.print("MP8511 output: ");
    Serial.print(uvLevel);
    Serial.print(" MP8511 voltage: ");
    Serial.print(outputVoltage);
  #endif

  return mapFloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);
}

float readpHValue(int ph_sensor_pin) {
  float analogVoltage = analogRead(ph_sensor_pin) * 5.0 / 1024;
  return analogVoltage * 3.5 * PH_OFF_SET;
}

void requestEvent(){
  #ifdef DEBUG
    Serial.println("Sending the information");
  #endif
  Wire.write("PHV");
  String pHString = String(pH_sensor_value);
  Wire.write(pHString.c_str());
  Wire.write('?');
  
  Wire.write("MOI");
  Wire.write(((String)relative_moisture_level).c_str());
  Wire.write(';');
  
  Wire.write("UVI:");
  String uvString  = String(ultraviolet_intensity_value);
  Wire.write(uvString.c_str());
  Wire.write('>');

  Wire.write("TEM:");
  Wire.write(((String)dht11temp).c_str());
  Wire.write('<');

  Wire.write("HUM:");
  Wire.write(((String)humidity).c_str());
  Wire.write(':');
}


