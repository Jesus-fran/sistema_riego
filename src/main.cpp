#include <Arduino.h>
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>

#define FIREBASE_HOST "sistemariego-70eeb-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ON93C4yS1xwu15iFbUgiYlfE7XLw89XoVZIyPi9d"

FirebaseData fbdo_hum;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -18000);

int epoch_time_actual;
bool conect = false;
unsigned long interval = 3000;
unsigned long previous_milis;

// Obtiene la fecha y hora
unsigned long getTime()
{
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

void RegistrarDatosFirebase(String path, FirebaseJson data)
{
  if (Firebase.pushJSON(fbdo_hum, path, data))
  {
    Serial.print("registrado!");
  }
  else
  {
    Serial.print("Error al registrar");
  }
}

String GetDatosFirebase(String path)
{
  // Filtro de consulta
  QueryFilter query;
  query.orderBy("fecha");
  // query.limitToFirst(2);
  query.limitToLast(1);

  if (Firebase.RTDB.getJSON(&fbdo_hum, path, &query))
  {
    Serial.print("Se mete al IF");
    FirebaseJson &json = fbdo_hum.jsonObject();
    // Serial.print(fbdo_hum.stringData());
    size_t len = json.iteratorBegin();
    String key, value = "";
    int type = 0;
    DynamicJsonDocument new_array(1024);

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
    // Serializa array
    String arr_serialized;
    serializeJson(new_array, arr_serialized);
    Serial.print(arr_serialized);
    new_array.clear();
    DeserializationError error_des = deserializeJson(new_array, arr_serialized);
    if (error_des)
    {
      Serial.print("Hay un error: ");
      Serial.print(error_des.c_str());
    }
    Serial.print(error_des.c_str());
    String dts_json = new_array[0];
    new_array.clear();
    return dts_json;
  }
  else
  {
    Serial.print("Hay un error getJSON Firebase:c");
    delay(3000);
    Serial.print(fbdo_hum.errorReason());
    return "";
  }
  // limpia todos los parametros de consulta
  query.clear();
}

void setup()
{
  Serial.begin(57600);
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
  previous_milis = millis();
}

void loop()
{
  WiFiClient client;
  unsigned long current_millis = millis();
  if ((unsigned long)(current_millis - previous_milis) >= interval)
  {

    if (conect == false)
    {
      Serial.print("conectado");
    }
    if (Serial.available() > 0)
    {
      String data = Serial.readStringUntil('\n');
      Serial.flush();
      if (data == "como estas")
      {
        conect = true;
        Serial.print("bien");
      }
      else if (data.indexOf("temp") != -1 && conect == true)
      {
        StaticJsonDocument<300> doc;
        DeserializationError error = deserializeJson(doc, data);
        String tipo_error = error.c_str();
        if (tipo_error == "Ok")
        {
          Serial.print("ok al deserealizar");
          float temp = doc["temp"];
          float hum = doc["hum"];

          Serial.print("Obteniendo datos de Firebase...");
          String dts_humedad = GetDatosFirebase("/tierra/humedad");
          Serial.print(dts_humedad);
          String dts_temp = GetDatosFirebase("/tierra/temperatura");
          Serial.print(dts_temp);

          FirebaseJson dato_enviar;
          epoch_time_actual = getTime();
          dato_enviar.set("fecha", epoch_time_actual);
          dato_enviar.set("valor", hum);
          Serial.print(temp);
          Serial.print(hum);
          RegistrarDatosFirebase("/tierra/humedad", dato_enviar);
          dato_enviar.clear();
          dato_enviar.set("fecha", epoch_time_actual);
          dato_enviar.set("valor", temp);
          RegistrarDatosFirebase("/tierra/temperatura", dato_enviar);
          dato_enviar.clear();
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
    previous_milis = millis();
  }
}