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
  bool started = gsm.begin(2400);
  Wire.begin(GSM_COMM_SYSTEM_ADDRESS);
  Wire.onReceive(wireReceiveHandler);
  #ifdef DEBUG
    if(started)
      Serial.println("\nstatus : READY\n");
    else
      Serial.println("\nstatus : IDLE\n");
  #endif
}

void loop() {
 
}

void wireReceiveHandler(int bytesReceived){
  int byteIndex = 0;
  while(Wire.available()){
    message[byteIndex] = Wire.read();
    byteIndex ++;
  }

  #ifdef DEBUG
    Serial.println(message);
  #endif

  sms.SendSMS(ownerNumber, message);
}

