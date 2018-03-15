#include <Wire.h>
#include <VirtualWire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"
#include <SD.h>

#define DEBUG

#define GSM_COMM_SYSTEM_ADDRESS 0xC1

#define RADIO_TRANSMIT_PIN 12

#define RADIO_RECEIVE_PIN 2

#define RADIO_TRANSMIT_ENABLE_PIN 3

#define WATER_PUMP_RELAY_PIN 4

#define MOISTURE_THRESHOLD_LEVEL_PIN A0

#define GSM_MESSAGE_LENGTH 100

#define _sclk 13
#define _miso 12
#define _mosi 11
#define _cs 10
#define _dc 9
#define _rst 8
#define _sd_cs 7

#define BACKGROUND_COLOR ILI9340_BLACK
#define FOREGROUND_COLOR ILI9340_WHITE

#define MAXIMUM_VX_MESSAGE_LENGTH 100

char vx_message[MAXIMUM_VX_MESSAGE_LENGTH];
float _temprature;
float _humidity;
float _uv_intensity;
float _moisture;
float _ph;

boolean water_pump_on = false;
boolean previous_pump_state = false;
boolean state_changed = false;
float moisture_threshold;
char _gsm_message[GSM_MESSAGE_LENGTH];

Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst); //initialising the Adafruit TFT screen

void setup() {

  Wire.begin(); //Initialize the wire library for I2C communication
  
  Serial.begin(9600); //intialize the Serial port communication
  
  pinMode(WATER_PUMP_RELAY_PIN, OUTPUT); //set the pin to which the water pump is connected to output mode

  // Initialise the IO and ISR
  vw_set_tx_pin(RADIO_TRANSMIT_PIN); //set the radio transmit pin for the virtua; wire communication protocol
  
  vw_set_rx_pin(RADIO_RECEIVE_PIN); //set the radio transmit pin for the virtua; wire communication protocol
  
  vw_set_ptt_pin(RADIO_TRANSMIT_ENABLE_PIN); // Required for DR3100
  
  vw_setup(2000); //Bits per second baudrate
  
  vw_rx_start();  //start receiver PLL running

  tft.begin(); //initialize the tft display 
  
  Serial.print("Testing Screen");
  
  Serial.println(testFillScreen());
  
  tft.fillScreen(BACKGROUND_COLOR); //fill the tft screen with background color
  
  tft.setTextColor(FOREGROUND_COLOR);//fill the tft screen with foreground color
  
  tft.setTextSize(2); //set the text size and font for the TFT display

  SD.begin(_sd_cs); //this initializes saving to the sd card
  
}

void loop() {

  vw_wait_rx_max(60000); //set maximum waiting timefor the 433.3MHz transiver

  /**
   * check if a message has been received by the virtual wire library, if so then read the 
   * message into the vx_message array buffer, with a maxmum number of characters to be read
   */
  if(vw_have_message()){ 
    
    bool message_read = vw_get_message(vx_message, MAXIMUM_VX_MESSAGE_LENGTH);
    
  }

  /**
   * Read the respective values from the read message array
   */
  _temprature = readTemprature(vx_message);
  
  _humidity = readHumidity(vx_message);
  
  _uv_intensity = readUV(vx_message);
  
  _moisture = readMoisture(vx_message);
  
  _ph = readPH(vx_message);

/**
 * Displaying to the TFT screen for Adafruit
 */
  tft.setCursor(0, 0);
  
  tft.println("Field Parameter Measurements");
  
  tft.print("Temperature : ");
  
  tft.println(_temprature);
  
  tft.print("Humidity : ");
  
  tft.println(_humidity);
  
  tft.print("UV Intensity : ");
  
  tft.println(_uv_intensity);
  
  tft.print("Moisture Level : ");
  
  tft.println(_moisture);
  
  tft.print("Hydrogen Potential (ph) : ");
  
  tft.println(_ph);

  /**
   * Water pump decision making
   */
  moisture_threshold = mapFloat(analogRead(MOISTURE_THRESHOLD_LEVEL_PIN), 0, 1023, 0, 10); //read the value for threshhold water level from the variable resister acting as the control nob

  if(moisture_threshold < _moisture){
    
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
    
    Wire.beginTransmission(GSM_COMM_SYSTEM_ADDRESS); //Begin I2C transmission to the GSM arduino system
    
    Wire.write(_gsm_message); //Write the message to be send to the GSM system
    
    Wire.endTransmission(); //End I2C transmission to the GSM arduino system
            
  }

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

  //Loop ending Here
  
}

float mapFloat(float x, float in_min, float in_max, float out_max, float out_min) {
  
  return (x - in_min) * (out_max - out_min) / (in_max - in_min);
  
}

/**
 * Testing the TFT screen, by filling with different colours
 */

unsigned long testFillScreen() {
  
  unsigned long start = micros();
  
  tft.fillScreen(ILI9340_BLACK);
  
  tft.fillScreen(ILI9340_RED);
  
  tft.fillScreen(ILI9340_GREEN);
  
  tft.fillScreen(ILI9340_BLUE);
  
  tft.fillScreen(ILI9340_BLACK);
  
  return micros() - start;
  
}

/**
 * EXTRACTING VALUES FROM THE RECEIVING BUFFER
 */
float readTemprature(char buff[]){

  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("TEM")+3), buffString.indexOf('<')).toFloat();
  
}

float readMoisture(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("MOI")+3), buffString.indexOf(';')).toFloat();
  
}

float readPH(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("PHV")+3), buffString.indexOf('?')).toInt();
  
}

float readUV(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("UVI")+3), buffString.indexOf('>')).toInt();
  
}

float readHumidity(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("HUM")+3), buffString.indexOf(':')).toInt();
}
