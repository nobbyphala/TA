#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN
const long address = 1000;

bool radio_listening(long addr, char* msg, unsigned int packet_size)
{
    radio.openReadingPipe(0, addr);
    radio.startListening();

    if(radio.available())
    {
        radio.read(msg, packet_size);
        return true;
    }

    return false;
}

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  //radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  char text[32] = "";
  if (radio_listening(address, text, sizeof(text))) {
    Serial.println(text);
  }
}
