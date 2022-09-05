#include <WiFi.h>
#include <HTTPClient.h>

enum class State { INIT, RUNNING, FINISHED };
enum class MsgType { TEMP_INFO, HEAT_WARNING, READY };
MsgType msgType = MsgType::TEMP_INFO;

const char* ssid = "AndroidAP";
const char* password = "Park2771Uruk";
const char* serverName = "http://maker.ifttt.com/trigger/palju_tila/with/key/cr1htqdHyv8u37TsqFjgMh";

State state = State::INIT;
unsigned long startTime = 0;
unsigned long timer = 0;
bool alertImmediately = true;
unsigned int prevValueTemp1;

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
  switch(state)
  {
    case State::INIT:
    {
      if ( analogRead(A3) > 0 )
      {
        Serial.println("Heating has started...");
        startTime = ( millis() / 1000 );
        timer = millis();
        prevValueTemp1 = analogRead(A3);
        sendEmail(msgType);
        state = State::RUNNING;
      }
    }
    break;

    case State::RUNNING:
    {
      Serial.println("Now we are in running state...");
      
      if (analogRead(A3) >= 4095 )
      {
        sendEmail(MsgType::READY);
        state = State::FINISHED;
        return;
      }

      if ( prevValueTemp1 > ( analogRead(A3) + 100 ) &&  alertImmediately )
      {
        Serial.println("Temp is getting too low");
        alertImmediately = false;
        msgType = MsgType::HEAT_WARNING;
        sendEmail(msgType);
        timer = 0;
      }
      else if ( ( prevValueTemp1 + 100 ) < analogRead(A3) && !alertImmediately )
      {
        Serial.println("Now heater works again");
        alertImmediately = true;
        msgType = MsgType::TEMP_INFO;
        
      }
      
      if ( ( millis() - timer ) > 30000 ) 
      {
        Serial.println("Timer interrupt");
        timer = millis();
        sendEmail(msgType);
      }
      

      prevValueTemp1 = analogRead(A3);
      delay(1000);
    }
    break;

    case State::FINISHED:
    {
      // Do nothing
    }
    break;
  }

  getTempNTC();
  delay(1000);
  
}

String systemClockStr() 
{
  char str[90];
  unsigned long currentSeconds = ( millis() / 1000 ) - startTime;
  unsigned int sec = currentSeconds % 60;
  unsigned int minutes = ( currentSeconds % 3600 ) / 60;
  unsigned int hours = currentSeconds / 3600;
  sprintf(str, "%02u:%02u:%02u", hours, minutes, sec);
  return String(str);
}

float getTempNTC()
{
  const float vRef = 3.3;
  const int R = 10000; 
  float RT, VR, ln, TX, T0, VRT;

  T0 = 25 + 273.15;
  VRT = analogRead(A4);
  VRT = ( 3.3 / 4095 ) * VRT;
  VR = 3.3 - VRT;
  RT = VRT / (VR / R);

  ln = log(RT / 10000);
  TX = (1 / ((ln / 3435) + (1 / T0)));

  TX = TX - 273.15;

  Serial.print("NTC temp: ");
  Serial.println(TX);

  return analogRead(A4);
}

unsigned int getTempWater2()
{
  return analogRead(A3);
}

void sendEmail(const MsgType& type)
{
  String body;
  WiFiClient client;
  HTTPClient http;
  
  switch(type)
  {
    case MsgType::TEMP_INFO:
    {
      body += "Lämmityksen aloituksesta: " + systemClockStr();
      body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";
      body += "<br>Veden lämpötila (1 anturi): " + String( getTempNTC() );
      body += "<br>Veden lämpötila (2 anturi): " + String( getTempWater2() );    
    }
    break;

    case MsgType::HEAT_WARNING:
    {
      body += "<br>!! PALJUN KAMIINAAN TARVITAAN LISÄÄ POLTTOPUITA !!";
      body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";
      body += "<br>Veden lämpötila (1 anturi): " + String( getTempNTC() );
      body += "<br>Veden lämpötila (2 anturi): " + String( getTempWater2() );     
    }
    break;

    case MsgType::READY:
    {
      body += "PALJU ON KYLPYVALMIS";
      body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";
      body += "<br>Veden lämpötila (1 anturi): " + String( getTempNTC() );
      body += "<br>Veden lämpötila (2 anturi): " + String( getTempWater2() );    
    }
    break;
  }

  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  String httpRequestData = "value1=" + body;
  int httpResponseCode = http.POST(httpRequestData); 
  
  http.end();
}
