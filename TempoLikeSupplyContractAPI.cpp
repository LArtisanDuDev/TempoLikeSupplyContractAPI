#include "TempoLikeSupplyContractAPI.h"

TempoLikeSupplyContractAPI::TempoLikeSupplyContractAPI(String client_secret, String client_id)
{
  _debug = false;
  _authSuccess = false;
  _client_secret = client_secret;
  _client_id = client_id;
  tomorrowColor = DAY_NOT_AVAILABLE;
  todayColor = DAY_NOT_AVAILABLE;
}

TempoLikeSupplyContractAPI::~TempoLikeSupplyContractAPI()
{
}

void TempoLikeSupplyContractAPI::setDebug(bool debug)
{
  _debug = debug;
}

void TempoLikeSupplyContractAPI::fetchAccessToken()
{
  _authSuccess = false;
  String tmp = _client_id + ":" + _client_secret;
  String base64Auth = base64::encode(tmp);

  if (_debug)
  {
    Serial.println("tmp :");
    Serial.println(tmp);
    Serial.println("base64Auth :");
    Serial.println(base64Auth);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    http.begin("https://digital.iservices.rte-france.com/token/oauth");
    http.addHeader("Authorization", "Basic " + base64Auth);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST("");
    if (_debug)
    {
      Serial.println("Server Response :");
      Serial.println(httpCode);
    }

    if (httpCode > 0)
    {
      DynamicJsonDocument doc(1024);
      String body = http.getString();
      if (_debug)
      {
        Serial.println("getAccessToken body :");
        Serial.println(body);
      }

      deserializeJson(doc, body);
      if (doc.containsKey("access_token"))
      {
        _access_token = doc["access_token"].as<String>();
        _authSuccess = true;
        if (_debug)
        {
          Serial.println(String("Received access token :") + _access_token);
        }
      }
      else
      {
        Serial.println("No access_token");
      }
    }
    else
    {
      Serial.println("getAccessToken Error : " + http.errorToString(httpCode));
    }
    http.end();
  }
}

int TempoLikeSupplyContractAPI::fetchColors(String today, String tomorrow, String afterTomorrow)
{
  int retour = TEMPOAPI_KO;
  if (!_authSuccess)
  {
    fetchAccessToken();
  }
  if (!_authSuccess)
  {
    Serial.println("Erreur d'authent");
    return retour;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    bool needPreviewRTE = true;
    HTTPClient http;
    String url = "https://digital.iservices.rte-france.com/open_api/tempo_like_supply_contract/v1/tempo_like_calendars";
    url += "?start_date=" + today + "T00:00:00%2B02:00&end_date=" + afterTomorrow + "T00:00:00%2B02:00";

    String bearer = "Bearer  " + _access_token;

    if (_debug)
    {
      Serial.println(url);
      Serial.println(bearer);
    }

    http.begin(url);
    http.addHeader("Authorization", bearer);
    http.addHeader("Accept", "application/json");

    int httpCode = http.GET();
    if (_debug)
    {
      Serial.println("Server Response :");
      Serial.println(httpCode);
    }

    if (httpCode > 0)
    {
      DynamicJsonDocument doc(2048);
      String body = http.getString();
      if (_debug)
      {
        Serial.println("tempo_like_calendars body :");
        Serial.println(body);
      }

      deserializeJson(doc, body);
      int foundColor = 0;
      if (doc.containsKey("tempo_like_calendars"))
      {
        JsonArray results = doc["tempo_like_calendars"]["values"].as<JsonArray>();
        for (JsonObject result : results)
        {
          if (_debug)
          {
            Serial.println("Test");
            Serial.println(result["start_date"].as<String>().substring(0, 10));
            Serial.println(" == " + today);
          }
          if (result["start_date"].as<String>().substring(0, 10) == today)
          {
            todayColor = result["value"].as<String>();
            foundColor++;
          }
          else
          {
            tomorrowColor = result["value"].as<String>();
            foundColor++;
          }

          if (_debug)
          {
            Serial.println("todayColor : " + todayColor);
            Serial.println("tomorrowColor : " + tomorrowColor);
          }
        }
      }
      else
      {
        Serial.println("No tempo_like_calendars");
      }
      needPreviewRTE = (foundColor != 2);
      if (_debug)
      {
        Serial.println("foundColor :");
        Serial.println(foundColor);
        Serial.println("needPreviewRTE :");
        Serial.println(needPreviewRTE);
      }

      retour = TEMPOAPI_OK;
    }
    else
    {
      Serial.println("tempo_like_calendars Error : " + http.errorToString(httpCode));
    }
    http.end();

    if (needPreviewRTE)
    {
      previewRTE(tomorrow);
    }
  }
  return retour;
}

int TempoLikeSupplyContractAPI::previewRTE(String tomorrow)
{
  int retour = TEMPOAPI_KO;
  if (_debug)
  {
    Serial.println("tomorrow : " + tomorrow);
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin("https://www.services-rte.com/cms/open_data/v1/tempoLight");
    int httpCode = http.GET();

    if (_debug)
    {
      Serial.println("Server Response :");
      Serial.println(httpCode);
    }

    if (httpCode > 0)
    {
      DynamicJsonDocument doc(1024);
      String body = http.getString();
      if (_debug)
      {
        Serial.println("tempoLigh body :");
        Serial.println(body);
      }

      deserializeJson(doc, body);
      if (doc.containsKey("values") && doc["values"].containsKey(tomorrow))
      {
        tomorrowColor = doc["values"][tomorrow].as<String>();
        retour = TEMPOAPI_OK;
        if (_debug)
        {
          Serial.println("Couleur preview de demain trouvée");
          Serial.println(tomorrowColor);
        }
      }
    }
    else
    {
      Serial.println("Échec de la récupération des couleurs preview RTE, code HTTP : " + String(httpCode));
    }
    http.end();
  }
  return retour;
}
