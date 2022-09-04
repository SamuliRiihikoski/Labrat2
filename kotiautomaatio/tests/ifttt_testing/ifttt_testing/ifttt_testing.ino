#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "AndroidAP";
const char* password = "Park2771Uruk";
const char* serverName = "http://maker.ifttt.com/trigger/tynnyri_tila/with/key/Lls0N0vNk00GTBStNdqBf";

void setup() 
{  
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wifi connected");



}

void loop() 
{
  unsigned long currentSeconds = ( millis() / 1000 );
  Serial.println(currentSeconds);
  WiFiClient client;
  HTTPClient http;

  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  String body;
  body += "Lämmityksen aloituksesta: " + systemClockStr();
  body += "<br>Veden lämpötila: " + String( getTempWater1() );
  
  String httpRequestData = "value1=" + body;
  int httpResponseCode = http.POST(httpRequestData); 

  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  
  // Free resources
  http.end();

  getTempWater1();
  delay(10000);
}

String systemClockStr() 
{
  char str[90];
  unsigned long currentSeconds = ( millis() / 1000 );
  unsigned int sec = currentSeconds % 60;
  unsigned int minutes = ( currentSeconds % 3600 ) / 60;
  unsigned int hours = currentSeconds / 3600;
  sprintf(str, "%02u:%02u:%02u", hours, minutes, sec);
  return String(str);
}

float getTempWater1()
{
  const int B = 4275;
  const int R0 = 100000;

  int a = analogRead(A0);
  float R = 4095.0/a - 1.0;
  R = R0*R;

  float temperature = 1.0/(log(R/R0)/B+1/298.15) - 273.15;
  Serial.print("temperature: ");
  Serial.println(temperature);

  return temperature;
}
