#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN
const long address = 1000;

void radio_send(long addr, char *msg, unsigned int packet_size)
{
    radio.openWritingPipe(addr);
    radio.stopListening();
    radio.write(msg, packet_size);
}

void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  //radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  char text[] = "Hello World";
  //radio.write(&text, sizeof(text));
  radio_send(address, text, sizeof(text));
  delay(1000);
}
