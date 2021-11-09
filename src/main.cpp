#include <Arduino.h>

int rs485Comunication = 3;

const byte txRs485 = 18; // Porta de comunicação TX arduino.
byte incomingByte;

void serialEvent1() {
  Serial.println("-------------------");
  Serial.println("Start package:");

  while(Serial1.available() > 0) {
    incomingByte = Serial1.read();
    Serial.println("Received:");
    Serial.println(incomingByte, HEX);
  }
  
  Serial.println("End package:");
  Serial.println("-------------------");
}

void sendMsg() {
  Serial.println("Start send msg.");
  byte msgRedCont[] = {0x80, 0x05, 0x01, 0x02, 0x88};

  for (byte i = 0; i < sizeof(msgRedCont); i++)
  {
    Serial.write(msgRedCont[i]); // Envio de cada bit da mensagem
    Serial.println(msgRedCont[i], HEX);
  }
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600); 
  pinMode(rs485Comunication, OUTPUT);
  digitalWrite(rs485Comunication, LOW); // Espera receber mensagem
}

void loop() {
  // digitalWrite(rs485Comunication, HIGH);
  
  // sendMsg(); // Envia msg
  // digitalWrite(rs485Comunication, LOW); // Espera receber mensagem

}

