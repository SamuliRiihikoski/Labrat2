#include <WiFi.h>
#include <HTTPClient.h>

enum class bodyType { TEMP_INFO, HEAT_WARNING, READY };  

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
  
  String httpRequestData = "value1=" + createEmailBody(bodyType::HEAT_WARNING);
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

String createEmailBody(const bodyType& type)
{
  String body;
  
  switch(type)
  {
    case bodyType::TEMP_INFO:
    {
      body += "Lämmityksen aloituksesta: " + systemClockStr();
      body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";
      body += "<br>Veden 1 lämpötila: " + String( getTempWater1() );
      body += "<br>Veden 2 lämpötila: ????";    
    }
    break;

    case bodyType::HEAT_WARNING:
    {
      body += "<br>!! PALJUN KAMIINAAN TARVITAAN LISÄÄ POLTTOPUITA !!";
      body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";
      body += "<br>Veden 1 lämpötila: " + String( getTempWater1() );
      body += "<br>Veden 2 lämpötila: ????";    
    }
    break;

    case bodyType::READY:
    {
      body += "PALJU ON KYLPYVALMIS";
      body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";
      body += "<br>Veden lämpötila (1 anturi): " + String( getTempWater1() );
      body += "<br>Veden lämpötila (2 anturi): ????";   
    }
    break;
  }

  return body;
}
