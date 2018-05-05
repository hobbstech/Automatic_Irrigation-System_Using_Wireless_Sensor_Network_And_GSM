#include <Wire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"
#include <SD.h>
#include <SoftwareSerial.h>
#include <SIM900.h>
#include <sms.h>

#define message_length             60
#define GSM_COMM_SYSTEM_ADDRESS    0xC1

SMSGSM sms;

char message[160];
char ownerNumber[] = "0778831917"; //the number for the receipient

#define DEBUG

#define GSM_COMM_SYSTEM_ADDRESS 0xC1

#define BT_TRANSMIT_PIN 3

#define BT_RECEIVE_PIN 2

#define WATER_PUMP_RELAY_PIN 4

#define MOISTURE_THRESHOLD_LEVEL_PIN A0

#define GSM_MESSAGE_LENGTH 100

#define _sclk 52
#define _miso 50
#define _mosi 51
#define _cs 53
#define _dc 9
#define _rst 8
#define _sd_cs 7

#define BACKGROUND_COLOR ILI9340_WHITE
#define FOREGROUND_COLOR ILI9340_BLACK

Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst); //initialising the Adafruit TFT screen

float _temprature;
float _humidity;
float _uv_intensity;
int _moisture;
float _ph;

boolean water_pump_on = false;
boolean previous_pump_state = false;
boolean state_changed = false;
int moisture_threshold;
char _gsm_message[GSM_MESSAGE_LENGTH];

void setup() {

  Wire.begin(); //Initialize the wire library for I2C communication
  
  Serial.begin(9600); //intialize the Serial port communication
  
  pinMode(WATER_PUMP_RELAY_PIN, OUTPUT); //set the pin to which the water pump is connected to output mode

  Serial2.begin(9600);

  tft.begin(); //initialize the tft display 
 
  tft.fillScreen(BACKGROUND_COLOR); //fill the tft screen with background color
  
  tft.setTextColor(FOREGROUND_COLOR);//fill the tft screen with foreground color

  tft.setRotation(0);
  
  tft.setTextSize(2); //set the text size and font for the TFT display

  SD.begin(_sd_cs); //this initializes saving to the sd card

  bool started = gsm.begin(2400);  //This initializes the GSM system with a baudrate of 2400 and returns true if the gsm is successfully started and has connected to the network
 
  #ifdef DEBUG
  
    if(started)
    
      Serial.println("\nstatus : READY\n");
      
    else
    
      Serial.println("\nstatus : IDLE\n");
      
  #endif
  
  if(started){
    
    sms.SendSMS(ownerNumber, "System Started Successfully"); //send the given message to the given number
  
  }

}

char bt_message[60];

void loop() {
  
  bool message_read = false;
  int index = 0;
  
  /**
   * check if a message has been received by the bluetooth system, if so then read the 
   * message into the bt_message array buffer, with a maxmum number of characters to be read
   */
   while(Serial2.available()){
    
    bt_message[index] = Serial2.read();

    index++;

    message_read = true;
    
  }

  if(message_read){

    #ifdef DEBUG
      Serial.println("*********************************************************");
      Serial.println(bt_message);
      Serial.println("*********************************************************");
    #endif
    
  }

  /**
   * Read the respective values from the read message array
   */
  _temprature = readTemprature(bt_message);
  
  _humidity = readHumidity(bt_message);
  
  _uv_intensity = readUV(bt_message);
  
  _moisture = readMoisture(bt_message);
  
  _ph = readPH(bt_message);

  #ifdef  DEBUG
    Serial.print("Moisture : ");
    Serial.println(_moisture);
    Serial.print("Humidity : ");
    Serial.println(_humidity);
    Serial.print("UV Intensity : ");
    Serial.println(_uv_intensity);
    Serial.print("Hydrogen Potential (ph) : ");
    Serial.println(_ph);
    Serial.print("Temperature : ");
    Serial.println(_temprature);
  #endif

  /**
   * Water pump decision making
   */
  moisture_threshold = analogRead(MOISTURE_THRESHOLD_LEVEL_PIN); //read the value for threshhold water level from the variable resister acting as the control nob

  #ifdef DEBUG
    Serial.print("Moisture Threshold : ");
    Serial.println(moisture_threshold);
  #endif

  if(moisture_threshold > _moisture){
    
    water_pump_on = false;
    
    if(previous_pump_state != water_pump_on)
    
      state_changed = true;
      
    else
    
      state_changed = false;
      
    previous_pump_state = water_pump_on;
    
  }else{
    
    water_pump_on = true;
    
    if(previous_pump_state != water_pump_on)

      state_changed = true;
      
     else
     
      state_changed = false;
      
    previous_pump_state = water_pump_on;
    
  }

  /**
   * Sending data to the GSM module for sms notification
   */
  if(state_changed){
    
    if(water_pump_on){
      
      strcpy(_gsm_message,"Water Pump has been turned ON"); //copy the string message to the message to be send to the GSM
      
      digitalWrite(WATER_PUMP_RELAY_PIN, HIGH);
      
    }else{
      
      strcpy(_gsm_message,"Water Pump has been truned OFF"); //copy the string message to the message to be send to the GSM
      
      digitalWrite(WATER_PUMP_RELAY_PIN, LOW);
      
    }

    sms.SendSMS(ownerNumber, _gsm_message); //send the given message to the given number
            
  }
  
    delay(5000);

     /**
 * Displaying to the TFT screen for Adafruit
 */
 
  tft.fillScreen(BACKGROUND_COLOR);
  
  tft.setCursor(0, 0);

  tft.setTextColor(ILI9340_BLUE);//fill the tft screen with foreground color
  
  tft.println("   Field Parameters");

  tft.println();

  tft.setTextColor(FOREGROUND_COLOR);//fill the tft screen with foreground color
  
  tft.print("Temp(C) : ");
  
  tft.println(_temprature);

  tft.println();
  
  tft.print("Humid(%) : ");
  
  tft.println(_humidity);

  tft.println();
  
  tft.print("UV(Index): ");
  
  tft.println(_uv_intensity);

  tft.println();
  
  tft.print("Moist Act: ");
  
  tft.println(_moisture);

  tft.println();
  
  tft.print("Soil pH : ");
  
  tft.println(_ph);

  tft.println();

  tft.print("Moist Thr: ");
  
  tft.println(moisture_threshold);

  tft.println();

  tft.print("Pump Status : ");

  if(water_pump_on){

    tft.println("ON");
    
  }else{
    
    tft.println("OFF");
    
  }
  

  tft.println();

  tft.setTextColor(ILI9340_BLUE);//fill the tft screen with foreground color
  
  tft.println("  Grace @2018");

  /**
   * saving the field parameter data to sd card
   */
  if(!SD.exists("AIS"))
  
    SD.mkdir("AIS");
    
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  
  dataFile.print("Temperature : ");
  
  dataFile.println(_temprature);
  
  dataFile.print("Humidity : ");
  
  dataFile.println(_humidity);
  
  dataFile.print("UV Intensity : ");
  
  dataFile.println(_uv_intensity);
  
  dataFile.print("Moisture Level : ");
  
  dataFile.println(_moisture);
  
  dataFile.print("Hydrogen Potential (ph) : ");
  
  dataFile.println(_ph);
  
  dataFile.close();

    Serial.println("********************************************************");
  
}

float mapFloat(float x, float in_min, float in_max, float out_max, float out_min) {
  
  return (x - in_min) * (out_max - out_min) / (in_max - in_min);
  
}

/**
 * EXTRACTING VALUES FROM THE RECEIVING BUFFER
 */
float readTemprature(char buff[]){

  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("TEM")+3), buffString.indexOf('<')).toFloat();
  
}

int readMoisture(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("MOI")+3), buffString.indexOf(';')).toInt();
  
}

float readPH(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("PHV")+3), buffString.indexOf('?')).toFloat();
  
}

float readUV(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("UVI")+3), buffString.indexOf('>')).toFloat();
  
}

float readHumidity(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("HUM")+3), buffString.indexOf(':')).toFloat();
}
