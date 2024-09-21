#include "TempoLikeSupplyContractAPI.h"

TempoLikeSupplyContractAPI::TempoLikeSupplyContractAPI(String client_secret, String client_id)
{
  _debug = false;
  _access_token = "";
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
  _access_token = oauthService();
}

int TempoLikeSupplyContractAPI::fetchColors(String today, String tomorrow, String afterTomorrow)
{
  int retour = TEMPOAPI_KO;
  tomorrowColor = DAY_NOT_AVAILABLE;
  todayColor = DAY_NOT_AVAILABLE;
  if (_access_token == "")
  {
    fetchAccessToken();
  }
  if (_access_token == "")
  {
    Serial.println("Erreur d'authent");
    return retour;
  }
  // je parie qu'il y aura un bug en passage heure d'hiver (on sera en GMT +1 au lieu de +2)
  String body = tempoLikeSupplyContractService(today + "T00:00:00%2B02:00", afterTomorrow + "T00:00:00%2B02:00");
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, body);
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
        todayColor = frenchColor(result["value"].as<String>());
      }
      else
      {
        tomorrowColor = frenchColor(result["value"].as<String>());
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

  bool needPreviewRTE = (tomorrowColor == String(DAY_NOT_AVAILABLE));
  if (_debug)
  {
    Serial.println("todayColor :");
    Serial.println(todayColor);
    Serial.println("tomorrowColor :");
    Serial.println(tomorrowColor);
    Serial.println("needPreviewRTE :");
    Serial.println(needPreviewRTE);
  }
  retour = TEMPOAPI_OK;

  if (needPreviewRTE)
  {
    fetchPreviewRTE(tomorrow);
  }
  return retour;
}


int TempoLikeSupplyContractAPI::fetchPreviewRTE(String tomorrow)
{
  int retour = TEMPOAPI_KO;
  if (_debug)
  {
    Serial.println("tomorrow : " + tomorrow);
  }
  String body = previewRTEService();
  if (body != "")
  {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, body);
    if (doc.containsKey("values") && doc["values"].containsKey(tomorrow))
    {
      tomorrowColor = frenchColor(doc["values"][tomorrow].as<String>()) + "*";
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
   
    Serial.println("Échec de la récupération des couleurs preview RTE");
  }

  return retour;
}

String TempoLikeSupplyContractAPI::oauthService()
{
  String base64Auth = base64::encode(_client_id + ":" + _client_secret);
  String token = "";
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
        token = doc["access_token"].as<String>();
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
  return token;
}

String TempoLikeSupplyContractAPI::tempoLikeSupplyContractService(String startDate, String endDate)
{
  String body = "";
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = "https://digital.iservices.rte-france.com/open_api/tempo_like_supply_contract/v1/tempo_like_calendars";
    url += "?start_date=" + startDate + "&end_date=" + endDate;

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
      body = http.getString();
      if (_debug)
      {
        Serial.println("tempo_like_calendars body :");
        Serial.println(body);
      }
    }
    else
    {
      Serial.println("tempo_like_calendars Error : " + http.errorToString(httpCode));
    }
    http.end();
  }
  return body;
}

String TempoLikeSupplyContractAPI::previewRTEService()
{
  String body = "";
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin("https://www.services-rte.com/cms/open_data/v1/tempoLight");
    int httpCode = http.GET();

    if (_debug)
    {
      Serial.println("tempoLight Server Response :");
      Serial.println(httpCode);
    }

    if (httpCode > 0)
    {
      body = http.getString();
      if (_debug)
      {
        Serial.println("tempoLigh body :");
        Serial.println(body);
      }
    }
    else
    {
      Serial.println("tempoLight Server Error : " + String(httpCode));
    }
    http.end();
  }
  return body;
}

String TempoLikeSupplyContractAPI::frenchColor(String color)
{
  if (color == "BLUE")
    return "BLEU";
  if (color == "WHITE")
    return "BLANC";
  if (color == "RED")
    return "ROUGE";
  return "???";
}
