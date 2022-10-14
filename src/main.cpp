#include <Arduino.h>

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
    if (error) { return; }
}

StaticJsonDocument<300> json_des;

// des un json
void DeserializeJson(String json)
{
    DeserializationError error = deserializeJson(json_des, json);
    if (error) { return; }
}


void setup()
{
  Serial.begin(115200);
  WiFi.begin("LAN_957428", "coronavirus7/");
  timeClient.begin();

  Serial.print("Conectando al WIFI");
  delay(1000);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Conectado, dirección IP: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop()
{

  WiFiClient client;
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Obteniendo datos de Firebase...");

  // Obtiene datos sensores
  float temperatura = 0;
  int humedad = 0;

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
    // Serial.println(data_array);

    epoch_time_actual = getTime();

    Serial.println("--------------------");
    Serial.printf("Fecha y hora actual: %d/%d/%d | %d:%d:%d", day(epoch_time_actual), month(epoch_time_actual), year(epoch_time_actual), hour(epoch_time_actual), minute(epoch_time_actual), second(epoch_time_actual));
    Serial.println("-");
    Serial.println("-");
    Serial.println("-");
    Serial.println("-");
   
    // des json del array
    String data_doc = new_array[0];
    Serial.println(data_doc);
    DeserializeJson(data_doc);
    int fecha_hora = json_des["fecha"];
    Serial.println(fecha_hora);

    Serial.println("-");
    Serial.println("-");
    Serial.println("-");
    // Serial.printf("Timestamp:  %d/%d/%d  %d:%d:%d", day(epoch_time), month(epoch_time), year(epoch_time), hour(epoch_time), minute(epoch_time), second(epoch_time));
    Serial.println("--------------------");
  }
  else
  {
    Serial.println("Hay un error :c");
    Serial.println(fbdo.errorReason());
  }

  // limpia todos los parametros de consulta
  query.clear();
  delay(5000);
}