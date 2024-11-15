#include "TempoLikeSupplyContractAPI.h"

TempoLikeSupplyContractAPI::TempoLikeSupplyContractAPI(String client_secret, String client_id)
{
  _debug = false;
  _access_token = "";
  _client_secret = client_secret;
  _client_id = client_id;
  tomorrowColor = DAY_NOT_AVAILABLE;
  todayColor = DAY_NOT_AVAILABLE;
  countBlue = 0;
  countRed = 0;
  countWhite = 0;
  cptCall = 0;
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

int TempoLikeSupplyContractAPI::fetchColors(String today, String tomorrow, String afterTomorrow, String debutSaison)
{
  int retour = TEMPOAPI_KO;
  tomorrowColor = DAY_NOT_AVAILABLE;
  todayColor = DAY_NOT_AVAILABLE;
  countBlue = 0;
  countRed = 0;
  countWhite = 0;
  if (_access_token == "")
  {
    fetchAccessToken();
  }
  if (_access_token == "")
  {
    Serial.println("Erreur d'authent");
    return retour;
  }
  // je parie qu'il y aura un bug en passage heure d'hiver (/ heure d'été) : on sera en GMT +1 au lieu de +2
  debutSaison.replace("+", "%2B");
  afterTomorrow.replace("+", "%2B");
  
  String body = tempoLikeSupplyContractService(debutSaison, afterTomorrow);
  DynamicJsonDocument doc(body.length());
  deserializeJson(doc, body);
  if (doc.containsKey("tempo_like_calendars"))
  {
    JsonArray results = doc["tempo_like_calendars"]["values"].as<JsonArray>();
    String tmpColor = "";
    for (JsonObject result : results)
    {
      tmpColor = frenchColor(result["value"].as<String>());
      if (_debug)
      {
        Serial.println(result["start_date"].as<String>().substring(0, 10) + " " + tmpColor);
      }

      if (result["start_date"].as<String>().substring(0, 10) == today.substring(0, 10))
      {
        todayColor = tmpColor;
        if (_debug)
        {
          Serial.println("Today Found");
        }
      }
      else if (result["start_date"].as<String>().substring(0, 10) == tomorrow.substring(0, 10))
      {
        tomorrowColor = tmpColor;
        if (_debug)
        {
          Serial.println("Tommorow Found");
        }
      }
      // on ne compte que les jours déjà passés.
      else
      {
        if (tmpColor == "BLEU")
        {
          countBlue++;
        }
        else if (tmpColor == "BLANC")
        {
          countWhite++;
        }
        else if (tmpColor == "ROUGE")
        {
          countRed++;
        }
        else
        {
          Serial.println("Couleur non reconnue : " + tmpColor);
        }
      }
    }

    if (_debug)
    {
      Serial.print("countBlue : ");
      Serial.println(countBlue);
      Serial.print("countWhite : ");
      Serial.println(countWhite);
      Serial.print("countRed : ");
      Serial.println(countRed);
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
    fetchPreviewRTE(today.substring(0, 10), tomorrow.substring(0, 10));
  }

  return retour;
}

// Plein de code dupliqué de la méthode fetchColors
int TempoLikeSupplyContractAPI::fecthColorsFreeApi(String today, String tomorrow, String saison)
{
  int retour = TEMPOAPI_KO;
  tomorrowColor = DAY_NOT_AVAILABLE;
  todayColor = DAY_NOT_AVAILABLE;
  countBlue = 0;
  countRed = 0;
  countWhite = 0;

  String body = tempoService(saison);
  DynamicJsonDocument doc(body.length());
  deserializeJson(doc, body);
  if (doc.containsKey("values"))
  {
    JsonObject results = doc["values"].as<JsonObject>();
    String valeur = "";
    String clef = "";
    for (JsonPair kv : results)
    {
      clef = kv.key().c_str();
      valeur = frenchColor(kv.value().as<String>());

      if (_debug)
      {
        Serial.println(clef + ":" + valeur);
      }

      if (clef == today.substring(0, 10))
      {
        todayColor = valeur;
        if (_debug)
        {
          Serial.println("Today Found");
        }
      }
      else if (clef == tomorrow.substring(0, 10))
      {
        tomorrowColor = valeur;
        if (_debug)
        {
          Serial.println("Tommorow Found");
        }
      }
      // on ne compte que les jours déjà passés.
      else
      {
        if (clef.length() == 10)
        {
          if (valeur == "BLEU")
          {
            countBlue++;
          }
          else if (valeur == "BLANC")
          {
            countWhite++;
          }
          else if (valeur == "ROUGE")
          {
            countRed++;
          }
          else
          {
            Serial.println("Couleur non reconnue : " + valeur);
          }
        }
      }
    }

    if (_debug)
    {
      Serial.print("countBlue : ");
      Serial.println(countBlue);
      Serial.print("countWhite : ");
      Serial.println(countWhite);
      Serial.print("countRed : ");
      Serial.println(countRed);
    }
  }
  else
  {
    Serial.println("No tempo");
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
    fetchPreviewRTE(today.substring(0, 10), tomorrow.substring(0, 10));
  }

  return retour;
}

int TempoLikeSupplyContractAPI::fetchPreviewRTE(String today, String tomorrow)
{
  int retour = TEMPOAPI_KO;
  if (_debug)
  {
    Serial.println("today : " + today);
    Serial.println("tomorrow : " + tomorrow);
  }
  String body = previewRTEService();
  if (body != "")
  {
    DynamicJsonDocument doc(body.length());
    deserializeJson(doc, body);
    if (doc.containsKey("values") && doc["values"].containsKey(tomorrow) && doc["values"].containsKey(today))
    {
      todayColor = frenchColor(doc["values"][today].as<String>()) + "*";
      tomorrowColor = frenchColor(doc["values"][tomorrow].as<String>()) + "*";
      retour = TEMPOAPI_OK;
      if (_debug)
      {
        Serial.println("Couleur preview trouvées");
        Serial.println(todayColor + "/" + tomorrowColor);
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
    error_code[cptCall++] = httpCode + 1000; 
    if (httpCode > 0)
    {
      String body = http.getString();
      if (_debug)
      {
        Serial.println("getAccessToken body :");
        Serial.println(body);
      }
      DynamicJsonDocument doc(body.length());
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
    error_code[cptCall++] = httpCode + 2000;     
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
    error_code[cptCall++] = httpCode + 3000;     
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

String TempoLikeSupplyContractAPI::tempoService(String saison)
{
  String body = "";
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    String url = "https://www.services-rte.com/cms/open_data/v1/tempo";
    url += "?season=" + saison;

    if (_debug)
    {
      Serial.println(url);
    }

    http.begin(url);
    http.addHeader("Accept", "application/json");

    int httpCode = http.GET();
    error_code[cptCall++] = httpCode + 4000;     
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
        Serial.println("tempo body :");
        Serial.println(body);
      }
    }
    else
    {
      Serial.println("tempo Error : " + http.errorToString(httpCode));
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
  Serial.println("Couleur inconnue : " + color);
  return "???";
}
