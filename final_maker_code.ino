
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"
/************************* WiFi Access Point *********************************/
#define WLAN_SSID       "OnePlus5"
#define WLAN_PASS       "Aaditya@123"

/************************* Adafruit.io Setup *********************************/
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "kabir_123"
#define AIO_KEY         "aio_EYGH52z1EjX1VX7WLr8pF7tpbTaU"

/************* DHT11 Setup ********************************************/
#define DHTPIN 12 //D1
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/************ Global State (you don't need to change this!) ******************/
// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;
// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");

/*************************** Sketch Code ************************************/
const int tem = A0, p_hot = 14, p_cold = 4;
float temp;
int high = 99;
int start_t = millis(), end_t, time_m;

void setup() 
{
  Serial.begin(115200);
  delay(10);
  dht.begin();
  
  // Connect to WiFi access point.
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(p_cold, OUTPUT);
  pinMode(p_hot, OUTPUT);
  pinMode(tem, INPUT);
}

void loop() 
{
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  end_t = millis();
  time_m = end_t - start_t;
  
  if (time_m>60000)
  {
    temperature.publish(temp);
    start_t = millis();
  }
  
  temp = dht.readTemperature();
  
  high = map(high, 0, 99, 0, 1023);

  Serial.print(F("\nTemperature: "));
  Serial.print(temp);

  if (temp<2.5)
  {
    analogWrite(p_hot, high); 
    Serial.println("P_cold on");
  }
  else if (temp>7.5)
  {
    

    analogWrite(p_cold, high);
    Serial.println("P_hot on");
  } 

  delay(1000);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
