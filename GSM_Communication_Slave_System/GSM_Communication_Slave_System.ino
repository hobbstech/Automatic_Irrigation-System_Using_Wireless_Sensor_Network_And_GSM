#include <SIM900.h>
#include <sms.h>
#include <Wire.h>

#define DEBUG

#define first_authorized_pos       1
#define last_authorized_pos        10
#define message_length             60
#define GSM_COMM_SYSTEM_ADDRESS    0xC1

SMSGSM sms;

char message[160];
char ownerNumber[] = "0778831917"; //the number for the receipient

void setup() {
  
  Serial.begin(9600);
  
  bool started = gsm.begin(2400);  //This initializes the GSM system with a baudrate of 2400 and returns true if the gsm is successfully started and has connected to the network
  
  Wire.begin(GSM_COMM_SYSTEM_ADDRESS); //Initialize the I2C protocol for the system, and joins the network as a slave with the given address
  
  Wire.onReceive(wireReceiveHandler); //registers the method to be called when data has been sent by the control room matser arduino system
  
  #ifdef DEBUG
    if(started)
      Serial.println("\nstatus : READY\n");
    else
      Serial.println("\nstatus : IDLE\n");
  #endif
}

void loop() {
 
}


/**
 * Sents a message to the owner's number given above, and takes the bytes from the wire transmission from the master and send the message
 */
 
void wireReceiveHandler(int bytesReceived){
  
  int byteIndex = 0;
  
  while(Wire.available()){
    
    message[byteIndex] = Wire.read(); //Read the message sent by the master
    
    byteIndex ++;
    
  }

  #ifdef DEBUG
    Serial.println(message);
  #endif

  sms.SendSMS(ownerNumber, message); //send the given message to the given number
}

