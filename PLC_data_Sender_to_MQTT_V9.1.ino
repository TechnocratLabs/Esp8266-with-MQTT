#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "NodeMCU"
#define WLAN_PASS       "12345678"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "Technocrat"
#define AIO_KEY         "eb850dfb54694790b4645e0b492e3d17"

/**************************Variable***********************************/
char Message[100];
char AFMessage[82];
char graph_data[10];
char AFgraph_data[8];
int messageLength = 0;
int Graph_messageLength = 0;
int i, j, k, l;
int flag = 0;
int flag1 = 0;
/**************************INPUT&OUTPUT DIO***********************************/

int Stop = 5;
/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish SerialData = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/SerialData");
Adafruit_MQTT_Publish graph = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/graph");
Adafruit_MQTT_Publish Machine_State = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Machine_State");

/*************************** Sketch Code ************************************/
void MQTT_connect();

void setup() {
  Serial.begin(38400);
  delay(10);

  //  Serial.println(F("Adafruit MQTT data send"));

  // Connect to WiFi access point.
  //  Serial.println(); Serial.println();
  //  Serial.print("Connecting to ");
  //  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //    Serial.print(".");
  }
  //  Serial.println();
  //
  //  Serial.println("WiFi connected");
  //  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  pinMode(Stop, INPUT);
}
//char message[] = ("~1Encon System International ITC Palletizer Production rate 100% ");

/************************* Program in loop *********************************/
void loop() {
  MQTT_connect();
  delay(10);
  
  bool Stp = digitalRead(Stop);


  if (Stp == 0) {
    if (flag1 == 0) {
      Serial.println(Stp);
      if (! Machine_State.publish(Stp)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
        flag1 = 1;
      }
    }

    if (flag == 0) {
      if (Serial.read() == '$') {
        for (i = 0; i < 100; i++) {
          Message[i] = Serial.read();
          delay(2);
          //      Serial.println(i);
          if (Message[i] == '#') {
            //        Serial.println();
            //        Serial.println("break");
            messageLength = i;
            break;
          }
        }
        Serial.println();
        Serial.println(messageLength);

        //    Serial.println();
        //    Serial.println("The incoming message is:");
        /*    for (i = 0; i < messageLength; i++)
            {
              Serial.print((Message[i]));
              delay(2);
            }                                                                   */
        for ( k = 0; k < messageLength; k++) {
          AFMessage[k] = Message[k];
          delay(2);
        }
        Serial.println();
        //    Serial.println("next data");
        Serial.println("DMD data");
        Serial.println(AFMessage);
        //    Serial.println("Main line is END");

        if (! SerialData.publish(AFMessage)) {
          Serial.println(F("Failed"));
        }
        else {
          Serial.println(F("OK!"));
          flag = 1;
        }
      }
    }

    if (flag == 1) {
      if (Serial.read() == '+')
      {
        for (j = 0; j < 100; j++)
        {
          graph_data[j] = Serial.read();
          delay(2);
          //      Serial.println(j);
          if (graph_data[j] == '#')
          {
            //        Serial.println();
            //        Serial.println("break");
            Graph_messageLength = j;
            break;
          }
        }
        //    Serial.println();
        //    Serial.println("The incoming message is:");
        /*    for (i = 0; i < messageLength; i++)
            {
              Serial.print((Message[i]));
              delay(2);
            }                                                                   */
        for ( l = 0; l < Graph_messageLength; l++) {
          if (graph_data[l] == '⸮') {
            AFgraph_data[l] = graph_data[l + 1];
            delay(2);
          }
          else {
            AFgraph_data[l] = graph_data[l];
            delay(2);
          }
        }
        Serial.println();
        //    Serial.println("next data");
        Serial.println("Graph data");
        Serial.println(AFgraph_data);
        //    Serial.println("Main line is END");

        if (! graph.publish(AFgraph_data)) {
          Serial.println(F("Failed"));
        } else {
          Serial.println(F("OK!"));
          flag = 0;
        }
      }
    }
  }
  else {
    if (flag1 == 1) {
      //char Stop_SMS[] = {"Machine NO. ESI/007/EER/01"};
      Serial.println(Stp);
      if (! Machine_State.publish(Stp)) {
        Serial.println(F("Failed"));
      } else {
        Serial.println(F("OK!"));
        flag1 = 0;
      }
    }
  }
  Serial.flush();
}


void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  //  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    //    Serial.println(mqtt.connectErrorString(ret));
    //    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  //  Serial.println("MQTT Connected!");
}
