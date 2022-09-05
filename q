[1mdiff --git a/kotiautomaatio/tests/ifttt_testing/ifttt_testing/ifttt_testing.ino b/kotiautomaatio/tests/ifttt_testing/ifttt_testing/ifttt_testing.ino[m
[1mindex ec5bead..087dcb4 100644[m
[1m--- a/kotiautomaatio/tests/ifttt_testing/ifttt_testing/ifttt_testing.ino[m
[1m+++ b/kotiautomaatio/tests/ifttt_testing/ifttt_testing/ifttt_testing.ino[m
[36m@@ -14,7 +14,7 @@[m [mState state = State::INIT;[m
 unsigned long startTime = 0;[m
 unsigned long timer = 0;[m
 bool alertImmediately = true;[m
[31m-unsigned int prevValueTemp1;[m
[32m+[m[32mfloat sensorTopMaxTemp;[m
 [m
 //AnalogRead(A3) = Bottom (0)[m
 //AnalogRead(A4) = Top (1)[m
[36m@@ -44,7 +44,7 @@[m [mvoid loop()[m
         Serial.println("Heating has started...");[m
         startTime = ( millis() / 1000 );[m
         timer = millis();[m
[31m-        prevValueTemp1 = getTempValues(Sensor::TOP);[m
[32m+[m[32m        sensorTopMaxTemp = getTempValues(Sensor::TOP);[m
         sendEmail(msgType);[m
         state = State::RUNNING;[m
       }[m
[36m@@ -64,7 +64,7 @@[m [mvoid loop()[m
       }[m
 [m
       // KUN LÄMPÖTILA ON ALKAA LASKEA[m
[31m-      if (getTempValues(Sensor::TOP) < prevValueTemp1( &&  alertImmediately )[m
[32m+[m[32m      if ( ( sensorTopMaxTemp - getTempValues(Sensor::TOP) > 1.0 ) && alertImmediately )[m
       {[m
         Serial.println("Temp is getting too low");      [m
         alertImmediately = false;[m
[36m@@ -74,7 +74,7 @@[m [mvoid loop()[m
       }[m
 [m
       // KUN LÄMPÖTILA ALKAA TAAS NOUSEMAAN[m
[31m-      else if (getTempValues(Sensor::TOP)>prevValueTemp1) && !alertImmediately )[m
[32m+[m[32m      else if ( getTempValues(Sensor::TOP) > sensorTopMaxTemp && !alertImmediately )[m
       {[m
         Serial.println("Now heater works again");[m
         alertImmediately = true;[m
[36m@@ -88,8 +88,19 @@[m [mvoid loop()[m
         timer = millis();[m
         sendEmail(msgType);[m
       }[m
[31m-      prevValueTemp1 = getTempValues(Sensor::TOP);[m
[31m-      delay(500);[m
[32m+[m
[32m+[m[32m      float temp = getTempValues(Sensor::TOP);[m
[32m+[m
[32m+[m[32m      if ( temp > sensorTopMaxTemp )[m
[32m+[m[32m      {[m
[32m+[m[32m        sensorTopMaxTemp = getTempValues(Sensor::TOP);[m
[32m+[m[32m      }[m
[32m+[m
[32m+[m[32m      Serial.print("NTC: ");[m
[32m+[m[32m      Serial.println(getTempValues(Sensor::TOP));[m
[32m+[m[32m      Serial.print("Potikka: ");[m
[32m+[m[32m      Serial.println(getTempValues(Sensor::BOTTOM));[m
[32m+[m[32m      delay(1000);[m
     }[m
     break;[m
 [m
[36m@@ -133,9 +144,6 @@[m [mfloat getTempValues(Sensor sensor)[m
     [m
     TX = TX - 273.15;[m
     [m
[31m-    Serial.print("NTC temp: ");[m
[31m-    Serial.println(TX);[m
[31m-    [m
     return TX;[m
   }[m
   else if (sensor == Sensor::BOTTOM)[m
[36m@@ -158,8 +166,8 @@[m [mvoid sendEmail(const MsgType& type)[m
     {[m
       body += "Lämmityksen aloituksesta: " + systemClockStr();[m
       body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";[m
[31m-      body += "<br>Veden lämpötila (1 anturi): " + String( getTempNTC() );[m
[31m-      body += "<br>Veden lämpötila (2 anturi): " + String( getTempWater2() );    [m
[32m+[m[32m      body += "<br>Veden lämpötila (1 anturi): " + String( getTempValues( Sensor::TOP ) );[m
[32m+[m[32m      body += "<br>Veden lämpötila (2 anturi): " + String( getTempValues( Sensor::BOTTOM ) );[m[41m    [m
     }[m
     break;[m
 [m
[36m@@ -167,8 +175,8 @@[m [mvoid sendEmail(const MsgType& type)[m
     {[m
       body += "<br>!! PALJUN KAMIINAAN TARVITAAN LISÄÄ POLTTOPUITA !!";[m
       body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";[m
[31m-      body += "<br>Veden lämpötila (1 anturi): " + String( getTempNTC() );[m
[31m-      body += "<br>Veden lämpötila (2 anturi): " + String( getTempWater2() );     [m
[32m+[m[32m      body += "<br>Veden lämpötila (1 anturi): " + String( getTempValues(Sensor::TOP ) );[m
[32m+[m[32m      body += "<br>Veden lämpötila (2 anturi): " + String( getTempValues(Sensor::BOTTOM ) );[m[41m     [m
     }[m
     break;[m
 [m
[36m@@ -176,8 +184,8 @@[m [mvoid sendEmail(const MsgType& type)[m
     {[m
       body += "PALJU ON KYLPYVALMIS";[m
       body += "<br>Paljun tavoitelämpötila: 37.0 celciusta";[m
[31m-      body += "<br>Veden lämpötila (1 anturi): " + String( getTempNTC() );[m
[31m-      body += "<br>Veden lämpötila (2 anturi): " + String( getTempWater2() );    [m
[32m+[m[32m      body += "<br>Veden lämpötila (1 anturi): " + String( getTempValues(Sensor::TOP ) );[m
[32m+[m[32m      body += "<br>Veden lämpötila (2 anturi): " + String( getTempValues(Sensor::BOTTOM ) );[m[41m    [m
     }[m
     break;[m
   }[m
