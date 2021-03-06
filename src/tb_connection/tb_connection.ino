

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "POWER_UP";
const char* password = "RosquillaGl4s3ada!";

#define TOKEN1 "o5RuPZnyKIPz83F5IppJ" //Access token of device Display
#define TOKEN2 "HtsQJrRn80mXL4YHJp6a" //Access token of device Display
#define WATER_SENSOR D8
char ThingsboardHost[] = "demo.thingsboard.io";

WiFiClient wifiClient;
PubSubClient client(wifiClient);
int status = WL_IDLE_STATUS;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("connected to");
  Serial.println(ssid);
  client.setServer( ThingsboardHost, 1883 );
  pinMode(A0, INPUT); //YL69
  pinMode(WATER_SENSOR, INPUT); //Water_sensor 
}

void loop()
{
if ( !client.connected() ) 
{
    reconnect();
}
getAndSendTemperatureAndHumidityData();
  delay(5000);
  isExposedToWater();
  delay(5000);
}

void getAndSendTemperatureAndHumidityData()
{
  
 int hum = analogRead(A0);
  // Prepare a JSON payload string
  String humeda = "{";
 humeda += "\"Humidity\":";humeda += hum;
  humeda += "}";

  char attributes[1000];
  humeda.toCharArray( attributes, 1000 );
  client.publish( "v1/devices/me/telemetry",attributes);
  Serial.println( attributes );
   
}

void isExposedToWater()
{
  int water = digitalRead(WATER_SENSOR);
  String payload = "{";
  payload += "\"Agua\":";payload += water;
  payload += "}";

  char attributes[1000];
  payload.toCharArray( attributes, 1000 );
  client.publish( "v1/devices/me/telemetry",attributes);
  Serial.println( attributes );
  
  
 }

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    status = WiFi.status();
    if ( status != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("Esp8266", TOKEN1, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.println( " : retrying in 5 seconds]" );
      delay( 500 );
    }

      if ( client.connect("Esp8266", TOKEN2, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.println( " : retrying in 5 seconds]" );
      delay( 500 );
    }
  }
}
