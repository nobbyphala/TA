#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const long sink_addr = 3003;

RF24 radio(7, 8);

struct dataSt{
    long addr;
    char msg[10];
    unsigned long id_packet;
};

void setup() {
    // put your setup code here, to run once:
    dataSt data;
    Serial.begin(57600);
    Serial.println(sizeof(data));
    radio.begin();
    radio.openReadingPipe(0, sink_addr);
    radio.startListening();
}

void loop() {
    // put your main code here, to run repeatedly:

    dataSt data;

    if(radio.available())
    {
        radio.read(&data, sizeof(data));

        Serial.println(data.addr);
        Serial.println(data.id_packet);
        Serial.println("------------------------------------------");
    }
}
