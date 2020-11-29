#include <ESP8266HTTPClient.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <math.h>
#include "DHT.h"
//#include <stdio.h>

//wifi manager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//#define signalMod0  D2
//#define signalMod1  // not use
//#define signalMod2  // not use
//#define signalMod3  // not use

//d3 , d4 not use 

#define DHTPIN      D0
//#define L298N_Relay D1 

#define fertpump_pin    D2
#define waterpump_pin   D5

//#define watersignal_in D6
//#define ldr D6
#define fertsignal_in  D7

#define slan_1     D11    
#define slan_2     D10    
#define slan_3     D9   
#define slan_4     D8   

#define watersignal_out D12
#define fertsignal_out  D13

#define DHTTYPE DHT11

String ControllerID;
String MqttTopic;
const char* mqttServer = "smartflowfarm.info";
const int   mqttPort = 1883;
String Globaldirthumid;

int BH1750address = 0x23; //i2c address

byte buff[2];
int donthaveuser = 0;

//connect_mqtt
  char topic[50];
//connect_mqtt

//callback
  char payloadC[112];
  int dirthumid_int;
//  const char* slan;
//  const char* waterpump;
//  const char* Liquidtype;
//  const char* slan_status;
//  const char* waterpump_status;
//  const char* fert_amount;
  char slan_C[2];
  char waterpump_C[2];
  char Liquidtype_C[2];
  char slan_status_C[3];
  char waterpump_status_C[3];
  char fert_amount_C[6];
  int fert_amount_int;
  char water_amount_C[6];
  int water_amount_int;
//callback

//make_json
  int i,j,k;
  uint16_t light_value;
  int temp_value;
  int dirthumid_value;
  int airhumid_value;
  char value[2];
  
  char Controller_id_C[25];
  char User_id_C[25];
  char light_C[9];
  char temp_C[6];
  char dirthumid_C[6];
  char airhumid_C[6];
  char json_C[163];
  char firstpart[12] = "{""\"C\":\"";
  char secondpart[14] = "\",""\"U\":\"";
  char thirdpart[14] = "\",""\"l\":\"";
  char forthpart[14] = "\",""\"t\":\"";
  char fifthpart[14] = "\",""\"d\":\"";
  char sixthpart[14] = "\",""\"a\":\"";
  char seventhpart[5] = "\"""}";

  char zero[2] = "0";
//make_json

//check user
  StaticJsonBuffer<95> JSONBuffer;
  String payload;
  char Status_parsed[3];
  char User_id_parsed[24];
  char json[100];
//check user

WiFiManager wifiManager;
WiFiClient espClient;
PubSubClient client(espClient);
DynamicJsonBuffer jsonBuffer;
DHT dht(DHTPIN, DHTTYPE);
HTTPClient http;

void setup() {
  //wifiManager.startConfigPortal("AutoConnectAP");
  Wire.begin();
  if (wifiManager.getSSID() != "") {
    wifiManager.setConnectTimeout(60 * 30); //seconds
  } else {
    wifiManager.setConnectTimeout(60 * 1); //seconds
    wifiManager.startConfigPortal("AutoConnectAP");
  }

  Serial.begin(115200);
  ControllerID = ReadControllerID();
  MqttTopic = ReadMqttTopic();
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println(WiFi.status());
  dht.begin();
  //connect_mqtt();
  pinMode(A0, INPUT);
//  pinMode(D0,OUTPUT);
//  pinMode(D1,OUTPUT);
  pinMode(D2,OUTPUT);
  pinMode(D5,OUTPUT);
  pinMode(D6,INPUT);
  pinMode(D7,OUTPUT);
  pinMode(D8,OUTPUT);
  pinMode(D9,OUTPUT);
  pinMode(D10,OUTPUT); //**
  pinMode(D11,OUTPUT);
  pinMode(D12,INPUT); //**
  pinMode(D13,OUTPUT);
//  pinMode(plot, OUTPUT);
//  pinMode(waterpump, OUTPUT);
//  pinMode(watertank, OUTPUT);
//  digitalWrite(plot, HIGH);
//  digitalWrite(waterpump, HIGH);
//  digitalWrite(watertank, HIGH);
//  WriteDirthumid("000");
//  slan_stop();
//  waterpump_off();
//  fert_off();
}

void loop() {

//  char b[5];
//  int x = 10;
//  itoa(x,b,10);
//  Serial.print(" B is ");
//  Serial.println(b);
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("disconnected");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to WiFi..");
    }
  }

  if (CheckUser()) { //has user
    Serial.println("Have User");
    if (!client.connected()) {
      connect_mqtt();
    }
    if (client.connected()) {
//      String SJson = make_json();
      make_json();
      Serial.print("memory : ");
      Serial.println(ESP.getFreeHeap());
      Serial.print("digital ");
      Serial.println(digitalRead(D6));
      Serial.print("analog ");
      Serial.println(analogRead(A0));
      Serial.print("slan status : ");
      Serial.println(ReadSlan_status());
//      Serial.print("SJson ");
//      Serial.println(SJson);
//      int str_len = SJson.length() + 1; 
      //char json[700] = "{\"C\":\"5f292f7f49b462536bb50f11\",\"U\":\"5f212f5c2dd67d143beab892\",\"l\":\"185\",\"t\":\"0\",\"d\":\"7\",\"a\":\"0\"}";
//      char json[str_len];
//      SJson.toCharArray(json, str_len);
      Serial.println(json_C);
      client.publish(MqttTopic.c_str(), json_C);
    }
  } else if (!CheckUser()) {
    Serial.println("Don't Have User");
    if (client.connected()) {
      client.disconnect();
    }
//    donthaveuser++;
//    if(donthaveuser >= 30){
//      ESP.restart();
//    }
  }

//  if (millis() > 1000 * 60 * 5) {
//    ESP.restart();  //if else in callback seems to stop working after 5 minute add this to restart every 5 minutes
//  }
  //  //CheckUser();
  //  //Serial.println(ControllerID);
  delay(500);
  client.loop();
}

//-------------------------------------------------------------------------

void connect_wifi() {
  //WiFi.begin(ssid, password);
  WiFi.begin(wifiManager.getSSID(), wifiManager.getPassword());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

void callback(char* topic, byte* payload, unsigned int length) {

  //char payloadC[length];
  //String payloadS;

  for (int i = 0; i < length; i++) {
    payloadC[i] = (char)payload[i];
  }

  //payloadS = String(payloadC);
  //JsonObject& payloadJ = jsonBuffer.parseObject(payloadS);
  //JsonObject& payloadJC = jsonBuffer.parseObject(payloadC);

  Serial.println("this is playload ");
  Serial.println(String(payloadC));
//
  Serial.print("memory : ");
  Serial.println(ESP.getFreeHeap());

//  for(i=0;i<length;i++){
//    Serial.print(payloadC[i]);
//  }
//  Serial.println("");
//  for(i=0;i<length;i++){
//    Serial.print("position :");
//    Serial.print(i);
//    Serial.print("char :");
//    Serial.println(payloadC[i]);
//  }

//  for(i = 0;i<2;i++){
//    waterpump_status[i] = (char)0;
//    slan_status[i] = (char)0;
//  }
//
//  for(i = 0;i<5;i++){
//    fert_amount[i] = (char)0;
//  }
//  
//  slan_C[0] = payloadC[5];
//  waterpump_C[0] = payloadC[17];
//  Liquidtype_C[0] = payloadC[30];
////  strcat(waterpump_status_C,payloadC[49]);
////  strcat(waterpump_status_C,payloadC[50]);
//  waterpump_status_C[0] = payloadC[49];
//  waterpump_status_C[1] = payloadC[50];
////  strcat(slan_status_C,payloadC[64]);
////  strcat(slan_status_C,payloadC[65]);
//  slan_status_C[0] = payloadC[64];
//  slan_status_C[1] = payloadC[65];
////  strcat(fert_amount_C,payloadC[97]);
////  strcat(fert_amount_C,payloadC[98]);
////  strcat(fert_amount_C,payloadC[99]);
////  strcat(fert_amount_C,payloadC[100]);
////  strcat(fert_amount_C,payloadC[101]);
//  fert_amount_C[0] = payloadC[97];
//  fert_amount_C[1] = payloadC[98];
//  fert_amount_C[2] = payloadC[99];
//  fert_amount_C[3] = payloadC[100];
//  fert_amount_C[4] = payloadC[101];

//{"slan" :0, "waterpump" :0, "L i q u i d t y p e " : 0, "waterpump_status" : "Of" , "slan_ status" : "Of","slan status":"Of", "fert amount": "100xx"}

  //slan
  slan_C[0] = (char)0;
  for(i=0;i<length;i++){
    if(payloadC[i] == 's' && payloadC[i+1] == 'l' && payloadC[i+2] == 'a' && payloadC[i+3] == 'n' && payloadC[i+4] == '"' && payloadC[i+5] == ':'){     
      slan_C[0] = payloadC[i+6];
    }
  }
//  if(slan_C[0] != '0' || slan_C[0] != '1'){
//    slan_C[0] = payloadC[5];
//  }

  //waterpump
  waterpump_C[0] = (char)0;
  for(i=0;i<length;i++){
    if(payloadC[i] == 'w' && payloadC[i+1] == 'a' && payloadC[i+2] == 't' && payloadC[i+3] == 'e' && payloadC[i+4] == 'r'
      && payloadC[i+5] == 'p' && payloadC[i+6] == 'u' && payloadC[i+7] == 'm' && payloadC[i+8] == 'p' 
      && payloadC[i+9] == '"' && payloadC[i+10] == ':'){     
      waterpump_C[0] = payloadC[i+11];
    }
  }

  //liquidtype
  Liquidtype_C[0] = (char)0;
  for(i=0;i<length;i++){
    if(payloadC[i] == 'L' && payloadC[i+1] == 'i' && payloadC[i+2] == 'q' && payloadC[i+3] == 'u' && payloadC[i+4] == 'i'
      && payloadC[i+5] == 'd' && payloadC[i+6] == 't' && payloadC[i+7] == 'y' && payloadC[i+8] == 'p' && payloadC[i+9] == 'e' 
      && payloadC[i+10] == '"' && payloadC[i+11] == ':'){     
      Liquidtype_C[0] = payloadC[i+12];
    }
  }  

  //waterpump_status
  for(i = 0;i<2;i++){
    waterpump_status_C[i] = (char)0;
  }
  for(i=0;i<length;i++){
    if(payloadC[i] == 'w' && payloadC[i+1] == 'a' && payloadC[i+2] == 't' && payloadC[i+3] == 'e' && payloadC[i+4] == 'r'
      && payloadC[i+5] == 'p' && payloadC[i+6] == 'u' && payloadC[i+7] == 'm' && payloadC[i+8] == 'p' && payloadC[i+9] == '_' 
      && payloadC[i+10] == 's' && payloadC[i+11] == 't' && payloadC[i+12] == 'a' && payloadC[i+13] == 't' && payloadC[i+14] == 'u' 
      && payloadC[i+15] == 's' && payloadC[i+16] == '"' && payloadC[i+17] == ':' && payloadC[i+18] == '"'){     
        waterpump_status_C[0] = payloadC[i+19];
        waterpump_status_C[1] = payloadC[i+20];
    }
  } 

  //slan_status
  for(i = 0;i<2;i++){
    slan_status_C[i] = (char)0;
  }
  for(i=0;i<length;i++){
    if(payloadC[i] == 's' && payloadC[i+1] == 'l' && payloadC[i+2] == 'a' && payloadC[i+3] == 'n' && payloadC[i+4] == '_'
      && payloadC[i+5] == 's' && payloadC[i+6] == 't' && payloadC[i+7] == 'a' && payloadC[i+8] == 't' && payloadC[i+9] == 'u' 
      && payloadC[i+10] == 's' && payloadC[i+11] == '"' && payloadC[i+12] == ':' && payloadC[i+13] == '"'){     
        slan_status_C[0] = payloadC[i+14];
        slan_status_C[1] = payloadC[i+15];
    }
  }

  //fert_amount
  for(i = 0;i<5;i++){
    fert_amount_C[i] = (char)0;
  }
  for(i=0;i<length;i++){
    if(payloadC[i] == 'f' && payloadC[i+1] == 'e' && payloadC[i+2] == 'r' && payloadC[i+3] == 't' && payloadC[i+4] == '_'
      && payloadC[i+5] == 'a' && payloadC[i+6] == 'm' && payloadC[i+7] == 'o' && payloadC[i+8] == 'u' && payloadC[i+9] == 'n' 
      && payloadC[i+10] == 't' && payloadC[i+11] == '"' && payloadC[i+12] == ':' && payloadC[i+13] == '"'){     
      if(payloadC[i+14] != 'x'){       
        fert_amount_C[0] = payloadC[i+14];
      }
      if(payloadC[i+15] != 'x'){       
        fert_amount_C[1] = payloadC[i+15];
      }
      if(payloadC[i+16] != 'x'){       
        fert_amount_C[2] = payloadC[i+16];
      }
      if(payloadC[i+17] != 'x'){       
        fert_amount_C[3] = payloadC[i+17];
      }
      if(payloadC[i+18] != 'x'){       
        fert_amount_C[4] = payloadC[i+18];
      }
      fert_amount_int = atoi(fert_amount_C);
    }
  }

  //fert_amount
  for(i = 0;i<5;i++){
    water_amount_C[i] = (char)0;
  }
  for(i=0;i<length;i++){
    if(payloadC[i] == 'w' && payloadC[i+1] == 'a' && payloadC[i+2] == 't' && payloadC[i+3] == 'e' && payloadC[i+4] == 'r' && payloadC[i+5] == '_'
      && payloadC[i+6] == 'a' && payloadC[i+7] == 'm' && payloadC[i+8] == 'o' && payloadC[i+9] == 'u' && payloadC[i+10] == 'n' 
      && payloadC[i+11] == 't' && payloadC[i+12] == '"' && payloadC[i+13] == ':' && payloadC[i+14] == '"'){     
      if(payloadC[i+15] != 'x'){       
        water_amount_C[0] = payloadC[i+15];
      }
      if(payloadC[i+16] != 'x'){       
        water_amount_C[1] = payloadC[i+16];
      }
      if(payloadC[i+17] != 'x'){       
        water_amount_C[2] = payloadC[i+17];
      }
      if(payloadC[i+18] != 'x'){       
        water_amount_C[3] = payloadC[i+18];
      }
      if(payloadC[i+19] != 'x'){       
        water_amount_C[4] = payloadC[i+19];
      }
      water_amount_int = atoi(water_amount_C);
    }
  }

  dirthumid_int = 0;

//  Serial.print("slan_C ");
//  Serial.println(payloadC[5]);
//  Serial.print("waterpump_C ");
//  Serial.println(payloadC[17]);
//  Serial.print("Liquidtype_C ");
//  Serial.println(payloadC[30]);
//  Serial.print("waterpump_status_C ");
//  Serial.print(payloadC[49]);
//  Serial.println(payloadC[50]);
//  //Serial.println(waterpump_status_C);
//  Serial.print("slan_status_C ");
//  Serial.print(payloadC[64]);
//  Serial.println(payloadC[65]);
//  //Serial.println(slan_status_C);
//  Serial.print("fert_amount_C ");
//  Serial.print(payloadC[97]);
//  Serial.print(payloadC[98]);
//  Serial.print(payloadC[99]);
//  Serial.print(payloadC[100]);
//  Serial.println(payloadC[101]);
//  //Serial.println(fert_amount_C);

  Serial.print("slan_C ");
  Serial.println(slan_C);
  Serial.print("waterpump_C ");
  Serial.println(waterpump_C);
  Serial.print("Liquidtype_C ");
  Serial.println(Liquidtype_C);
  Serial.print("waterpump_status_C ");
  Serial.println(waterpump_status_C);
  Serial.print("slan_status_C ");
  Serial.println(slan_status_C);
  Serial.print("fert_amount_C ");
  Serial.println(fert_amount_C);
  Serial.print("fert_amount_int ");
  Serial.println(fert_amount_int);
  Serial.print("water_amount_C ");
  Serial.println(water_amount_C);
  Serial.print("water_amount_int ");
  Serial.println(water_amount_int);
              
//  String slan = payloadJ[String("slan")];
//  String waterpumps = payloadJ[String("waterpump")];
//  String Liquidtype = payloadJ[String("Liquidtype")];
//  String InstaLightOn = payloadJ[String("InstaLightOn")];
//  String InstaLightOff = payloadJ[String("InstaLightOff")];
//  String InstaWaterOn = payloadJ[String("InstaWaterOn")];
//  String InstaWaterOff = payloadJ[String("InstaWaterOff")];
//  String slan_status = payloadJ[String("slan_status")];
//  String waterpump_status = payloadJ[String("waterpump_status")];
//  String fert_amount = payloadJ[String("fert_amount")];

//  slan = payloadJC["slan"];
//  waterpump = payloadJC["waterpump"];
//  Liquidtype = payloadJC["Liquidtype"];
//  slan_status = payloadJC["slan_status"];
//  waterpump_status = payloadJC["waterpump_status"];
//  fert_amount = payloadJC["fert_amount"];
  //strncpy(web.user, data["web.user"], sizeof(web.user));

//  Serial.print("slan ");
//  Serial.println(slan);
//  Serial.print("waterpump ");
//  Serial.println(waterpump);
//  Serial.print("Liquidtype ");
//  Serial.println(Liquidtype);
//  Serial.print("slan_status ");
//  Serial.println(slan_status);
//  Serial.print("waterpump_status ");
//  Serial.println(waterpump_status);
//  Serial.print("fert_amount ");
//  Serial.println(fert_amount);
//  Serial.print("slan_status ");
//  Serial.println(slan_status);
//  Serial.print("waterpump_status ");
//  Serial.println(waterpump_status);
//  Serial.print("fert_amo ");
//  Serial.println(fert_amount);

//  WriteDirthumid("000");  //reset average per minute

//  Serial.println(strcmp(slan_status,"On"));
//  Serial.println(strcmp(slan_status,"On"));
  if(!strcmp(slan_status_C,"On")){
    Serial.print("digitalRead ");
    Serial.println(digitalRead(D6));
    Serial.println(" Slan On Mode " );
     if(ReadSlan_status() != 1){
     //if(analogRead(A0) >= 300){
     //if(digitalRead(D6) == LOW){
        slan_on();
        //delay(1000*14);
        slan_stop();
        WriteSlan_status(1);   
     }
  }
  else if(!strcmp(slan_status_C,"Of")){
    Serial.println(" Slan Off Mode " );
     if(ReadSlan_status() != 0){
     //if(analogRead(A0) <= 300){
     //if(digitalRead(D6) == HIGH){ 
        slan_off();
        //delay(1000*14);
        slan_stop();
        WriteSlan_status(0);   
     }
  }else if(!strcmp(slan_status_C,"Au")){
     Serial.println("Slan auto mode");
     Serial.println(!strcmp(slan_C,"1"));
     Serial.println(!strcmp(slan_C,"0"));
     if(!strcmp(slan_C,"1")){
        Serial.println("Slan auto mode on");
        if(ReadSlan_status() != 1){
        //if(digitalRead(D6) == LOW){
          slan_on();
          //delay(1000*14);
          slan_stop();
          WriteSlan_status(1);   
        }
     }else if(!strcmp(slan_C,"0")){
        Serial.println("Slan auto mode off");
        if(ReadSlan_status() != 0){
        //if(digitalRead(D6) == HIGH){ 
          slan_off();
          //delay(1000*14);
          slan_stop();
          WriteSlan_status(0);   
        }
     }
  }

  if(!strcmp(waterpump_status_C,"On")){
     Serial.println("waterpump_on mode");
     if(!strcmp(Liquidtype_C,"1")){
       //check for water_in
       //if no water_in fert_on
       fert_on();
       //delay for fert_amount/3 
       delay( (fert_amount_int)/3 * 1000);
       //fert_off
       fert_off();
       //check for fert_in
       //if no fert_in waterpump_on
       waterpump_on();
       //delay 10s
       delay(10*1000);
       //waterpump_off
       waterpump_off();
     }
     else{
       //check for fert_in
       //if no fert_in waterpump_on
       waterpump_on();
       //delay 1s
       delay(1000);
     }
     //save waterpump_status as 1
     //WriteWaterpump_status(1);
  }
  else if(!strcmp(waterpump_status_C,"Of")){
    Serial.println("waterpump_off mode");
     //waterpump_off
     waterpump_off();
     //delay 30s for stablelize value
     delay(15*1000);
     //check if first value is "000"
     //loop 30 time
     //if "000" save new value 
     //if not "000" then (new value + old value)/2 and save as new value
     //delay 1s
     //end loop
     //temporary stop average value for bug in void lopp which will not finish char array 
     dirthumid_int = analogRead(A0);
     for(i = 0;i<30;i++){
        //pull value from eeprom
        //String Edirthumid = ReadDirthumidEEProm();
        
        //calculate new value by average it with new vaule
        dirthumid_int = (dirthumid_int + analogRead(A0))/2;
        //Serial.print("dirthumid_int ");
        //Serial.println(dirthumid_int);
        //send value back to eeprom
        //Serial.println(Edirthumid);
        //WriteDirthumid(Edirthumid);
        delay(500);
      }
  }
  else if(!strcmp(waterpump_status_C,"Au")){
     Serial.println("waterpump_auto");
     if(!strcmp(waterpump_C,"1")){
        if(!strcmp(Liquidtype_C,"1")){
           //check for water_in 
           //if no water_in fert_on
           fert_on();
           //delay for fert_amount/3 
           delay( (fert_amount_int)/3 * 1000);
           //fert_off
           fert_off();
           //check for fert_in
           //if no fert_in waterpump_on
           //waterpump_on();
           //delay 10s
           //delay(10*1000);
           //waterpump_off
           //waterpump_off();
        }else {
           //check for fert_in
           //if no fert_in waterpump_on
           waterpump_on();
           //delay 30s
           delay(15*1000);
           //waterpump_off
           waterpump_off();
           //delay 30s for stablelize value
           delay(15*1000);
           //check if first value is "000"
           //loop 30 time
           //if "000" save new value 
           //if not "000" then (new value + old value)/2 and save as new value
           //delay 1s
           //end loop   
           //temporary stop average value for bug in void lopp which will not finish char array 
           dirthumid_int = analogRead(A0);
           for(i = 0;i<30;i++){
              //pull value from eeprom
              //String Edirthumid = ReadDirthumidEEProm();
              
              //calculate new value by average it with new vaule
              dirthumid_int = (dirthumid_int + analogRead(A0))/2;
              //Serial.print("dirthumid_int ");
              //Serial.println(dirthumid_int);
              //send value back to eeprom
              //Serial.println(Edirthumid);
              //WriteDirthumid(Edirthumid);
              delay(500);
            } 
        }
     }else{
       //waterpump_off
       waterpump_off();
       //delay 30s for stablelize value
       delay(15*1000);
       //check if first value is "000"
       //loop 30 time
       //if "000" save new value 
       //if not "000" then (new value + old value)/2 and save as new value
       //delay 1s
       //end loop
       //pull value from eeprom
       //temporary stop average value for bug in void lopp which will not finish char array 
     }
  }
  else if(!strcmp(waterpump_status_C,"Ti")){
     Serial.println("waterpump_timed");
     if(!strcmp(waterpump_C,"1")){
        if(!strcmp(Liquidtype_C,"1")){
           //check for water_in 
           //if no water_in fert_on
           fert_on();
           //delay for fert_amount/3 
           delay( (fert_amount_int)/3 * 1000);
           //fert_off
           fert_off();
           //check for fert_in
           //if no fert_in waterpump_on
           //waterpump_on();
           //delay 10s
           //delay(10*1000);
           //waterpump_off
           //waterpump_off();
        }else {
           //check for fert_in
           //if no fert_in waterpump_on
           waterpump_on();
           //delay 30s
           //delay(15*1000);
           delay( (water_amount_int)/3 * 1000);
           //waterpump_off
           waterpump_off();
           //delay 30s for stablelize value
           delay(15*1000);
           //check if first value is "000"
           //loop 30 time
           //if "000" save new value 
           //if not "000" then (new value + old value)/2 and save as new value
           //delay 1s
           //end loop   
           //temporary stop average value for bug in void lopp which will not finish char array 
           dirthumid_int = analogRead(A0);
           for(i = 0;i<30;i++){
              //pull value from eeprom
              //String Edirthumid = ReadDirthumidEEProm();
              
              //calculate new value by average it with new vaule
              dirthumid_int = (dirthumid_int + analogRead(A0))/2;
              //Serial.print("dirthumid_int ");
              //Serial.println(dirthumid_int);
              //send value back to eeprom
              //Serial.println(Edirthumid);
              //WriteDirthumid(Edirthumid);
              delay(500);
           } 
        }
     }else{
       //waterpump_off
       waterpump_off();
       //delay 30s for stablelize value
       delay(15*1000);
       //check if first value is "000"
       //loop 30 time
       //if "000" save new value 
       //if not "000" then (new value + old value)/2 and save as new value
       //delay 1s
       //end loop
       //pull value from eeprom
       //temporary stop average value for bug in void lopp which will not finish char array 
       dirthumid_int = analogRead(A0);
       for(i = 0;i<30;i++){
          //pull value from eeprom
          //String Edirthumid = ReadDirthumidEEProm();
          
          //calculate new value by average it with new vaule
          dirthumid_int = (dirthumid_int + analogRead(A0))/2;
          //Serial.print("dirthumid_int ");
          //Serial.println(dirthumid_int);
          //send value back to eeprom
          //Serial.println(Edirthumid);
          //WriteDirthumid(Edirthumid);
          delay(500);
        }
     }
  }
}

void slan_on(){
      Serial.println("slan_on");
      digitalWrite(slan_1,1023);
      analogWrite(slan_2,LOW);
      digitalWrite(slan_3,600);
      analogWrite(slan_4,LOW);
      //delay(14000);
      delay(1000*13);
//      analogWrite(slan_1,LOW);//175
//      digitalWrite(slan_2,LOW);
//      analogWrite(slan_3,600);//255
//      digitalWrite(slan_4,LOW);     
//      delay(1000); 
}

void slan_off(){
      Serial.println("slan_off");
      analogWrite(slan_1,LOW);//175
      digitalWrite(slan_2,600);
      analogWrite(slan_3,LOW);//255
      digitalWrite(slan_4,1023);
      delay(1000*13);
      //delay(14000);
//      analogWrite(slan_1,LOW);//175
//      digitalWrite(slan_2,600);
//      analogWrite(slan_3,LOW);//255
//      digitalWrite(slan_4,LOW);     
//      delay(1000); 
}

void slan_stop(){
      Serial.println("slan_stop");
      analogWrite(slan_1,LOW);//175
      digitalWrite(slan_2,LOW);
      analogWrite(slan_3,LOW);//255
      digitalWrite(slan_4,LOW);
      delay(1000);
}

void waterpump_on(){
      Serial.println("waterpump_on");
      digitalWrite(waterpump_pin,HIGH);
      //digitalWrite(watersignal_out,HIGH);
      //digitalWrite(fertsignal_in,LOW);
}

void waterpump_off(){
      Serial.println("waterpump_off");
      digitalWrite(waterpump_pin,LOW);
      //digitalWrite(watersignal_out,LOW);
      //digitalWrite(fertsignal_in,LOW);
}

void fert_on(){
      Serial.println("fert_on");
      digitalWrite(fertpump_pin,HIGH);
      //digitalWrite(watersignal_out,LOW);
      //digitalWrite(fertsignal_in,HIGH);
}

void fert_off(){
      Serial.println("fert_off");
      digitalWrite(fertpump_pin,LOW);
      //digitalWrite(watersignal_out,LOW);
      //digitalWrite(fertsignal_in,LOW);
}

void connect_mqtt() {
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    //String Con_id = "5f212e362dd67d143beab891";

    const char* id = ControllerID.c_str();
    //Serial.println(id);
    //if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
    if (client.connect(id)) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }
  String mqtt_topic = ControllerID;
  mqtt_topic.toCharArray(topic, 50);
  client.subscribe(topic);
}

bool CheckUser() {
  http.begin("http://188.166.248.109:3000/controller/CheckIfHasOwner/" + ControllerID);
  int httpCode = http.GET();

  //payload = http.getString();
  
  http.getString().toCharArray(json, 100);

  //  for(i=0;i<length;i++){
//    Serial.print("position :");
//    Serial.print(i);
//    Serial.print("char :");
//    Serial.println(payloadC[i]);
//  }
//  Serial.print("payload http ");
//  Serial.println(payload);

  http.end();

//  Serial.print("char array ");
//  Serial.println(json);
//  for(i=0;i<100;i++){
//    Serial.print("position :");
//    Serial.print(i);
//    Serial.print("char :");
//    Serial.println(json[i]);
//  }
  Status_parsed[0] = json[11];
  Status_parsed[1] = json[12];

  //JsonObject& parsed = JSONBuffer.parseObject(payload);
  //JsonObject& parsed = jsonBuffer.parseObject(payload);
  //unsigned char testparsed;
  //testparsed = parsed.get<unsigned char>("status");
  
  //const char* Status_parsed_C;
  //const char* User_id_parsed_C;
  //String Status_parsed_C = parsed["status"];
  //Status_parsed.concat(parsed["status"]);
  //String User_id_parsed_C = parsed["User_id"];
  //User_id_parsed.concat(parsed["User_id"]);

//      Serial.print("Status_parsed_C ");
//      Serial.println(Status_parsed_C);
//      Serial.println("parsed ");
//      Serial.println(User_id_parsed_C);

  //parsed status first if have user parsed user

  //if (parsed.success()) { //Check for errors in parsing

    if (!strcmp(Status_parsed,"Ye")) {
      for(i = 0;i<23;i++){
        User_id_parsed[i] = json[i+26];
      }
      Serial.println("Ye ");
      if(!CheckUserInEEPROM()){
        WriteUserID_C();
      }
      return true;
    } else if (!strcmp(Status_parsed,"No")) {
      Serial.println("No ");
      WriteUserID("NoUser");
      return false;
    }else{
      Serial.println("no status");
      return false;
    }

  //}
//  else if(!parsed.success()){
//      Serial.println("parse fail do something");
//  }
}

bool CheckUserInEEPROM() {
  bool hasUser;

  if (ReadUserID() == "NoUser") {
    hasUser = false;
  } else {
    hasUser = true;
  }

  return hasUser;
}

void make_json() {
//  String Controller_id = ControllerID;
//  ReadControllerID_C();
  //char Controller_id_C[25] = ReadControllerID_C();
//  char Controller_id_c[Controller_id.length];
//  Controller_id.toCharArray(Controller_id_c, Controller_id.length);
//  String User_id = ReadUserID();
//  ReadUserID_C();
//  char User_id_c[User_id.length];
//  User_id.toCharArray(User_id_c, User_id.length);
  //String light = "0";//ReadLight();
//  String light = ReadLight();
//  MakeLight_C();
  //dtostrf(get_LightSensorValue_F(), 8, 0, light_C);
  
//  char light_c[light.length];
//  light.toCharArray(light_c, light.length);
  //String temp = "0";//String(dht.readTemperature());
  
//  String temp = String(dht.readTemperature());
//  Serial.print("read temp ");
//  Serial.println(dht.readTemperature());
//  MakeTemp_C();
  
//  Serial.print("temp_C ");
//  Serial.println(temp_C);  

//  if(temp_C[0] != ' ' && temp_C[1] != ' ' && temp_C[3] != ' ' && temp_C[4] != ' '){
//    Serial.println("temp is not empty");
//  }
  //dtostrf(dht.readTemperature(), 6, 2, temp_C);
  
//  if(String(dht.readTemperature()) != "nan"){
//    dtostrf(dht.readTemperature(), 6, 2, temp_C);
//  }else{
//    strcat(temp_C,zero);
//    //temp_C[0] = '0';
//    //temp_C[1] = '\0';
//  }
//  char temp_c[light.length];
//  temp.toCharArray(temp_c, temp.length);
  //String dirthumid = "0";//ReadDirthumid();
  
//  String dirthumid = ReadDirthumidAnalog();
//  itoa(analogRead(A0),dirthumid_C,10);
//  Serial.print("dirthumid_C ");
//  Serial.println(dirthumid_C);  
  //MakeDirthumid_C();
//  if(dirthumid_C[0] != ' ' && dirthumid_C[1] != ' ' && dirthumid_C[3] != ' ' && dirthumid_C[4] != ' '){
//    Serial.println("dirthumid_C is not empty");
//  }
  //dtostrf(float(analogRead(A0)), 6, 0, dirthumid_C);
  
//  char dirthumid_c[dirthumid.length];
//  dirthumid.toCharArray(dirthumid_c, dirthumid.length);
  //String airhumid = "0";//String(dht.readHumidity());
  
//  String airhumid = String(dht.readHumidity());
//  Serial.print("read humid ");
//  Serial.println(dht.readHumidity());
//  dtostrf(dht.readHumidity(), 6, 2, airhumid_C);
  
//  if(String(dht.readHumidity()) != "nan"){
//    dtostrf(dht.readHumidity(), 6, 2, airhumid_C);
//  }else{
//    strcat(airhumid_C,zero);
//    //json_C[0] = '0';
//    //json_C[1] = '\0';
//  }
  //dtostrf(dht.readTemperature(), 6, 2, airhumid_C);
//  char temp_c[light.length];
//  temp.toCharArray(temp_c, temp.length);
  //strcat(char1,char2) join char concat
  //concat(char1,char2)
//  Serial.print("Controller_id_C ");
//  Serial.println(Controller_id_C);
//  Serial.print(" User_id_C ");
//  Serial.println(User_id_C);
//  Serial.print("light_C ");
//  Serial.println(light_C);
//  Serial.print("temp_C ");
//  Serial.println(temp_C);  
//  itoa(analogRead(A0),dirthumid_C,10);
//  Serial.print("dirthumid_C ");
//  Serial.println(dirthumid_C);  
//  Serial.print("airhumid_C ");
//  Serial.println(airhumid_C);  
//    char char_1[17] = "test ch";
//    char char_2[9] = "ar array";
//    strcat(char_1,char_2);
    //char concat_test = concat(Controller_id_c,User_id_c);

//    Serial.print("strcat_test ");
//    Serial.println(char_1);
//    Serial.print("concat_test ");
//    Serial.println(concat_test);
    
//    Serial.print("light ");
//    Serial.println(light);
//    Serial.print("temp ");
//    Serial.println(temp);
//    Serial.print("dirthumid ");
//    Serial.println(dirthumid);
//    Serial.print("airhumid ");
//    Serial.println(airhumid);

//    if(temp == "nan"){
//      temp = "0";
//    }
//    if(airhumid == "nan"){
//      airhumid = "0";
//    }

//  String SJson =
//    "{"
//    "\"C\":\"" + Controller_id + "\","
//    "\"U\":\"" + User_id + "\","
//    "\"l\":\"" + light + "\","
//    "\"t\":\"" + temp + "\","
//    "\"d\":\"" + dirthumid + "\","
//    "\"a\":\"" + airhumid + "\""
//    "}";
  //char json_C[163];
  for(i = 0;i<163;i++){
    json_C[i] = (char)0;
  }
  
  strcat(json_C,firstpart);
  ReadControllerID_C();
  strcat(json_C,Controller_id_C);
  strcat(json_C,secondpart);
  ReadUserID_C();
  strcat(json_C,User_id_C);
  strcat(json_C,thirdpart);
  MakeLight_C();
  strcat(json_C,light_C);
  strcat(json_C,forthpart);
  MakeTemp_C();
  strcat(json_C,temp_C);
  strcat(json_C,fifthpart);
  //Serial.print("dirthumid_int ");
  //Serial.println(dirthumid_int);
  //toa(analogRead(A0),dirthumid_C,10);
  itoa(dirthumid_int,dirthumid_C,10);
  strcat(json_C,dirthumid_C);
  strcat(json_C,sixthpart);
  //dtostrf(dht.readHumidity(), 5, 2, airhumid_C);
  MakeAirhumid_C();
  strcat(json_C,airhumid_C);
  strcat(json_C,seventhpart);

//  Serial.print("json_C ");
//  Serial.println(json_C);

//  Serial.print("memory ");
//  Serial.println(ESP.getFreeHeap());
  //char json = 

//  Serial.print("Json back ");
//  Serial.println(SJson);
  
  //digitalWrite(D2, LOW);
  //return SJson;
}

String ReadLight() {
  //digitalWrite(D2,LOW);
  //delay(1000);
  //Serial.print("ReadLight ");
  //Serial.println(analogRead(A0));
  return String(get_LightSensorValue());
}

void ReadLight_C() {
//  char b[5];
//  int x = 10;
//  itoa(x,b,10);
//  Serial.print(" B is ");
//  Serial.println(b);
  uint16_t light = get_LightSensorValue();
  itoa(light,light_C,10);
}

void MakeLight_C(){
  light_value =  get_LightSensorValue();
//  if(light_value/10000 > 0){
//    light_C[0] = '0' + (light_value/10000); 
//  }

  for(i = 0;i<5;i++){
    light_C[i] = (char)0;
  }
  
  for(i = 0;i<4;i++){
    for(j=0;j<4;j++){
      //Serial.print("round j");
      //Serial.println(j);
      if(light_C[j] == ' '){ break; }
    }
    for(j;j>=0;j--){
      //Serial.print("round j");
      //Serial.println(j);
      light_C[j+1] = light_C[j]; 
    }
    //light_C[0+i] = light_C[i];
    light_C[0] = '0' + light_value%10;
//    Serial.print("round ");
//    Serial.println(i);
//    Serial.print("light_C ");
//    Serial.println(light_C[i]);
//    Serial.println(light_C[0]);
//    Serial.println(light_C[1]);
//    Serial.println(light_C[2]);
//    Serial.println(light_C[3]);
    light_value = light_value/10;
    if(light_value <= 0){ break; }
  }
}

void MakeTemp_C(){
    temp_C[0] = '0';
  if( String(dht.readTemperature()) != "nan"){
    
    temp_value =  dht.readTemperature()*100;
    //Serial.print("read temp * 10000");
    //Serial.println(dht.readTemperature()*100);
//  if(light_value/10000 > 0){
//    light_C[0] = '0' + (light_value/10000); 
//  }

  for(i = 0;i<5;i++){
    temp_C[i] = ' ';
  }
  
  for(i = 0;i<4;i++){
    for(j=0;j<4;j++){
      //Serial.print("round j");
      //Serial.println(j);
      if(temp_C[j] == ' '){ break; }
    }
    for(j;j>=0;j--){
      //Serial.print("round j");
      //Serial.println(j);
      temp_C[j+1] = temp_C[j]; 
    }
    //light_C[0+i] = light_C[i];
    temp_C[0] = '0' + temp_value%10;
//    Serial.print("round ");
//    Serial.println(i);
//    Serial.print("temp_C ");
//    Serial.println(temp_C[i]);
//    Serial.println(temp_C[0]);
//    Serial.println(temp_C[1]);
//    Serial.println(temp_C[2]);
//    Serial.println(temp_C[3]);
    temp_value = temp_value/10;
    if(temp_value <= 0){ break; }
    }
    temp_C[4] = temp_C[3];
    temp_C[3] = temp_C[2];
    temp_C[2] = '.';    
  }
}

void MakeDirthumid_C(){
    dirthumid_value =  analogRead(A0);
//    Serial.print("dirthumid read ");
//    Serial.println(dirthumid_value);
//  if(light_value/10000 > 0){
//    light_C[0] = '0' + (light_value/10000); 
//  }

  for(i = 0;i<5;i++){
    dirthumid_C[i] = ' ';
  }
  
  for(i = 0;i<4;i++){
    for(j=0;j<4;j++){
      //Serial.print("round j");
      //Serial.println(j);
      if(dirthumid_C[j] == ' '){ break; }
    }
    for(j;j>=0;j--){
      //Serial.print("round j");
      //Serial.println(j);
      dirthumid_C[j+1] = dirthumid_C[j]; 
    }
    //light_C[0+i] = light_C[i];
    dirthumid_C[0] = '0' + dirthumid_value%10;
//    Serial.print("round ");
//    Serial.println(i);
//    Serial.print("dirthumid_C ");
//    Serial.println(dirthumid_C[i]);
//    Serial.println(dirthumid_C[0]);
//    Serial.println(dirthumid_C[1]);
//    Serial.println(dirthumid_C[2]);
//    Serial.println(dirthumid_C[3]);
    dirthumid_value = dirthumid_value/10;
    if(dirthumid_value <= 0){ break; }
  }
    Serial.print("dirthumid_C final ");
    Serial.println(dirthumid_C);
}

void MakeAirhumid_C(){
    airhumid_C[0] = '0';
  if( String(dht.readHumidity()) != "nan"){
    
    airhumid_value =  dht.readHumidity()*100;
    //Serial.print("read temp * 10000");
    //Serial.println(dht.readTemperature()*100);
//  if(light_value/10000 > 0){
//    light_C[0] = '0' + (light_value/10000); 
//  }

  for(i = 0;i<5;i++){
    airhumid_C[i] = ' ';
  }
  
  for(i = 0;i<4;i++){
    for(j=0;j<4;j++){
      //Serial.print("round j");
      //Serial.println(j);
      if(airhumid_C[j] == ' '){ break; }
    }
    for(j;j>=0;j--){
      //Serial.print("round j");
      //Serial.println(j);
      airhumid_C[j+1] = airhumid_C[j]; 
    }
    //light_C[0+i] = light_C[i];
    airhumid_C[0] = '0' + airhumid_value%10;
//    Serial.print("round ");
//    Serial.println(i);
//    Serial.print("temp_C ");
//    Serial.println(temp_C[i]);
//    Serial.println(temp_C[0]);
//    Serial.println(temp_C[1]);
//    Serial.println(temp_C[2]);
//    Serial.println(temp_C[3]);
    airhumid_value = airhumid_value/10;
    if(airhumid_value <= 0){ break; }
    }
    airhumid_C[4] = airhumid_C[3];
    airhumid_C[3] = airhumid_C[2];
    airhumid_C[2] = '.';    
  }  
}

String ReadDirthumid() {
  //digitalWrite(D2,HIGH);
  //delay(1000);
  //Serial.print("ReadDirthumid ");
  //Serial.println(analogRead(A0));
  if (ReadDirthumidEEProm() == "000") {
    //    Serial.print("000 : ");
    //    Serial.println( String(analogRead(A0)) );
    return ReadDirthumidAnalog();
  } else {
    //    Serial.print("not 000 : ");
    //    Serial.println(ReadDirthumidEEProm());
    return ReadDirthumidEEProm();
  }
}

void ReadDirthumid_C() {
  Serial.print("A0 ");
  Serial.println(analogRead(A0));
  Serial.print("dirthumid_C ");
  Serial.println(dirthumid_C);
  int A0 = analogRead(A0);
  itoa(A0,dirthumid_C,10);
}

String ReadDirthumidAnalog() {
  return String(analogRead(A0));
}

void WriteUserID(String UserIDIn) {
  EEPROM.begin(512);
  for (int i = 0; i <= 23; i++) {
    EEPROM.write(i, UserIDIn[i]);
    EEPROM.commit();
  }
  EEPROM.end();
}

void WriteUserID_C() {
  EEPROM.begin(512);
  for (i = 0; i <= 23; i++) {
    EEPROM.write(i, User_id_parsed[i]);
    EEPROM.commit();
  }
  EEPROM.end();
}

void WriteWaterpump(bool waterpump) {
  EEPROM.begin(512);
  EEPROM.write(167, waterpump);
  EEPROM.commit();
  EEPROM.end();
}


void WriteWatertank(bool watertank) {
  EEPROM.begin(512);
  EEPROM.write(168, watertank);
  EEPROM.commit();
  EEPROM.end();
}

//void WritePlot(bool plot) {
//  EEPROM.begin(512);
//  EEPROM.write(169, waterpump);
//  EEPROM.commit();
//  EEPROM.end();
//}

void WriteDirthumid(String dirthumid) {
  EEPROM.begin(512);
//  for (int i = 171; i <= 173; i++) {
//    EEPROM.write(i, dirthumid[i - 171]);
//    EEPROM.commit();
//  }
  for (int i = 255; i <= 257; i++) {
    EEPROM.write(i, dirthumid[i - 255]);
    EEPROM.commit();
  }
  EEPROM.end();
}

void WriteSlan_status(bool slan_status) {
  EEPROM.begin(512);
  EEPROM.write(174, slan_status);
  EEPROM.commit();
  EEPROM.end();
}

void WriteWaterpump_status(bool waterpump_status) {
  EEPROM.begin(512);
  EEPROM.write(175, waterpump_status);
  EEPROM.commit();
  EEPROM.end();
}

bool ReadWaterpump() {
  bool Readmsg;
  EEPROM.begin(512);
  Readmsg = EEPROM.read(167);
  EEPROM.end();
  return Readmsg;
}

bool ReadWatertank() {
  bool Readmsg;
  EEPROM.begin(512);
  Readmsg = EEPROM.read(168);
  EEPROM.end();
  return Readmsg;
}

bool ReadPlot() {
  bool Readmsg ;
  EEPROM.begin(512);
  Readmsg = EEPROM.read(169);
  EEPROM.end();
  return Readmsg;
}

String ReadDirthumidEEProm() {
  String Readmsg = "";
  EEPROM.begin(512);
//  for (int i = 171; i <= 173; i++) {
//    char x = EEPROM.read(i);
//    Readmsg += x;
//  }
  for (int i = 255; i <= 257; i++) {
    char x = EEPROM.read(i);
    Readmsg += x;
  }
  EEPROM.end();
  return Readmsg;
}

bool ReadSlan_status() {
  bool Readmsg ;
  EEPROM.begin(512);
  Readmsg = EEPROM.read(174);
  EEPROM.end();
  return Readmsg;
}

bool ReadWaterpump_status() {
  bool Readmsg ;
  EEPROM.begin(512);
  Readmsg = EEPROM.read(175);
  EEPROM.end();
  return Readmsg;
}

String ReadUserID() {
  String Readmsg = "";
  EEPROM.begin(512);
  for (int i = 0; i <= 23; i++) {
    char x = EEPROM.read(i);
    Readmsg += x;
  }
  EEPROM.end();
  return Readmsg;
}

void ReadUserID_C() {
  //char Readmsg[25];
  EEPROM.begin(512);
  for (int i = 0; i <= 23; i++) {
    char x = EEPROM.read(i);
    User_id_C[i] = EEPROM.read(i);
  }
  EEPROM.end();
  //return Readmsg;
}

String ReadControllerID() {
  String Readmsg = "";
  EEPROM.begin(512);
  for (int i = 48; i <= 71; i++) {
    char x = EEPROM.read(i);
    Readmsg += x;
  }
  EEPROM.end();
  return Readmsg;
}

void ReadControllerID_C() {
  //char Readmsg[25];
  EEPROM.begin(512);
  for (int i = 48; i <= 71; i++) {
    //char x = EEPROM.read(i);
    Controller_id_C[i-48] = EEPROM.read(i);
  }
  EEPROM.end();
  //return Readmsg;
}

String ReadMqttTopic() {
  String Readmsg = "";
  EEPROM.begin(512);
  for (int i = 72; i <= 87; i++) {
    char x = EEPROM.read(i);
    Readmsg += x;
  }
  EEPROM.end();
  return Readmsg;
}

//light sensor
uint16_t get_LightSensorValue()
{
  int i;
  uint16_t val = 0;
  BH1750_Init(BH1750address);
  delay(150);
  Serial.println(BH1750_Read(BH1750address));
  if (2 == BH1750_Read(BH1750address))
  {
    val = ((buff[0] << 8) | buff[1]) / 1.2;
    Serial.print(val, DEC);
    Serial.println(" lux");
    return val;
  }
}

float get_LightSensorValue_F()
{
  int i;
  //uint16_t val = 0;
  float val = 0;
  BH1750_Init(BH1750address);
  delay(150);
  Serial.println(BH1750_Read(BH1750address));
  if (2 == BH1750_Read(BH1750address))
  {
    val = ((buff[0] << 8) | buff[1]) / 1.2;
    Serial.print(val, DEC);
    Serial.println(" lux");
    return val;
  }
}

int BH1750_Read(int address)
{
  int i = 0;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, 2);
  while (Wire.available()) //
  {
    buff[i] = Wire.read();  // receive one byte
    i++;
  }
  Wire.endTransmission();
  return i;
}

void BH1750_Init(int address)
{
  Wire.beginTransmission(address);
  Wire.write(0x10);//1lx reolution 120ms
  Wire.endTransmission();
}
//end light sensor
