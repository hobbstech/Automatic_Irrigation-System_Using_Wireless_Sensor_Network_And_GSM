#include <Wire.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"
#include <SD.h>

#define DISLAPY_AND_SD_SYSTEM_ADDRESS 0xC2

#define _sclk 52
#define _miso 50
#define _mosi 51
#define _cs 10
#define _dc 9
#define _rst 8
#define _sd_cs 7

#define BACKGROUND_COLOR ILI9340_WHITE
#define FOREGROUND_COLOR ILI9340_BLACK

Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst); //initialising the Adafruit TFT screen

float _temprature;
float _humidity;
float _uv_intensity;
float _moisture;
float _ph;
char wire_message[100];

bool readingDone = false;

void setup() {
  
  Wire.begin(DISLAPY_AND_SD_SYSTEM_ADDRESS); //Initialize the wire library for I2C communication

  Wire.onReceive(onReceiveHandler);

  Wire.onRequest(onRequestHandler);
  
  Serial.begin(9600); //intialize the Serial port communication

  tft.begin(); //initialize the tft display 
  
  Serial.print("Testing Screen  : ");
  
  Serial.println(testFillScreen());
  
  tft.fillScreen(BACKGROUND_COLOR); //fill the tft screen with background color
  
  tft.setTextColor(FOREGROUND_COLOR);//fill the tft screen with foreground color

  tft.setRotation(3);
  
  tft.setTextSize(2); //set the text size and font for the TFT display

  SD.begin(_sd_cs); //this initializes saving to the sd card

}

void onReceiveHandler(int receivedBytes){

  readingDone = false;

  int count = 0;

  while(Wire.available() > 0){

    wire_message[count] = Wire.read();
    
    count ++;
    
  }

  Serial.println(wire_message);

  executeWireReceivedMessage();

  readingDone = true;
  
}

void onRequestHandler(){

  if(readingDone)
    Wire.write('1');
  else
    Wire.write('0');
  
}

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

void executeWireReceivedMessage(){

  _temprature = readTemprature(wire_message);
  
  _humidity = readHumidity(wire_message);
  
  _uv_intensity = readUV(wire_message);
  
  _moisture = readMoisture(wire_message);
  
  _ph = readPH(wire_message);
  
  /**
 * Displaying to the TFT screen for Adafruit
 */
  tft.setCursor(0, 0);

  tft.setTextColor(ILI9340_BLUE);//fill the tft screen with foreground color
  
  tft.println("   Field Parameters");

  tft.println();

  tft.setTextColor(FOREGROUND_COLOR);//fill the tft screen with foreground color
  
  tft.print("Temperature : ");
  
  tft.println(_temprature);

  tft.println();
  
  tft.print("Humidity : ");
  
  tft.println(_humidity);

  tft.println();
  
  tft.print("UV Intensity : ");
  
  tft.println(_uv_intensity);

  tft.println();
  
  tft.print("Moisture Level : ");
  
  tft.println(_moisture);

  tft.println();
  
  tft.print("Soil pH : ");
  
  tft.println(_ph);

  tft.println();

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

  //tft.fillScreen(BACKGROUND_COLOR);
  
}


void loop() {

}
