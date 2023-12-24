#include <WiFiClientSecure.h>
#include <esp_crt_bundle.h>
#include <ssl_client.h>
#include "base64.h"  // Libreria per la conversione in base64
#include <SPIFFS.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h> //webserver per debug
#include "esp_camera.h" //gestire la foto
#include "soc/soc.h"           //Disable brownour problems
#include "soc/rtc_cntl_reg.h"  //Disable brownour problems

#include "WiFiClientSecure.h" //per mettere in sicurezza il wifi(capire)
#include <PubSubClient.h> //import for mqtt protocol

#define LED 33 //Led Built-in sotto la scheda

//mqtt var
const char* mqtt_server = "192.168.81.216";
const int mqtt_port = 1883;//no tls prova
const char* mqtt_user = "ciao";
const char* mqtt_pass = "ciao";
const char* mqtt_topic = "mq2_mqtt"; //routing key

WiFiClientSecure espClient;
PubSubClient client(espClient);

AsyncWebServer server(80);

const char* ssid = "Silvio";          // Your WiFi SSID
const char* password ="12345678";
const int pinSensoreMagnetico = 2; //Sensore magnetica invia i dati sul pin 2 di esp32

//Certificate file for TLS
const char* cert;
const char* private_key;
const char* ca_cert;

void setupCamera();
void startCamera();
camera_fb_t * takePicture(bool flash);

//Wifi manager
void writePasswd(const char* plainPasswd);
String getWiFiPasswd();

void setup_wifi() {
  // Connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}



void recvMsg(uint8_t *data, size_t len){
  Serial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);
  if (d == "ON"){
    digitalWrite(LED, LOW);
  }
  if (d=="OFF"){
    digitalWrite(LED, HIGH);
  }
}

void setup() {
  Serial.begin(115200);
  //setup wifi
  setup_wifi();

  //authentication
  espClient.setCACert(ca_cert);
  espClient.setCertificate(cert);
  espClient.setPrivateKey(private_key);
  
  //setup led
  pinMode(LED, OUTPUT);
  pinMode(pinSensoreMagnetico, INPUT);

  //Camera setup
  setupCamera();
  startCamera();

  //setting mqtt server and port
  client.setServer(mqtt_server,mqtt_port);

  // WebSerial is accessible at "<IP Address>/webserial" in browser
  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  server.begin();
}

void RabbitConnection(){
  //caricare i certificati
  while(!client.connected()){
    Serial.println("Attempting MQTT connection");

    if(client.connect("ClientEsp",mqtt_user,mqtt_pass)){//credenziali dell'utente sul server
      Serial.println("connected");
  }
    else{
      Serial.println("failed rc= ");
      Serial.print(client.state());
      Serial.println("Try again in 5 seconds");
      delay(5000);
    }
  }
}

void send_jpeg_fragment(const char* fragmentTopic, const uint8_t* jpegData, size_t dataSize) {
  if (client.publish(fragmentTopic, jpegData, dataSize)) {
    Serial.println("JPEG fragment published successfully");
  } else {
    Serial.print("Failed to publish JPEG fragment: message too large or connection rc=");
    Serial.println(client.state());
  }
}

void send_jpeg_image(const char* imageTopic) {
  camera_fb_t *fb = takePicture(true);
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  const size_t maxFragmentSize = 128;  // Dimensione massima del frammento in byte (rabbitmq di default ha max size 256 byte per messaggio mqtt)
  size_t remainingBytes = fb->len;
  const uint8_t* imageData = fb->buf;
  size_t NumOfFragment = (int) remainingBytes/maxFragmentSize;
  client.publish((String(mqtt_topic) + "/total_fragments").c_str(), String(NumOfFragment).c_str());
  Serial.println(NumOfFragment);

  size_t fragmentIndex = 0;

  while (remainingBytes > 0) {
    size_t fragmentSize = std::min(remainingBytes, maxFragmentSize);
    Serial.println(fragmentSize);
    const uint8_t* fragmentData = imageData + (fragmentIndex * maxFragmentSize);

    // Invia il frammento corrente
    //const char* topicFragName = (imageTopic + String("/fragment/") + String(fragmentIndex)).c_str();
    send_jpeg_fragment(mqtt_topic, fragmentData, fragmentSize);
    remainingBytes -= fragmentSize;
    fragmentIndex++;
    Serial.println(fragmentIndex);
    //delay(2000);
  }
  client.publish(mqtt_topic,"END");
  Serial.println("End message published successfully");

  // Rilascia la risorsa della fotocamera
  esp_camera_fb_return(fb);
  client.endPublish();
  client.disconnect();
}

void loop() {
  if (!client.connected()) {
    RabbitConnection();
  }
  client.loop();

  int statoSensore = digitalRead(pinSensoreMagnetico);

  if (statoSensore == HIGH) { //sensore non rileva il magnete -> fai foto dopo tot sec
    digitalWrite (LED, HIGH);
    Serial.println("Campo magnetico non  rilevato!");
    delay(5000);
    //Scattare foto
    send_jpeg_image(mqtt_topic);
    delay(10000);  // Attendere 10 secondi tra gli invii
  }
  else {
    digitalWrite (LED, LOW);
    Serial.println("Campo magnetico rilevato.");
  }
  
}




