#include <WiFi.h>
#include <HTTPClient.h>

#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP  5

enum class State { INIT, RUNNING, FINISHED };
enum class MsgType { TEMP_INFO, HEAT_WARNING, READY };
enum class Sensor {TOP, BOTTOM};

MsgType msgType = MsgType::TEMP_INFO;

const char* ssid = "AndroidAP";
const char* password = "Park2771Uruk";
const char* serverName = "http://maker.ifttt.com/trigger/palju_tila/with/key/cr1htqdHyv8u37TsqFjgMh";

RTC_DATA_ATTR State state = State::INIT;
RTC_DATA_ATTR unsigned long startTime = 0;
RTC_DATA_ATTR unsigned long currentTime = 0;
RTC_DATA_ATTR unsigned long timer = 0;
RTC_DATA_ATTR bool alertImmediately = true;
RTC_DATA_ATTR float sensorTopMaxTemp;

//AnalogRead(A3) = Bottom (0)
//AnalogRead(A4) = Top (1)
// anturi 2 on korkeudella 30,78. (ylhäältä-alas)

void setup() 
{  
  Serial.begin(115200);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void loop() 
{
  switch(state)
  {
    case State::INIT:
    {
      if (getTempValues(Sensor::TOP))
      {
        Serial.println("Heating has started...");
        startTime = currentTime;
        timer = currentTime;
        sensorTopMaxTemp = getTempValues(Sensor::TOP);
        sendEmail(msgType);
        state = State::RUNNING;
      }
    }
    break;

    case State::RUNNING:
    {
      Serial.println("Now we are in running state...");

      //KUN LÄMMMITYS ON VALMIS
      if (getTempValues(Sensor::TOP) >= 60 && getTempValues(Sensor::BOTTOM) >= 60) //get top sensor value
      {
        sendEmail(MsgType::READY);                     
        state = State::FINISHED;
        return;
      }

      // KUN LÄMPÖTILA ON ALKAA LASKEA
      if ( ( sensorTopMaxTemp - getTempValues(Sensor::TOP) > 1.0 ) && alertImmediately )
      {
        Serial.println("Temp is getting too low");      
        alertImmediately = false;
        msgType = MsgType::HEAT_WARNING;
        sendEmail(msgType);
        timer = 0;
      }

      // KUN LÄMPÖTILA ALKAA LASKUN JÄLKEEN TAAS NOUSEMAAN
      else if ( getTempValues(Sensor::TOP) > sensorTopMaxTemp && !alertImmediately )
      {
        Serial.println("Now heater works again");
        alertImmediately = true;
        msgType = MsgType::TEMP_INFO;
        
      }
      
      if ( ( currentTime - timer ) > 60000 ) // Sending email every 10 mins.
      {
        Serial.println("Send email interrupt");
        timer = currentTime;
        sendEmail(msgType);
      }

      // let's following max temp. If current temp drops too much from max let's send heating alarm.
      float temp = getTempValues(Sensor::TOP);

      if ( temp > sensorTopMaxTemp )
      {
        sensorTopMaxTemp = getTempValues(Sensor::TOP);
      }
    }
    break;

    case State::FINISHED:
    {
      Serial.println("FINISHED");
      // Do nothing
    }
    break;
  }
  Serial.print("CurrentTime: ");
  Serial.println(currentTime);
  delay(1000); // with this we make sure Serial.print gets finished before going sleep mode

  currentTime += millis() + ( TIME_TO_SLEEP * 1000 );
  
  esp_deep_sleep_start();
}

String systemClockStr() 
{
  char str[90];
  unsigned long currentSeconds = ( currentTime / 1000 ) - ( startTime / 1000 );
  unsigned int sec = currentSeconds % 60;
  unsigned int minutes = ( currentSeconds % 3600 ) / 60;
  unsigned int hours = currentSeconds / 3600;
  sprintf(str, "%02u:%02u:%02u", hours, minutes, sec);
  return String(str);
}

float getTempValues(Sensor sensor)
{
  if (sensor == Sensor::TOP)
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
    
    return TX;
    
  }
  else if (sensor == Sensor::BOTTOM)
  {  
    return ( analogRead(A3) / (float)4095 ) * 60;
  }

  return float{};
}

void sendEmail(const MsgType& type)
{  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  
  while( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("wifi connected.");
  
  String body;
  WiFiClient client;
  HTTPClient http;
  
  switch(type)
  {
    case MsgType::TEMP_INFO:
    {
      body += "Lämmityksen aloituksesta: " + systemClockStr();
      body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";
      body += "<br>Veden lämpötila (1 anturi): " + String( getTempValues( Sensor::TOP ) );
      body += "<br>Veden lämpötila (2 anturi): " + String( getTempValues( Sensor::BOTTOM ) );    
    }
    break;

    case MsgType::HEAT_WARNING:
    {
      body += "<br>!! PALJUN KAMIINAAN TARVITAAN LISÄÄ POLTTOPUITA !!";
      body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";
      body += "<br>Veden lämpötila (1 anturi): " + String( getTempValues(Sensor::TOP ) );
      body += "<br>Veden lämpötila (2 anturi): " + String( getTempValues(Sensor::BOTTOM ) );     
    }
    break;

    case MsgType::READY:
    {
      body += "PALJU ON KYLPYVALMIS";
      body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";
      body += "<br>Veden lämpötila (1 anturi): " + String( getTempValues(Sensor::TOP ) );
      body += "<br>Veden lämpötila (2 anturi): " + String( getTempValues(Sensor::BOTTOM ) );    
    }
    break;
  }

  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  String httpRequestData = "value1=" + body;
  int httpResponseCode = http.POST(httpRequestData); 
  
  http.end();
  WiFi.disconnect();
}
