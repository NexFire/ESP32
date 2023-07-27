#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <HTTPClient.h>

#include "web_page.h"

#define GREEN_LED 32
#define BLUE_LED 33
#define RED_LED 25
#define SLEEP 34

//global variables
Preferences preferences;
WiFiServer server(80);
bool configServerRunning=false;
int leds[]={GREEN_LED,RED_LED,BLUE_LED};
bool pressedSleep=false;
StaticJsonDocument<1024> jsonDoc;
HTTPClient http;

//here are function declarations
bool last_wifi_connection();
void test_wifi();
void config();
void led_test();
void led_reload();
void wifi_setup();
void input_test();
String get_version();
void update_firmware(String);

//setting up the ports and other shit
void setup() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(SLEEP,INPUT_PULLUP);
  Serial.begin(115200);
  delay(5000);
  //delay(1000);
  led_reload();
  led_test();
  led_reload();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_34,1);
  digitalWrite(BLUE_LED,HIGH);

  Serial.println("Test 1");
  wifi_setup();
  if(configServerRunning!=true){
    update_firmware(get_version());
  }
  Serial.println("This is version 2");
}
int counter=0;
int timer=0;
int myCounter=0;
//here is running loop
void loop() {
  input_test();
  if(configServerRunning==true){
    config();
  }

  if(WiFi.status()==WL_CONNECTED){
    digitalWrite(GREEN_LED,HIGH);
  }
  else{
    digitalWrite(GREEN_LED,LOW);
    digitalWrite(RED_LED,HIGH);
  }
  //Serial.println("loop");
  //led_test();

}


void wifi_setup(){
  WiFi.mode(WIFI_AP_STA);
  Serial.println("Starting the connecting procedure.");
  bool is_connected=last_wifi_connection();
  if(!is_connected){
    Serial.println("Setting config AP");
    WiFi.softAP("DEVICE", "DEVICEDEVICE");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    server.begin();
    configServerRunning=true;
    led_reload();
    digitalWrite(RED_LED,HIGH);
  }
}
void led_reload(){
  for(int a=0;a<3;a++){
    digitalWrite(leds[a],LOW);
  }
}
void led_test(){
  for (int a=0;a<3;a++){
    delay(100);
    if(a!=0){
      digitalWrite(leds[a-1],LOW);
    }
    else{
      digitalWrite(leds[2],LOW);
    }
    digitalWrite(leds[a],HIGH);
    delay(100);
  }
}

void config(){
    WiFiClient client = server.available();
    String header;
    if(client){
      Serial.println("New Client");
      String currentLine = "";
      while(client.connected()){
        if(client.available()){
          char c = client.read();             // read a byte, then
          //Serial.write(c);                    // print it out the serial monitor
          header += c;
          if(c=='\n'){
            if(currentLine.length()==0){ //two \n at the end is end of the header
              
              //checking if already submited
              if(header.indexOf("ssid=")>0){
                String ssid=header.substring(header.indexOf("ssid=")+5,header.indexOf("&passwd"));
                String passwd=header.substring(header.indexOf("passwd=")+7,header.indexOf(" HTTP/1.1"));
                Serial.println(ssid+"::"+passwd);
                client.println(configDonePage);
                int n = WiFi.scanNetworks();
                if(n==0){
                  Serial.println("We have not WIFI here no point to continue, exiting and creating access point");
                }
                else{
                  Serial.print(n);
                  Serial.println(" networks found, now will search for the last one we connect to and will try to connect to it");
                  for (int i = 0; i < n; ++i) {
                  // Print SSID and RSSI for each network found
                    if(ssid==WiFi.SSID(i)){
                        WiFi.disconnect();
                        WiFi.begin(ssid, passwd);
                        Serial.print("Connecting to WiFi ..");
                        timer=0;
                        while (WiFi.status() != WL_CONNECTED) {
                          timer++;
                          //Serial.println(WiFi.status()+"::"+WL_CONNECTED);
                          Serial.print('.');
                          if(timer>=10){
                            ESP.restart();
                          }
                          delay(1000);
                        }
                        Serial.println("Connected");
                        preferences.begin("lastWifi", false);
                        preferences.putString("ssid",ssid);
                        preferences.putString("passwd",passwd);
                        preferences.end();
                        return;
                    }
                    delay(10);
                  }
                }
                client.println();

              }
              else{
                //printing standard shit
                client.println("HTTP/1.1 200 OK");
                client.println("Content-type:text/html");
                client.println("Connection: close");
                client.println();
                //end of the standard shit
                //here displaying config screen
                String webString=web;
                String start=webString.substring(0,webString.indexOf("<options>"));
                String options="";
                int n = WiFi.scanNetworks();
                if(n==0){
                  Serial.println("We have not WIFI here no point to continue, exiting and creating access point");
                }
                else{
                  Serial.print(n);
                  Serial.println(" networks found, now will search for the last one we connect to and will try to connect to it");
                  for (int i = 0; i < n; ++i) {
                  // Print SSID and RSSI for each network found
                      options+="<option value= \""+WiFi.SSID(i)+"\" >"+WiFi.SSID(i)+"</option>";
                    }
                    delay(10);
                  }
                String end=webString.substring(webString.indexOf("</select>"),webString.length());
                client.println(start+options+end);
                client.println();
                //end of displaying config file
                break;
              }
            }
            else{
              currentLine="";
            }
          }
          else if(c!='\r'){
            currentLine+=c;
          }
        }
      }
      client.stop();
      Serial.println("Client disconnected.");
      Serial.println("");
    }
}

//this function makes the config AP and web server
//basic ssid is "DEVICE"
//basic passwd is "DEVICE"
bool last_wifi_connection(){
  bool is_wifi=false;
  preferences.begin("lastWifi", false);
  String ssid=preferences.getString("ssid","_");
  String passwd=preferences.getString("passwd","");
  Serial.println("Password: "+passwd);
  Serial.println("SSID: "+ssid);
  if(passwd.length()>=63){
    return false;
  }
  WiFi.disconnect();
  if(ssid!="_" && passwd!=""){
    Serial.println("Dissconnected from the networks now searching for the last connected WIFI");
    int n = WiFi.scanNetworks();
    if(n==0){
      Serial.println("We have not WIFI here no point to continue, exiting and creating access point");
      return false;
    }
    else{
      Serial.print(n);
      Serial.println(" networks found, now will search for the last one we connect to and will try to connect to it");
      Serial.println("Password: "+passwd);
      Serial.println("SSID: "+ssid);
      for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
        if(ssid==WiFi.SSID(i)){
            WiFi.begin(ssid, passwd);
            Serial.print("Connecting to WiFi ..");
            while (WiFi.status() != WL_CONNECTED) {
              if(timer>=10){
                timer=0;
                ESP.restart();
              }
              Serial.print('.');
              timer++;
              delay(1000);
            }
            Serial.println("Connected");
            preferences.end();
            return true;
        }
        //delay(10);
      }
    }
    preferences.end();
    return false;
  }
  else{
    preferences.end();
    return false;
  }

}
void test_wifi(){
  String ssid="O2-Internet-368";
  String passwd="GUHAJSP8Yj";
  Serial.println("Starting connecting procedure");
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  int n = WiFi.scanNetworks();
    if(n==0){
      Serial.println("We have not WIFI here no point to continue, exiting and creating access point");
    }
    else{
      Serial.print(n);
      Serial.println(" networks found, now will search for the last one we connect to and will try to connect to it");
      for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
        if(ssid==WiFi.SSID(i)){
            WiFi.begin(ssid, passwd);
            Serial.print("Connecting to WiFi ..");
            while (WiFi.status() != WL_CONNECTED) {
              if(timer>=10){
                timer=0;
                ESP.restart();
              }
              Serial.print('.');
              timer++;
              delay(1000);
            }
            Serial.println("Connected");
            Serial.println("We had: "+String(timer)+" loops");
        }
        delay(10);
      }
    }
}

void input_test(){
  if(digitalRead(SLEEP)==HIGH && pressedSleep==false){
    //myCounter++;
    //Serial.println("Now pressed: "+String(myCounter));
    Serial.println("Going to sleep now");
    delay(1000);
    esp_deep_sleep_start();
    pressedSleep=true;
  }
}

String get_version(){
  String url = "https://otaupdates-129df-default-rtdb.europe-west1.firebasedatabase.app/version.json";
  http.setTimeout(1000);
  http.begin(url);
  int status = http.GET();
  if (status <= 0) {
   Serial.printf("HTTP error: %s\n", http.errorToString(status).c_str());
   return "";
  }
  String payload = http.getString();
  return payload.substring(1,payload.length()-1);
}

void update_firmware(String version){
  preferences.begin("version", false);
  String currentVersion=preferences.getString("version","");
  if(version!=currentVersion){
    String url = "https://otaupdates-129df-default-rtdb.europe-west1.firebasedatabase.app/firmwares/firmware_"+version+"/url.json";
    Serial.println(url);
    http.setTimeout(1000);
    http.begin(url);
    int status = http.GET();
    if (status <= 0) {
    Serial.printf("HTTP error: %s\n", http.errorToString(status).c_str());
    return;
    }
    String payload = http.getString();
    url=payload.substring(1,payload.length()-1);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode <= 0) {
      Serial.printf("HTTP failed, error: %s\n", 
        http.errorToString(httpCode).c_str());
      return;
    }
    int contentLen = http.getSize();
    Serial.printf("Content-Length: %d\n", contentLen);
    bool canBegin = Update.begin(contentLen);
    if (!canBegin) {
      Serial.println("Not enough space to begin OTA");
      return;
    }
    WiFiClient* client = http.getStreamPtr();
    size_t written = Update.writeStream(*client);
    Serial.printf("OTA: %d/%d bytes written.\n", written, contentLen);
    if (written != contentLen) {
      Serial.println("Wrote partial binary. Giving up.");
      return;
    }
    if (!Update.end()) {
      Serial.println("Error from Update.end(): " + 
        String(Update.getError()));
      return;
    }
    if (Update.isFinished()) {
      Serial.println("Update successfully completed. Rebooting."); 
      preferences.putString("version",version);
      // This line is specific to the ESP32 platform:
      ESP.restart();
    } else {
      Serial.println("Error from Update.isFinished(): " + 
        String(Update.getError()));
      return;
    }
  }
  preferences.end();
}