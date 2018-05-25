// #include <SPI.h>
// #include <nRF24L01.h>
// #include <RF24.h>
// #include<CountUpDownTimer.h>
//
// RF24 radio(7, 8);
//
// const long broadcast_addr = 1000;
//
// struct data_CH{
//     long CH_addr;
//     long status;
// };
//
// data_CH text;
// CountUpDownTimer T(UP, HIGH);
//
//
// void setup()
// {
//   while (!Serial);
//   Serial.begin(57600);
//   Serial.println("mulai");
//   radio.begin();
//   radio.openReadingPipe(0, broadcast_addr);
//   radio.openWritingPipe(broadcast_addr);
//   // radio.openReadingPipe(0, rxAddr);
//   //
//   // radio.startListening();
//   T.StartTimer();
// }
//
// bool radio_listening(long addr, data_CH *ret)
// {
//
//     radio.startListening();
//
//     if(radio.available())
//     {
//         radio.read(ret, sizeof(ret));
//         return true;
//     }
//
//     return false;
// }
//
// void radio_send(long addr, data_CH * msg, long delay_micro)
// {
//
//     radio.stopListening();
//     delay(delay_micro);
//     radio.write(msg, sizeof(msg));
// }
//
// void loop()
// {
//     T.Timer();
//
//     if(T.ShowSeconds() % 2 == 0)
//     {
//       if(radio_listening(broadcast_addr, &text))
//       {
//           Serial.println(text.status);
//       }
//     }
//
//     if(T.ShowSeconds() % 5 == 0)
//     {
//       randomSeed(analogRead(0));
//       data_CH data;
//       data.status = 2.0;
//       radio_send(broadcast_addr, &data, random(1,200));
//     }
// }

// Pengiriman per 5 detik dengan delay 1 (bisa diextend untuk listening di setiap 1 microsecond

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);

const byte rxAddr[6] = "00003";
int a = 0, xMili = 0, xSecond = 0, xMinute = 0, xHour = 0, currentSecond = 0, currentMilis = 0;
unsigned long wTime;

struct data_CH{
    long CH_addr;
    float status;
};

float getRandom()
{
    randomSeed(analogRead(0));
    return random(1,100) / 100.0;
}

void setup()
{

  xMili = 0; xSecond = 0; xMinute = 0; xHour = 0; currentMilis=0;


 // while (!Serial);
  Serial.begin(57600);


  radio.begin();
  radio.openReadingPipe(0, rxAddr);
  radio.startListening();

}


void countTime() {
  if (xMili == 999) {
    ++xSecond;
    xMili = 0;
  }
  if ((xSecond > 59)) {
    ++xMinute;
    xSecond = 0;
    currentSecond = -1;
  }
  if (xMinute > 59) {
    ++xHour;
    xMinute = 0;

  }
  ++xMili;
}

void countMillis() {

  wTime = millis();
  if (currentMilis == 0) {
    currentMilis = wTime;
  }

  if ((wTime - currentMilis > 999)) {
    ++xSecond;
    currentMilis = wTime;
  }
  if ((xSecond > 59)) {
    ++xMinute;
    xSecond = 0;
    currentSecond = -1;
  }
  if (xMinute > 59) {
    ++xHour;
    xMinute = 0;

  }
  // ++xMili;
}


void tapListening() {

  // radio.openReadingPipe(0, rxAddr);
  // radio.startListening();
  data_CH data;
  if (radio.available())
  {
    // int a = 0;
    //const char rtext[25];
    Serial.print("Dapat data: ");
    radio.read(&data, sizeof(data_CH));
    Serial.println(data.status);
  }

}

void transmit(data_CH* Message)
{
  //Serial.print("Alamat2");
  //Serial.println(Message->status);
  //radio.setRetries(15, 15);
  radio.stopListening();
  radio.openWritingPipe(rxAddr);

  // String c = Message;
  // char pesan[25];
  // c.toCharArray(pesan, 25);

  radio.write(Message, sizeof(data_CH));
  radio.openReadingPipe(0, rxAddr);
  radio.startListening();

}

void loop()
{



  currentSecond = xSecond;
  countTime();


  tapListening();


  if ((currentSecond < xSecond) && ((xSecond % 2) == 0)) {

    // char text[25];
    // String myMessage = String(xHour, DEC) + ":" + String(xMinute, DEC) + ":" + String(xSecond, DEC);
    // myMessage.concat("Uno 2");
    // Serial.println(myMessage);

    data_CH ch;

    ch.status = getRandom();
    //Serial.print("Alamat1 ");
    //Serial.println(sizeof(ch));
    transmit(&ch);


  }
  delay(1);
}
