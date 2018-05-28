#include<Arduino.h>
#include <math.h>
#include <CountUpDownTimer.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


int n = 2;
int r = 1;
double p = 0.48;

RF24 radio(7, 8);

const long broadcast_addr = 1000;
const long self_addr = 2001;
const long sink_addr = 3003;
long CH_addr;

const char change_ch_message[] = "CNGCH";
//Flag saya CH atau bukan
bool flagCH = false;

//Flag CH sudah ada belum
bool flag_ch_found = false;

struct data_CH{
    long CH_addr;
    float status;
};

struct dataSt{
    long addr;
    char msg[100];
};

data_CH ch_lain;
data_CH ch_now;

dataSt gmsg;
CountUpDownTimer T(DOWN);

float findT()
{
    float phase1 = r * fmod(1 , p);
    float phase2 = 1 - p * phase1;
    float t = p / phase2;

    return t;
}

float getRandom()
{
    randomSeed(analogRead(A0));
    return random(1,100) / 100.0;
}

float getStatus()
{
  //TODO: masih dummy
    randomSeed(analogRead(A0));
    return random(1,100);

}

bool findCH()
{

    //Cek jumlah round
    if(r > n)
    {
        //Reset round
        r = 1;
    }

    float T = findT();
    float ran = getRandom();

    r++;

    if(ran < T)
    {
        return true;
    }
    else
    {
        return false;
    }

}

void setTimer(int h, int m, int s)
{
    T.SetTimer(h,m,s);
    T.StartTimer();
}

//------------------------------------------------------------------------------
//Blok fungsi untuk overloaded radio_listening----------------------------------
bool radio_listening(long addr, data_CH *ret)
{
    radio.openReadingPipe(0, addr);
    radio.startListening();

    if(radio.available())
    {
        radio.read(ret, sizeof(data_CH));
        return true;
    }

    return false;
}

bool radio_listening(long addr, dataSt* msg, unsigned int packet_size)
{
    radio.openReadingPipe(0, addr);
    radio.startListening();

    if(radio.available())
    {
        radio.read(msg, sizeof(dataSt));
        return true;
    }

    return false;
}

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

//End block fungsi radio_listening----------------------------------------------

//------------------------------------------------------------------------------
//Block fungsi radio_send--------------------------------------------------------
void radio_send(long addr, data_CH *msg)
{
    radio.stopListening();
    radio.openWritingPipe(addr);

    radio.write(msg, sizeof(data_CH));
}

void radio_send(long addr, data_CH * msg, long delay_micro)
{
    radio.openWritingPipe(addr);
    radio.stopListening();
    delay(delay_micro);
    radio.write(msg, sizeof(data_CH));
}

void radio_send(long addr, char *msg, unsigned int packet_size, long delay_micro)
{
    radio.openWritingPipe(addr);
    radio.stopListening();
    delay(delay_micro);
    radio.write(msg, sizeof(msg));
}

void radio_send(long addr, char *msg, unsigned int packet_size)
{
    radio.openWritingPipe(addr);
    radio.stopListening();
    radio.write(msg, packet_size);
}

void radio_send(long addr, dataSt* msg, long delay_micro)
{
    radio.openWritingPipe(addr);
    radio.stopListening();
    delay(delay_micro);
    radio.write(msg, sizeof(dataSt));
}

//End block fungsi radio_send---------------------------------------------------

void reset_ch_flag()
{
    //Reset status CH sebelum melakukan findCH kembali
    flagCH = false;
    flag_ch_found = false;
}

void wait_for_ch()
{
    setTimer(0, 0, 11);
    ch_now.status = 0;
    while(!T.TimeCheck(0, 0, 0))
    {
        //reset ch_sekarang
        //TODO: Jadikan fungsi
        T.Timer();

        if(radio_listening(broadcast_addr, &ch_lain))
        {
            Serial.println("Menunggu CH");
            
            if(ch_lain.status > ch_now.status)
            {
                ch_now.CH_addr = ch_lain.CH_addr;
                ch_now.status = ch_lain.status;
                flag_ch_found = true;
            }
        }
    }
}

void setup_phase()
{
    if(findCH())
    {
        //Jika terpilih jadi CH
        flagCH = true;
        Serial.println("jadiCH");
        data_CH data;
        data.CH_addr = self_addr;
        data.status = getStatus();



        Serial.print("Status: ");
        Serial.println(data.status);
        //set broadcast selama 6 detik
        setTimer(0,0,10);

        //Broadcast CH dan listening kemungkinan ada CH lain
        while(!T.TimeCheck(0,0,0))
        {
            //Serial.println("sfsf");
            T.Timer();

            //Broadcast dan listening secara bergantian
            //Broadcast saya CH di detik genap
            if(T.ShowSeconds() % 2 ==0)
            {
              radio_send(broadcast_addr, &data,random(1,200));
            }

            //Listening kemungkinan ada CH lain di detik kelipatan 3
            if(T.ShowSeconds() % 3 == 0)
            {
              if(radio_listening(broadcast_addr, &ch_lain))
              {
                  Serial.println("dapat dari CH lain: ");
                  Serial.println(ch_lain.status);

                  //cek jika dapat ada CH lain dengan status sendiri dan membandingkan
                  //TODO: status masih random
                  if(ch_lain.status > data.status)
                  {
                      //Set CH addr ke CH lain
                      flagCH = false;
                      flag_ch_found = true;
                      Serial.print(ch_lain.CH_addr);
                      Serial.println(" jadi CH");
                      ch_now.CH_addr = ch_lain.CH_addr;
                      ch_now.status = ch_lain.status;
                      //CH_addr = ch_lain.CH_addr;
                  }
                  else
                  {
                      Serial.println("Saya jadi CH");
                      flagCH = true;
                  }
              }
            }
        }
    }
    else{
        //Jika tidak jadi CH listening untuk broadcast CH

        //Setting Timer untuk lama listening menunggu broadcast dari CH
        wait_for_ch();
    }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  radio.begin();
  Serial.println("mulai");
  gmsg.addr = self_addr;

  while( (flag_ch_found == false) && (flagCH == false) )
  {
      Serial.println("setup phase");
      setup_phase();
  }

}

void loop() {
    //T.Timer();
    Serial.println("Masuk loop");
    if(flagCH)
    {
        //Jadi CH
        //radio.setPALevel(RF24_PA_HIGH);
        Serial.println("Menunggu data");
        setTimer(0, 0, 20);

         dataSt msg;

        while(!T.TimeCheck(0, 0, 0))
        {
            T.Timer();


            if(radio_listening(self_addr, &msg, sizeof(msg)))
            {
                //Kirim me sink
                Serial.println(msg.msg);
                radio_send(sink_addr, &msg, 0);
            }
        }

        //Broadcast ganti CH
        setTimer(0,0,3);

        while(!T.TimeCheck(0,0,0))
        {
            T.Timer();
            Serial.println("Ganti ch");
            char text[] = "CNGCH";
            radio_send(broadcast_addr, text, sizeof(text));
        }
    }
    else if(flag_ch_found)
    {
        //jadi node biasa
        //radio.setPALevel(RF24_PA_MIN);
        setTimer(0,15,0);
        char msg[25];
        while(!T.TimeCheck(0,0,0))
        {
            T.Timer();
            if(radio_listening(broadcast_addr,msg,sizeof(msg)))
            {
                Serial.println("Mendengar");
                Serial.println(msg);
                if(strcmp(msg, change_ch_message) == 0)
                {
                    Serial.println("Mendapatkan pesan cahnge CH");
                    break;
                }
            }

            if(T.ShowSeconds() % 3 == 0)
            {
                Serial.println("mengirim data");
                strcpy(gmsg.msg, "dummy");
                radio_send(ch_now.CH_addr, &gmsg, getRandom());
            }

        }
    }

    //TODO: Ganti cara cari ch agar bersamaan

    if(flagCH)
    {
        reset_ch_flag();
        wait_for_ch;
    }
    else
    {
      reset_ch_flag();

      while( (flag_ch_found == false) && (flagCH == false) )
      {
          setup_phase();
      }
    }
}
