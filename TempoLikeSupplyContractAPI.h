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

#define USER_AGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36"
const int TIMEOUT = 30000;

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
  // RTEAPI
  String tempoLikeSupplyContractService(String startDate, String endDate);
  
  // preview RTE si soucis
  String previewRTEService();

  // FreeAPI
  String tempoService(String saison);
  int error_code[6] = {};
  
private:
  String _client_secret;
  String _client_id;
  String _access_token;
  int cptCall;
  bool _debug;
};