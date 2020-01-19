#include <ESP8266WiFi.h>
#include <Wire.h>
//#include <Time.h>

#include <SHTSensor.h>
#include <Adafruit_VEML7700.h>
#include <PubSubClient.h>



#define YL69PIN   A0
#define WATERPIN  D7
#define RELAYPIN  12 /* D6 */

#define TBTOKEN "q4M2LoLE5GeWvSvsuFj9"

const char *ssid = "SBC";
const char *pass = "sbc$18-maceta";
char ThingsboardHost[] = "demo.thingsboard.io";


WiFiClient wc;
PubSubClient client(wc);
int status = WL_IDLE_STATUS;

SHTSensor sht;
Adafruit_VEML7700 veml = Adafruit_VEML7700();



/* Funciones de inicializacion */
void connectwifi(const char *, const char *);
void shtinit();
void veml7700init();
void yl69init();
void waterinit();

/* Read from sensors */
float sht85readhum();
float sht85readtemp();
float yl69read();
int   waterread();
int   veml7700readLux();

/* Change things */
void relaystate(int);
void reconnect();


void setup() {
  Serial.begin(115200);
  Wire.begin();

//Inicializa el reloj de la placa
  //setTime(19,00,00,20,1,2020);

  shtinit();
  veml7700init();
  yl69init();
  waterinit();

  pinMode(RELAYPIN, OUTPUT); // Relay pin -> D6
  connectwifi(ssid, pass);
  client.setServer(ThingsboardHost, 1883);
}



void loop() {
  if (!client.connected())
    reconnect();

  char attributes[1000];
    
  float humSuelo    = yl69read();
  float humAmbiente = sht85readhum();
  float temAmbiente = sht85readtemp();
  int   lux         = veml7700readLux();
  int   water       = waterread();
  
/*
  time_t t = now();
// Mostrar el reloj por pantalla
   Serial.print(day(t));
   Serial.print(+ "/") ;
   Serial.print(month(t));
   Serial.print(+ "/") ;
   Serial.print(year(t)); 
   Serial.print( " ") ;
   Serial.print(hour(t));  
   Serial.print(+ ":") ;
   Serial.print(minute(t));
   Serial.print(":") ;
   Serial.println(second(t));
   delay(1000);
//Si son las nueve de la mañana activa la electrovalvula
   if (hour(t)==9&&minute(t)==0&&second(t)==0){
    relaystate(1);
   }

*/ 

  Serial.print("Humedad del suelo: ");
  Serial.println(humSuelo);

  String humedadSuelo = "{";
  humedadSuelo += "\"Humedad suelo\":";
  humedadSuelo += humSuelo;
  humedadSuelo += "}";
  humedadSuelo.toCharArray( attributes, 1000 );

  client.publish( "v1/devices/me/telemetry",attributes);
  

  Serial.print("Humedad ambiente: ");
  Serial.println(humAmbiente);

  String humedadAmbiente = "{";
  humedadAmbiente += "\"Humedad ambiente\":";
  humedadAmbiente += humAmbiente;
  humedadAmbiente += "}";
  humedadAmbiente.toCharArray( attributes, 1000 );

  client.publish( "v1/devices/me/telemetry",attributes);

  /*
//Si la humedad es alta o baja para la planta se muestra un mensaje por pantalla
  if (humAmbiente<50){
    String pocaHum = "El hambiente no es lo suficientemente húmedo para la planta";
    pocaHum.toCharArray( attributes, 1000 );
    client.publish( "v1/devices/me/telemetry",attributes);
    }

  if (humAmbiente>70){
    String muchaHum = "El hambiente es demasiado húmedo para la planta";
    muchaHum.toCharArray( attributes, 1000 );
    client.publish( "v1/devices/me/telemetry",attributes);
    }
*/

  Serial.print("Temperatura ambiente: ");
  Serial.println(temAmbiente);

  String temperatura = "{";
  temperatura += "\"Temperatura\":";
  temperatura += temAmbiente;
  temperatura += "}";
  temperatura.toCharArray( attributes, 1000 );

  client.publish( "v1/devices/me/telemetry",attributes);

  /*
//Si la temperatura es alta o baja para la planta se muestra un mensaje por pantalla  
  if (temAmbiente<15){
    String bajaTemp = "La temperatura es muy baja para la planta";
    bajaTemp.toCharArray( attributes, 1000 );
    if (temAmbiente<10){
      String muyBajaTemp = "¡La planta puede morir!";
      muyBajaTemp.toCharArray( attributes, 1000 );
    }
    client.publish( "v1/devices/me/telemetry",attributes);
    }

  if (temAmbiente>25){
    String altaTemp = "La temperatura es muy alta para la planta";
    altaTemp.toCharArray( attributes, 1000 );
    client.publish( "v1/devices/me/telemetry",attributes);
    }
*/

  Serial.print("Lux: ");
  Serial.println(lux);

  String luz = "{";
  luz += "\"Luz\":";
  luz += lux;
  luz += "}";
  luz.toCharArray( attributes, 1000 );

  client.publish( "v1/devices/me/telemetry",attributes);

/*
// Si la planta recibe poca o mucha luz muestra un mensaje por pantalla
  if (lux<8000){
    String pocaLuz = "No le llega suficiente luz a la planta";
    pocaLuz.toCharArray( attributes, 1000 );
    client.publish( "v1/devices/me/telemetry",attributes);
    }

  if (lux>10000){
    String muchaLuz = "A la planta llega demasiada luz, puede ser perjudicial";
    muchaLuz.toCharArray( attributes, 1000 );
    client.publish( "v1/devices/me/telemetry",attributes);
    }
*/

  Serial.print("Water: ");
  Serial.println(water);

  String agua = "{";
  agua += "\"Agua\":";
  agua += water;
  agua += "}";
  agua.toCharArray( attributes, 1000 );
  
  delay(1000);

  client.publish( "v1/devices/me/telemetry",attributes);

/*
//Si el sensor detecta agua y esta la electrovalvula encendica la apaga
  if(waterread()==1&&digitalRead(RELAYPIN) == HIGH){
    relaystate(0);
    }
*/
  
  Serial.println( attributes );
}


/**************************************************************************
 * * * * * * * * * * * * * * * INITIALIZATION * * * * * * * * * * * * * * * 
 **************************************************************************/
void connectwifi(const char *ssid, const char *pass) {
  WiFi.begin(ssid, pass);
  Serial.print("[*] Connecting to: ");
  Serial.println(WiFi.SSID());

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("[+] Connection established");
  Serial.println(WiFi.localIP());
}


void shtinit() { /* SHT85 functiona por I2C */
  sht.init();
  Serial.println("[*] SHT85 inicializado");
}


void veml7700init() { /* VEML7700 funciona por I2C. */
  veml.begin();
  veml.setGain(VEML7700_GAIN_1_8);
  veml.setIntegrationTime(VEML7700_IT_800MS);

  veml.setLowThreshold (10000);
  veml.setHighThreshold(20000);
  veml.interruptEnable(false);
  Serial.println("[*] VEML7700 inicializado");
}


void yl69init() {
  pinMode(YL69PIN, INPUT);
}


void waterinit() {
  pinMode(WATERPIN, INPUT);
}


/**************************************************************************
 * * * * * * * * * * * * * * READ FROM SENSORS * * * * * * * * * * * * * * 
 **************************************************************************/
float sht85readhum() {
  sht.readSample();
  return sht.getHumidity();
}


float sht85readtemp() {
  sht.readSample();
  return sht.getTemperature();
}


float yl69read() {
  return analogRead(A0);
}


int veml7700readLux() {
  return veml.readLux();
}


int waterread() {
    int waterstate = 0;
    if (digitalRead(WATERPIN) == LOW)
        waterstate = 1;
    return waterstate; 
}


/**************************************************************************
 * * * * * * * * * * * * * * CHANGE CONFIGURATION * * * * * * * * * * * * * 
 **************************************************************************/
void relaystate(int state) {
  if (state != 0 || state != 1)
    return;
  
  Serial.print("[+] Estado del relé cambiado a ");
  state == 1 ? Serial.println("on") : Serial.println("off");

  digitalWrite(RELAYPIN, state);
}


void reconnect() {
  while (!client.connected()) {
    status = WiFi.status();
    if (status != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      Serial.println("\n[+] Connected to AP");
    }
    Serial.println("[*] Connecting to Thingsboard node...");
    if (client.connect("ESP8266", TBTOKEN, NULL)) {
      Serial.println("[+] Done");
    }
    else {
      Serial.println("[!] Failed. Retrying.");
      delay(500);
    }
  }
}


 /* 0  -> D3
    1  -> D10
    2  -> D4
    3  -> D9
    4  -> D2
    5  -> D1
    12 -> D6
    13 -> D7
    14 -> D5
    15 -> D8
    16 -> D0 */