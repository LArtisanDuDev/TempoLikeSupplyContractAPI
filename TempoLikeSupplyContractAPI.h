#ifndef TempoLikeSupplyContractAPI_h
#define TempoLikeSupplyContractAPI_h
#endif

#include <WString.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>

#define TEMPOAPI_OK 1
#define TEMPOAPI_KO 0

#define DAY_NOT_AVAILABLE "N/A"

class TempoLikeSupplyContractAPI
{
public:
  TempoLikeSupplyContractAPI(String client_secret, String client_id);
  ~TempoLikeSupplyContractAPI();
  // des accesseurs ça serait mieux...
  String todayColor;
  String tomorrowColor;
  int countBlue;
  int countWhite;
  int countRed;

  void setDebug(bool debug);

  // mes méthodes pour mon projet
  int fetchColors(String today, String tomorrow, String afterTomorrow, String debutSaison);
  int fecthColorsFreeApi(String today, String tomorrow, String saison);
  int fetchPreviewRTE(String today, String tomorrow);
  void fetchAccessToken();
  String frenchColor(String color);

  // Les appels de Services RTE bruts
  String oauthService();
  String tempoLikeSupplyContractService(String startDate, String endDate);
  String previewRTEService();
  String tempoService(String saison);

private:
  String _client_secret;
  String _client_id;
  String _access_token;
  bool _debug;
};