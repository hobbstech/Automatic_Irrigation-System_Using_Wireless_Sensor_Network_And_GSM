#include <Wire.h>
#include <VirtualWire.h>

#define DEBUG

#define ENVIROMENT_ARDUINO_STARTING_ADDRESS 0xB0
#define ENVIROMENT_BYTES_TO_RECEIVE 50
#define RADIO_TRANSMIT_PIN 12
#define RADIO_RECEIVE_PIN 2
#define RADIO_TRANSMIT_ENABLE_PIN 3

#define NUMBER_OF_SLAVES 2

void setup() {
  Wire.begin();
  
  #ifdef DEBUG
    Serial.begin(9600);
  #endif

  // Initialise the IO and ISR
  vw_set_tx_pin(RADIO_TRANSMIT_PIN);
  vw_set_rx_pin(RADIO_RECEIVE_PIN);
  vw_set_ptt_pin(RADIO_TRANSMIT_ENABLE_PIN); // Required for DR3100
  vw_setup(2000); //Bits per second baudrate
  
}

char wireReceivingBuffer[ENVIROMENT_BYTES_TO_RECEIVE];

float average_moisture;
float total_moisture_values;
float average_ph;
float total_ph_values;
float average_uv_intensity;
float total_uv_intensity;
float average_temprature;
float total_temprature_value;
float average_humidity;
float total_humidity_values;
int count;

void loop() {

  /**
   * Initializing the variables
   */

  average_moisture = 0;
  total_moisture_values = 0;
  average_ph = 0;
  total_ph_values = 0;
  average_uv_intensity  = 0;
  total_uv_intensity = 0;
  average_temprature = 0;
  total_temprature_value = 0;
  average_humidity = 0;
  total_humidity_values = 0;

  for(int slaveIndex = 0; slaveIndex < NUMBER_OF_SLAVES; slaveIndex++){
    char * p = requestFromSlave(ENVIROMENT_BYTES_TO_RECEIVE
                , ENVIROMENT_ARDUINO_STARTING_ADDRESS + slaveIndex);

    //populating the array
    for(int i = 0; i < ENVIROMENT_BYTES_TO_RECEIVE; i++)
       wireReceivingBuffer[i] = *(p+i);
    
    //extracting values
        
    total_temprature_value =+ readTemprature(wireReceivingBuffer);
    total_moisture_values =+ readMoisture(wireReceivingBuffer);
    total_ph_values =+ readPH(wireReceivingBuffer);
    total_uv_intensity =+ readUV(wireReceivingBuffer);
    total_humidity_values =+ readHumidity(wireReceivingBuffer);
  }

  average_moisture = total_moisture_values/NUMBER_OF_SLAVES;
  average_ph = total_ph_values/NUMBER_OF_SLAVES;
  average_uv_intensity = total_uv_intensity/NUMBER_OF_SLAVES;
  average_temprature = total_temprature_value/NUMBER_OF_SLAVES;
  average_humidity = total_humidity_values/NUMBER_OF_SLAVES;

  #ifdef DEBUG
    Serial.print("average_moisture :");
    Serial.println(average_moisture);
    Serial.print("average_ph :");
    Serial.println(average_ph);
    Serial.print("average_uv_intensity :");
    Serial.println(average_uv_intensity);
    Serial.print("average_temprature :");
    Serial.println(average_temprature);
    Serial.print("average_humidity :");
    Serial.println(average_humidity);
  #endif

  /**
   * Sending information to the Control Room system through 433MHz radio Receiver
   */

   String parameterDetails = "PHV" + String(average_ph) + "?MOI" + String(average_moisture) 
                  + ";UVI" + String(average_uv_intensity) + ">TEM" + String(average_temprature)
                  + "<HUM" + String(average_humidity) + ":";
                  
   char virtualWireBuffer[parameterDetails.length()];
   
   parameterDetails.toCharArray(virtualWireBuffer,parameterDetails.length());
   
  vw_send (virtualWireBuffer, parameterDetails.length());
  vw_wait_tx(); //wait until the whole message has been send

  #ifdef DEBUG
    Serial.println("Message sent to the receiver");
    Serial.println(parameterDetails);
  #endif
  
}
/**
 * EXTRACTING VALUES FROM THE RECEIVING BUFFER
 */
float readTemprature(char buff[]){

  String buffString = String(buff);
  return buffString.substring((buffString.indexOf("TEM")+3), buffString.indexOf('<'))
              .toFloat();
  
}

float readMoisture(char buff[]){
  
  String buffString = String(buff);
  return buffString.substring((buffString.indexOf("MOI")+3), buffString.indexOf(';'))
              .toFloat();
}

float readPH(char buff[]){
  
  String buffString = String(buff);
  return buffString.substring((buffString.indexOf("PHV")+3), buffString.indexOf('?'))
              .toInt();
}

float readUV(char buff[]){
  
  String buffString = String(buff);
  return buffString.substring((buffString.indexOf("UVI")+3), buffString.indexOf('>'))
              .toInt();
}

float readHumidity(char buff[]){
  
  String buffString = String(buff);
  return buffString.substring((buffString.indexOf("HUM")+3), buffString.indexOf(':'))
              .toInt();
}

/**
 * Method that will do the Wire request to the slaves for field parameter values
 */

  char* requestFromSlave(int bytesToReceive, int slaveAddress){
  char receivingBuffer[bytesToReceive]; 
  Wire.requestFrom(slaveAddress, bytesToReceive);
  count = 0;  
  while(Wire.available()){
    receivingBuffer[count] = Wire.read();
    count++;
  }

  #ifdef DEBUG
    Serial.print("Values from Slave ");
    Serial.print(slaveAddress);
    Serial.println(receivingBuffer);
  #endif
  
  return receivingBuffer;
 }

