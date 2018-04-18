#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>

#define DEBUG

#define ANALOG_MOISTURE_SENSOR_PIN A0

#define ANALOG_SUNLIGHT_SENSOR_PIN A1

#define REF_3V3 A2 //3.3V power on the Arduino board

#define ANALOG_PH_SENSOR_PIN A3

#define PH_OFF_SET 0.00

#define DHTPIN                      4 //pin for the digital temprature and humidity sensor

#define DHTTYPE                     DHT11 // type of dhtxx series to be used

#define WIRE_PROTOCOL_ADDRESS 0xB1

int moisture_sensor_analog_value;

float relative_moisture_level;

int ultraviolet_intensity_value;

float pH_sensor_value;

float humidity = 0; // humidity value from the dht11 sensor

float dht11temp = 0; // temprature value from the dht11 sensor

/**
 * declaring objects for the sensors and initializing the DHT object that will be used to access the respective functionality of the sensor
 */
DHT dht11(DHTPIN,DHTTYPE);

void setup() {

  #ifdef DEBUG
  
    Serial.begin(9600);
    
    Serial.println("Field Parameters Measurement System");
    
  #endif
  
  Wire.begin(WIRE_PROTOCOL_ADDRESS); //Initialize the I2C protocols by joining to the network as a slave with the given address WIRE_PROTOCOL_ADDRESS
  
  Wire.onRequest(requestEvent); // This registers a function to be called when data is requested on the I2C wire protocol

  pinMode(ANALOG_MOISTURE_SENSOR_PIN, INPUT);
  
  pinMode(ANALOG_SUNLIGHT_SENSOR_PIN, INPUT);
  
  pinMode(REF_3V3, INPUT);
  
  pinMode(ANALOG_PH_SENSOR_PIN, INPUT);
  
}

void loop() {

  dht11temp = readDHT11Temp(dht11); //read the dht11 sensor values for temprature
  
  humidity = readDHT11Humidity(dht11);//read the dht11 sensor values for humidity
  
  moisture_sensor_analog_value = readSoilMoisture(ANALOG_MOISTURE_SENSOR_PIN); //read the value for the soil moisture
  
  relative_moisture_level = mapFloat(moisture_sensor_analog_value, 0, 1023, 0, 10); //convert the read value for the soil moisture to the relative value that is comparable

  ultraviolet_intensity_value = readUVLightS12D();
  
  //ultraviolet_intensity_value = readUVlight(ANALOG_SUNLIGHT_SENSOR_PIN, REF_3V3); //read the UV light intensity values
  
  pH_sensor_value = readPH(ANALOG_PH_SENSOR_PIN); // read the ph sensor values

  #ifdef DEBUG
    Serial.print("Moisture : ");
    Serial.println(moisture_sensor_analog_value);
    Serial.print("pH value : ");
    Serial.println(pH_sensor_value);
    Serial.print("UV Intensity (mW/cm^2) : ");
    Serial.println(ultraviolet_intensity_value);
    Serial.print("Temperature : ");
    Serial.println(dht11temp);
    Serial.print("Humidity : ");
    Serial.println(humidity);
    Serial.println("*************************************************************");
  #endif

  delay(1000);

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

float mapFloat(float x, float in_min, float in_max, float out_max, float out_min) {
  
  return (x - in_min) * (out_max - out_min) / (in_max - in_min);
  
}

//float readUVlight(int uv_sensor_pin, int ref_3v3_pin) {
//  
//  int uvLevel = analogRead(uv_sensor_pin);
//  
//  int reference = analogRead(ref_3v3_pin);
//
//  float outputVoltage = 3.3 / reference * uvLevel;
//
//  #ifdef DEBUG
//    Serial.print("MP8511 output: ");
//    Serial.println(uvLevel);
//    Serial.print("MP8511 voltage: ");
//    Serial.println(outputVoltage);
//  #endif
//
//  return mapFloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);
//}

int readUVLightS12D(){
  
  int sensorValue;
 
  sensorValue = analogRead(ANALOG_SUNLIGHT_SENSOR_PIN);
  
  #ifdef DEBUG
    Serial.print("UV sensor reading = ");
    Serial.print(sensorValue);
    Serial.println("");
    delay(1000);
  #endif

  return sensorValue;
  
}

float readPH(int SensorPin){

  int buf[10];                //buffer for read analog

  //Get 10 sample value from the sensor for smooth the value
  for(int i=0;i<10;i++){
    
    buf[i]=analogRead(SensorPin);
    delay(10);
    
  }

  //sort the analog from small to large
  for(int i=0;i<9;i++){
    
    for(int j=i+1;j<10;j++){
      
      if(buf[i]>buf[j]){
        
        int temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
        
      }
    }
  }
  
  unsigned long int avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue += buf[i];
   
  float phValue = (float) avgValue * 5.0 / 1024 / 6; //convert the analog into millivolt
  phValue = 3.5 * phValue + PH_OFF_SET;                      //convert the millivolt into pH value
  return phValue;
}

void requestEvent(){
  
  #ifdef DEBUG
    Serial.println("Sending the information"); // Only executed when it is compiled with the #define DEBUG is uncommented
  #endif

  /**
   * Now write the corresponding data to the wire that has been requested by the controll 
   * room arduino, data for ph, moisture, uv, temprature, humidity.
   */

   /**
    * Sending as PHV7.73?MOI1005;UVI:2.00>TEM:<:⸮0>TEM0.00<HUM0.0:
    */
  
  Wire.write("H");
  String humidityString  = String(humidity);
  Wire.write(humidityString.c_str());
  Wire.write(':');
  
  Wire.write("P");
  String pHString = String(pH_sensor_value);
  Wire.write(pHString.c_str());
  Wire.write('?');
  
  Wire.write("M");
  Wire.write(((String)moisture_sensor_analog_value).c_str());
  Wire.write(';');
  
  Wire.write("U");
  String uvString  = String(ultraviolet_intensity_value);
  Wire.write(uvString.c_str());
  Wire.write('>');

  Wire.write("T");
  String tempString  = String(dht11temp);
  Wire.write(tempString.substring(0,6).c_str());
  Wire.write('<');

  
}


