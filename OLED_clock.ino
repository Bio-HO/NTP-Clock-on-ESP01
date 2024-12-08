#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "epoch_to_time.h"
#include "MPU6050_ctrl.h"

/* OLED setup */
#define SCREEN_WIDTH 128 // OLED width
#define SCREEN_HEIGHT 64 // OLED height
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int refresh_OLED = 0;
/* OLED setup */

/* WIFI & NTP setup */
char* ssid="1st_ssid";               //WiFi SSID
char* password="1st_password";                //WiFi password

char* ssid2="2nd_ssid";               //WiFi SSID
char* password2="2nd_password";                //WiFi password

unsigned int localPort=2390;   //local port to listen for UDP packets
IPAddress timeServerIP;    //time.nist.gov NTP server address
const char* ntpServerName="time.nist.gov"; //NTP Server host name
//const char *ntpServerName = "watch.stdtime.gov.tw";

int connect_count = 0;
const int NTP_PACKET_SIZE=48;    // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];   //buffer to hold incoming and outgoing packets
WiFiUDP udp;   //UDP instance to let us send and receive packets over UDP

unsigned long epoch;
int UTC = 8;
int year;
int month;
unsigned long day;
int week;
int hour;
int minut;

double Ax,Ay,Az,Gx,Gy,Gz,Temp;
int error;

byte Uart_rx_buff[8];
byte epoch_buff[4];
int buff_start = 8;
int buff_end = 1;

Ticker flipper;

/* WIFI & NTP setup */

unsigned char logo_bmp[] =
{ 0b11111111,0b10011111,0b11111111,
  0b11111111,0b00011111,0b11111111,
  0b11111111,0b00011111,0b11111111,
  0b11111110,0b00111111,0b11111111,
  0b11111100,0b01111111,0b11111111, //5
  0b11111000,0b00000000,0b00111111,
  0b11110000,0b00000000,0b00011111,
  0b10000000,0b11111111,0b00011111,
  0b10000011,0b11111111,0b00011111,
  0b10001111,0b11111111,0b00011111, //10
  0b11001111,0b00000011,0b00011111,
  0b11001100,0b00000011,0b00011111,
  0b11001000,0b00000111,0b00011111,
  0b11000111,0b11111111,0b00111111,
  0b11000111,0b11111110,0b00111111, //15
  0b11000111,0b11111110,0b00111111,
  0b11100000,0b00000110,0b01111111,
  0b11100000,0b00000000,0b01111111,
  0b11110000,0b00000000,0b11111111,
  0b11111111,0b11111000,0b11111111  //20
};

int count = 0;

void setup() { 
    // inverse LOGO
    for(int i = 0 ; i<60 ; i++){
      logo_bmp[i]= ~logo_bmp[i];
    }
    Serial.begin(9600);

    Wire.begin(2,0);  // attach SDA & SCL to IO0 and IO2
    MPU6050_ini();
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // check if OLED start
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }
  
    // display default screen 1s
    display.display();
    delay(1000); 
    display.clearDisplay();
    display.display();
    
    //Connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(0,0);
    display.print("Connecting to ");
    display.setCursor(0,10);
    display.print("SSID:");
    display.print(ssid);
    display.display();
    
    while ((WiFi.status() != WL_CONNECTED) && (connect_count < 10)) {
      delay(500);
      Serial.print(".");
      connect_count++;
    }
    if(connect_count == 10){
      Serial.println(".");
      Serial.print("can't connect to ");
      Serial.println(ssid);
      WiFi.disconnect();
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(1);
      display.setCursor(0,0);
      display.print("Connect fail");
      display.setCursor(0,10);
      display.print("Connecting to ");
      display.setCursor(0,20);
      display.print("SSID:");
      display.print(ssid2);
      display.display();
      
      connect_count = 0;
      Serial.print("Connecting to ");
      Serial.println(ssid2);
      WiFi.begin(ssid2, password2);
      while ((WiFi.status() != WL_CONNECTED) && (connect_count < 10)) {
        delay(500);
        Serial.print(".");
        connect_count++;
      }
      if(connect_count == 10){
          Serial.print("NO WIFI");
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(1);
          display.setCursor(0,0);
          display.print("NO WIFI");
          display.setCursor(0,10);
          display.print("pls Reset");
          display.display();
          while((WiFi.status() != WL_CONNECTED));
      }
    }
    

    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(0,0);
    display.print("WiFi connected");
    display.setCursor(0,10);
    display.print("IP:");
    display.print(WiFi.localIP());
    display.setCursor(0,20);
    display.print("Starting UDP");
    display.display();
    
    //Start UDP
    Serial.println("Starting UDP");
    udp.begin(localPort);

    while(!udp.parsePacket()){
      sendNTPpacket(ntpServerName);    //send an NTP packet to a time server
      delay(1000);  // wait 1s for respond 
    }

    udp.read(packetBuffer, NTP_PACKET_SIZE);    //read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    //or two words, long. First, esxtract the two words:
    unsigned long highWord=word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord=word(packetBuffer[42], packetBuffer[43]);
    //combine the four bytes (two words) into a long integer
    //this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900=highWord << 16 | lowWord;

    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears=2208988800UL;
    // subtract seventy years:
    epoch=secsSince1900 - seventyYears;

    flipper.attach(1, flip);
    }

void loop() {    

  MPU6050_read(&Ax,&Ay,&Az,&Gx,&Gy,&Gz,&Temp,&error);

  ///////////////////////
  // OLED display handle
  ///////////////////////
  
  if(refresh_OLED){

    // date
    display.clearDisplay();
    display.fillRect(0, 52, 80, 64, SSD1306_WHITE);    
    display.setTextSize(1);
    display.setTextColor(0);
    display.setCursor(3,54);
    display.print(year);
    display.print("/");
    display.print(month);
    display.print("/");
    display.print(day);

    display.setTextColor(1);
    display.setCursor(0,15);
    display.setTextSize(4);
    if(hour < 10) {
      display.print('0');
    }
    display.print(hour);
    display.print(':');
    if(minut < 10) {
      display.print('0');
    }
    display.print(minut);


    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(0,0);
    if(error){
      display.println("Data error!!");
    }
    else{
      display.print("Temp:");
      display.print(Temp,1);
      display.print(char(247));
      display.print("C");
    }


    display.setTextSize(1);
    display.setTextColor(1);
    display.setCursor(90,0);
    display.print("UTC+");
    display.print(UTC);

    

    display.drawBitmap(108,44,logo_bmp, 20,20, 1);
    
    display.display();
    refresh_OLED = 0;
  }


  ///////////////
  // UART handle
  ///////////////

  if(Serial.available()){
    if(buff_end == 7){
      buff_end = 0;
    }
    else{
      buff_end++;
    } 
    if(buff_end == 7){
      buff_start = 0;
    }
    else{
      buff_start = buff_end + 1;
    }
    
    Uart_rx_buff[buff_end] = Serial.read();

    if((Uart_rx_buff[buff_start] == 's')&&(Uart_rx_buff[buff_end] == 'e')){
      epoch = 0;
      for(int i = 0;i < 4;i++){
        if((buff_start + i + 1) > 7){
          epoch_buff[i] = Uart_rx_buff[buff_start + i - 6];
        }
        else{
          epoch_buff[i] = Uart_rx_buff[buff_start + i + 1];
        }
      }
      epoch += epoch_buff[0] << 24;
      epoch += epoch_buff[1] << 16;
      epoch += epoch_buff[2] << 8;
      epoch += epoch_buff[3];
    }
    
  }
  
  delay(100);
}

unsigned long sendNTPpacket(const char* host) {

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(host, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();

  return 0;
}

void flip() {

  // print the hour, minute and second:
  epoch2time(&year,&month,&day,&week,&hour,&minut,epoch,UTC);
  refresh_OLED = 1;
//  Serial.print("The time is ");
//  Serial.print(year);
//  Serial.print("/");
//  Serial.print(month);
//  Serial.print("/");
//  Serial.print(day);
//  Serial.print("(");
//  switch(week){
//    case 0:
//      Serial.print("WED");
//    break;
//    case 1:
//      Serial.print("THU");
//    break;
//    case 2:
//      Serial.print("FRI");
//    break;
//    case 3:
//      Serial.print("SAT");
//    break;
//    case 4:
//      Serial.print("SUN");
//    break;
//    case 5:
//      Serial.print("MON");
//    break;
//    case 6:
//      Serial.print("TUE");
//    break;
//  }
//  Serial.print(")");
//  Serial.print(hour);
//  Serial.print(":");
//  Serial.println(minut);

  epoch++;
  if(error == 0){
    if((Ay > 0.7)&&(UTC < 12)){
      UTC++;
    }
    else if((Ay < -0.7)&&(UTC > -12)){
      UTC--;
    }
  }
  
  flipper.attach(1, flip);
}
