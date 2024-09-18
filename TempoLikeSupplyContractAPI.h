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
  void setDebug(bool debug);
  int fetchColors(String today, String tomorrow, String afterTomorrow);
  String todayColor;
  String tomorrowColor;

protected:
  bool _debug;

private:
  int previewRTE(String tomorrow);
  void fetchAccessToken();
  String _client_secret;
  String _client_id;
  String _access_token;
  bool _authSuccess;
};