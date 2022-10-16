#include <Arduino.h>
#include <SoftwareSerial.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

#define FIREBASE_HOST "sistemariego-70eeb-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ON93C4yS1xwu15iFbUgiYlfE7XLw89XoVZIyPi9d"
String path = "/tierra";

FirebaseData fbdo;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -18000);

int epoch_time;
int epoch_time_actual;
String fecha_humedad;
String hora_humedad;
float valor_humedad;
bool conect = false;

// Obtiene la fecha y hora
unsigned long getTime()
{
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

// des un array
StaticJsonDocument<300> array_des;
void DeserializeArray(String array)
{
  DeserializationError error = deserializeJson(array_des, array);
  if (error)
  {
    return;
  }
}

StaticJsonDocument<300> json_des;

// des un json
void DeserializeJson(String json)
{
  DeserializationError error = deserializeJson(json_des, json);
  if (error)
  {
    return;
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin("LAN_957428", "coronavirus7/");
  timeClient.begin();

  Serial.print("Conectado");
  delay(1000);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  delay(3000);
  Serial.print("IP: ");
  Serial.print(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  delay(5000);
}

void loop()
{
  WiFiClient client;

  if (conect == false)
  {
    Serial.print("conectado");
  }
  delay(5000);
  if (Serial.available() > 0)
  {
    String data = Serial.readStringUntil('\n');
    Serial.flush();
    if (data == "como estas")
    {
      conect = true;
      Serial.print("bien");
      delay(3000);
    }
    else if (data.indexOf("temp") != -1)
    {
      StaticJsonDocument<300> doc;
      DeserializationError error = deserializeJson(doc, data);
      if (error.c_str() == "Ok")
      {
        Serial.print("ok al deserealizar");
        float temp = doc["temp"];
        float hum = doc["hum"];
        delay(3000);
        Serial.print(temp);
        delay(3000);
        Serial.print(hum);
        delay(3000);
        // Serial.print(WiFi.localIP());
        delay(3000);

        Serial.print("Obteniendo datos de Firebase...");

        // Filtro de consulta
        QueryFilter query;
        query.orderBy("fecha");
        // query.limitToFirst(2);
        query.limitToLast(2);

        if (Firebase.RTDB.getJSON(&fbdo, "/tierra/humedad", &query))
        {

          FirebaseJson &json = fbdo.jsonObject();
          size_t len = json.iteratorBegin();
          String key, value = "";
          int type = 0;
          StaticJsonDocument<300> new_array;

          // itera el json
          for (size_t i = 0; i < len; i++)
          {
            json.iteratorGet(i, type, key, value);
            bool is_int = value.toInt();
            if (!is_int)
            {
              // almacena en un array
              new_array.add(value);
            }
          }
          json.iteratorEnd();
          json.clear();
          String data_array;
          serializeJson(new_array, data_array);
          // Serial.print(data_array);

          epoch_time_actual = getTime();

          Serial.printf("Fecha y hora actual: %d/%d/%d | %d:%d:%d", day(epoch_time_actual), month(epoch_time_actual), year(epoch_time_actual), hour(epoch_time_actual), minute(epoch_time_actual), second(epoch_time_actual));

          // des json del array
          String data_doc = new_array[0];
          // Serial.print(data_doc);
          DeserializeJson(data_doc);
          int fecha_hora = json_des["fecha"];
          float valor = json_des["valor"];
          Serial.print(fecha_hora);
          delay(3000);
          Serial.print(valor);
          delay(3000);
          // Serial.printf("Timestamp:  %d/%d/%d  %d:%d:%d", day(epoch_time), month(epoch_time), year(epoch_time), hour(epoch_time), minute(epoch_time), second(epoch_time));
          Serial.print("--------------------");
          delay(3000);
        }
        else
        {
          Serial.print("Hay un error :c");
          delay(3000);
          Serial.print(fbdo.errorReason());
          delay(3000);
        }

        // limpia todos los parametros de consulta
        query.clear();
        delay(8000);
      }
      else
      {
        Serial.print("Error al parsear");
        delay(3000);
      }
      Serial.flush();
    }
  }
  if (conect == true)
  {
    Serial.print("data");
  }
}