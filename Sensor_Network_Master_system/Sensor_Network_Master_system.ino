#include <Wire.h>
#include <VirtualWire.h>

#define DEBUG

#define ENVIROMENT_ARDUINO_STARTING_ADDRESS 0xB0

#define ENVIROMENT_BYTES_TO_RECEIVE 35

#define RADIO_TRANSMIT_PIN 12

#define RADIO_RECEIVE_PIN 2

#define RADIO_TRANSMIT_ENABLE_PIN 3

#define NUMBER_OF_SLAVES 1

void setup() {
  Wire.begin();
  
  #ifdef DEBUG
    Serial.begin(9600);
  #endif

  vw_set_ptt_inverted(true); 

  // Initialise the IO and ISR
  vw_set_tx_pin(RADIO_TRANSMIT_PIN); //set the radio transmit pin for the virtua; wire communication protocol
  
  //vw_set_rx_pin(RADIO_RECEIVE_PIN); //set the radio transmit pin for the virtua; wire communication protocol
  
  vw_set_ptt_pin(RADIO_TRANSMIT_ENABLE_PIN); // Required for DR3100
  
  vw_setup(4000); //Bits per second baudrate
  
}

char wireReceivingBuffer[ENVIROMENT_BYTES_TO_RECEIVE];

float average_moisture = 0;

float total_moisture_values = 0;

float average_ph = 0;

float total_ph_values = 0;

float average_uv_intensity = 0;

float total_uv_intensity = 0;

float average_temprature = 0;

float total_temprature_value = 0;

float average_humidity = 0;

float total_humidity_values = 0;

int count = 0;

void loop() {

//  /**
//   * Initializing the variables
//   */
//
//  average_moisture = 0;
//  
//  total_moisture_values = 0;
//  
//  average_ph = 0;
//  
//  total_ph_values = 0;
//  
//  average_uv_intensity  = 0;
//  
//  total_uv_intensity = 0;
//  
//  average_temprature = 0;
//  
//  total_temprature_value = 0;
//  
//  average_humidity = 0;
//  
//  total_humidity_values = 0;

  /**
   * Poll through each slave that has been connected to the master and get its enviromental readings, and add them to respective sums
   */

  for(int slaveIndex = 0; slaveIndex < NUMBER_OF_SLAVES; slaveIndex++){
    
    char * p = requestFromSlave(ENVIROMENT_BYTES_TO_RECEIVE, ENVIROMENT_ARDUINO_STARTING_ADDRESS + slaveIndex + 1);

    //populating the array
    for(int i = 0; i < ENVIROMENT_BYTES_TO_RECEIVE; i++){

      wireReceivingBuffer[i] = *(p+i);
      
    }
    
    //extracting values
        
    total_temprature_value =+ readTemprature(wireReceivingBuffer);
    
    total_moisture_values =+ readMoisture(wireReceivingBuffer);
    
    total_ph_values =+ readPH(wireReceivingBuffer);
    
    total_uv_intensity =+ readUV(wireReceivingBuffer);
    
    total_humidity_values =+ readHumidity(wireReceivingBuffer);
    
  }

  /**
   * Compute averages for each values, so as to get the general average values to send to the master, these include values for the whole field
   */

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
   
  vw_send (virtualWireBuffer, parameterDetails.length()); //send the values to the control room system through the virtual wire library, for the 433.3Mhz radio transiever
  
  vw_wait_tx(); //wait until the whole message has been send

  #ifdef DEBUG
    Serial.println("Message sent to the receiver");
    Serial.println(parameterDetails);
    Serial.println("****************************************************");
  #endif


  delay(5000);
  
}

/**
 * EXTRACTING VALUES FROM THE RECEIVING BUFFER
 */
 
float readTemprature(char buff[]){

  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("T")+1), buffString.indexOf('<'))
              .toFloat();
  
}

float readMoisture(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("M")+1), buffString.indexOf(';'))
              .toFloat();
              
}

float readPH(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("P")+1), buffString.indexOf('?'))
              .toInt();
              
}

float readUV(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("U")+1), buffString.indexOf('>'))
              .toInt();
}

float readHumidity(char buff[]){
  
  String buffString = String(buff);
  
  return buffString.substring((buffString.indexOf("H")+1), buffString.indexOf(':'))
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
    Serial.print("Values from Slave :");
    Serial.println(slaveAddress);
    Serial.print("\t");
    Serial.println(receivingBuffer);
  #endif
  
  return receivingBuffer;
 }

