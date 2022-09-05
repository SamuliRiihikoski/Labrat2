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

  Serial.println(getTempNTC());
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

  unsigned int tempVolt = ( analogRead(A4) / 4095 ) * 3.3;
  
  /*
  const int B = 4275;
  const int R0 = 100000;

  int a = analogRead(A4);
  float R = 4095.0/a - 1.0;
  R = R0*R;

  float temperature = 1.0/(log(R/R0)/B+1/298.15) - 273.15;
  Serial.print("temperature: ");
  Serial.println(temperature);
  */

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
