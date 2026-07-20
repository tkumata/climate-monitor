#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <NetworkClientSecure.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <EEPROM.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <cstring>
#include <esp_random.h>

// OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
// BME280
#include <Adafruit_BME280.h>

// ============================================
// ハードウェア設定
// ============================================

// Display (SSD1306 OLED)
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
constexpr uint8_t OLED_I2C_ADDRESS = 0x3C;

// Sensor (BME280)
#define USEIIC 0 // use SPI by default
#if (!USEIIC)
constexpr uint8_t SPI_SCK = 13;
constexpr uint8_t SPI_MISO = 12;
constexpr uint8_t SPI_MOSI = 11;
constexpr uint8_t SPI_CS = 10;
#endif

// ============================================
// ネットワーク / 時刻設定
// ============================================
constexpr char PRIMARY_NTP[] = "ntp.nict.jp";
constexpr char SECONDARY_NTP[] = "ntp.jst.mfeed.ad.jp";
constexpr long JST_GMT_OFFSET_SEC = 9L * 3600L;
constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000UL;
constexpr unsigned long WIFI_RETRY_DELAY_MS = 500UL;
constexpr unsigned long WIFI_RECONNECT_INTERVAL_MS = 30000UL;
constexpr uint8_t NTP_RETRY_MAX = 10;
constexpr unsigned long NTP_RETRY_DELAY_MS = 200UL;
constexpr unsigned long NTP_RESYNC_INTERVAL_MS = 60000UL;
constexpr uint8_t NTP_MAX_SYNC_ATTEMPTS = 5; // 最大同期試行回数
constexpr char TZ_JST[] = "JST-9";
constexpr time_t EPOCH_UNINITIALIZED = 0;
constexpr char AP_SSID[] = "ESP32AP";
constexpr char WEB_USERNAME[] = "admin";
constexpr char WEB_REALM[] = "Climate Monitor";
constexpr char ACCESS_KEY_ALPHABET[] = "abcdefghijkmnpqrstuvwxyz23456789";
constexpr size_t ACCESS_KEY_LENGTH = 12;
constexpr unsigned long ACCESS_INFO_DISPLAY_MS = 120000UL;
constexpr size_t MAX_SSID_LENGTH = 32;
constexpr size_t MAX_PASSWORD_LENGTH = 64;
constexpr size_t MAX_DEVICE_ID_LENGTH = 32;
constexpr size_t MAX_LOCATION_LENGTH = 9;
constexpr size_t MAX_INGEST_KEY_LENGTH = 64;
constexpr size_t MAX_GRAFANA_API_KEY_LENGTH = 256;
constexpr uint32_t LEGACY_CREDENTIAL_MAGIC = 0x54484D45;
constexpr uint32_t NEW_RELIC_CREDENTIAL_MAGIC = 0x54484D46;
constexpr uint32_t SEA_LEVEL_CREDENTIAL_MAGIC = 0x54484D47;
constexpr uint32_t ALTITUDE_CREDENTIAL_MAGIC = 0x54484D48;
constexpr uint32_t ACCESS_CREDENTIAL_MAGIC = 0x54484D49;
constexpr uint32_t CREDENTIAL_MAGIC = 0x54484D4A;
constexpr char EDGE_SERVER_URL[] = "http://192.168.10.103:8081/api/climate/save";
constexpr unsigned long EDGE_POST_INTERVAL_MS = 15UL * 60UL * 1000UL;
constexpr unsigned long EDGE_POST_TIMEOUT_MS = 5000UL;
constexpr char NEW_RELIC_URL[] = "https://metric-api.newrelic.com/metric/v1";
constexpr unsigned long NEW_RELIC_POST_INTERVAL_MS = 5UL * 60UL * 1000UL;
constexpr unsigned long NEW_RELIC_POST_TIMEOUT_MS = 5000UL;
constexpr char GRAFANA_OTLP_URL[] = "https://otlp-gateway-prod-ap-northeast-0.grafana.net/otlp/v1/metrics";
constexpr char GRAFANA_INSTANCE_ID[] = "1726789";
constexpr unsigned long GRAFANA_POST_INTERVAL_MS = 5UL * 60UL * 1000UL;
constexpr unsigned long GRAFANA_POST_TIMEOUT_MS = 5000UL;
constexpr char JMA_AMEDAS_URL_FORMAT[] = "https://www.jma.go.jp/bosai/amedas/data/point/44132/%04d%02d%02d_12.json";
constexpr unsigned long JMA_HTTP_TIMEOUT_MS = 8000UL;
constexpr unsigned long JMA_RETRY_INTERVAL_MS = 60UL * 60UL * 1000UL;
constexpr uint8_t JMA_FETCH_HOUR_JST = 13;
constexpr float JMA_PRESSURE_MIN_HPA = 850.0f;
constexpr float JMA_PRESSURE_MAX_HPA = 1100.0f;
constexpr char JMA_SEA_LEVEL_FIELD[] = "\"normalPressure\":[";
constexpr char SENSOR_NAME[] = "BME280";
constexpr char LOCATION_HOME[] = "home";
constexpr char LOCATION_APARTMENT[] = "apartment";
constexpr char LOCATION_OUTDOOR[] = "outdoor";
constexpr char LOCATION_OFFICE[] = "office";

constexpr char DIGICERT_GLOBAL_ROOT_G2[] PROGMEM = R"EOF(-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----)EOF";

constexpr char GLOBALSIGN_ROOT_R3[] PROGMEM = R"EOF(-----BEGIN CERTIFICATE-----
MIIDXzCCAkegAwIBAgILBAAAAAABIVhTCKIwDQYJKoZIhvcNAQELBQAwTDEgMB4
GA1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjMxEzARBgNVBAoTCkdsb2JhbF
NpZ24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDkwMzE4MTAwMDAwWhcNMjkw
MzE4MTAwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzET
MBEGA1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQY
JKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMwldpB5BngiFvXAg7aEyiie/QV2Ec
WtiHL8RgJDx7KKnQRfJMsuS+FggkbhUqsMgUdwbN1k0ev1LKMPgj0MK66X17YUh
hB5uzsTgHeMCOFJ0mpiLx9e+pZo34knlTifBtc+ycsmWQ1z3rDI6SYOgxXG71uL
0gRgykmmKPZpO/bLyCiR5Z2KYVc3rHQU3HTgOu5yLy6c+9C7v/U9AOEGM+iCK65
TpjoWc4zdQQ4gOsC0p6Hpsk+QLjJg6VfLuQSSaGjlOCZgdbKfd/+RFO+uIEn8rU
AVSNECMWEZXriX7613t2Saer9fwRPvm2L7DWzgVGkWqQPabumDk3F2xmmFghcCA
wEAAaNCMEAwDgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0O
BBYEFI/wS3+oLkUkrk1Q+mOai97i3Ru8MA0GCSqGSIb3DQEBCwUAA4IBAQBLQNv
AUKr+yAzv95ZURUm7lgAJQayzE4aGKAczymvmdLm6AC2upArT9fHxD4q/c2dKg8
dEe3jgr25sbwMpjjM5RcOO5LlXbKr8EpbsU8Yt5CRsuZRj+9xTaGdWPoO4zzUhw
8lo/s7awlOqzJCK6fBdRoyV3XpYKBovHd7NADdBj+1EbddTKJd+82cEHhXXipa0
095MJ6RMG3NzdvQXmcIfeg7jLQitChws/zyrVQ4PkX4268NXSb7hLi18YIvDQVE
TI53O9zJrlAGomecsMx86OyXShkDOOyyGeMlhLxS67ttVb9+E7gUJTb0o2HLO02
JQZR7rkpeDMdmztcpHWD9f
-----END CERTIFICATE-----)EOF";

// PROGMEM strings for web server
constexpr char HTTP_HEADER_HTML[] PROGMEM = "text/html";
constexpr char HTTP_HEADER_PLAIN[] PROGMEM = "text/plain";
constexpr char ERR_MISSING_PARAMS[] PROGMEM = "Missing ssid or password";
constexpr char ERR_SSID_REQUIRED[] PROGMEM = "SSID is required";
constexpr char ERR_INPUT_TOO_LONG[] PROGMEM = "Input too long";
constexpr char ERR_INVALID_DEVICE_ID[] PROGMEM = "Device ID must contain only ASCII letters and digits";
constexpr char ERR_INVALID_LOCATION[] PROGMEM = "Invalid location";
constexpr char ERR_SAVE_FAILED[] PROGMEM = "Failed to save credentials";
constexpr char ERR_CLEAR_FAILED[] PROGMEM = "Failed to clear credentials";
constexpr char ERR_NOT_FOUND[] PROGMEM = "Not found";

struct LegacyCredentialStorage
{
  uint32_t magic;
  char ssid[MAX_SSID_LENGTH + 1];
  char password[MAX_PASSWORD_LENGTH + 1];
  float humidityOffset;
};

struct NewRelicCredentialStorage
{
  uint32_t magic;
  char ssid[MAX_SSID_LENGTH + 1];
  char password[MAX_PASSWORD_LENGTH + 1];
  float humidityOffset;
  char deviceId[MAX_DEVICE_ID_LENGTH + 1];
  char location[MAX_LOCATION_LENGTH + 1];
  char ingestKey[MAX_INGEST_KEY_LENGTH + 1];
};

struct SeaLevelCredentialStorage
{
  uint32_t magic;
  char ssid[MAX_SSID_LENGTH + 1];
  char password[MAX_PASSWORD_LENGTH + 1];
  float humidityOffset;
  char deviceId[MAX_DEVICE_ID_LENGTH + 1];
  char location[MAX_LOCATION_LENGTH + 1];
  char ingestKey[MAX_INGEST_KEY_LENGTH + 1];
  float seaLevelPressureHpa;
  uint32_t seaLevelPressureDate;
  uint32_t jmaAttemptDate;
};

struct AltitudeCredentialStorage
{
  uint32_t magic;
  char ssid[MAX_SSID_LENGTH + 1];
  char password[MAX_PASSWORD_LENGTH + 1];
  float humidityOffset;
  char deviceId[MAX_DEVICE_ID_LENGTH + 1];
  char location[MAX_LOCATION_LENGTH + 1];
  char ingestKey[MAX_INGEST_KEY_LENGTH + 1];
  float seaLevelPressureHpa;
  uint32_t seaLevelPressureDate;
  float altitude;
};

struct CredentialStorage
{
  uint32_t magic;
  char ssid[MAX_SSID_LENGTH + 1];
  char password[MAX_PASSWORD_LENGTH + 1];
  float humidityOffset;
  char deviceId[MAX_DEVICE_ID_LENGTH + 1];
  char location[MAX_LOCATION_LENGTH + 1];
  char ingestKey[MAX_INGEST_KEY_LENGTH + 1];
  float seaLevelPressureHpa;
  uint32_t seaLevelPressureDate;
  float altitude;
  char accessKey[ACCESS_KEY_LENGTH + 1];
  char grafanaApiKey[MAX_GRAFANA_API_KEY_LENGTH + 1];
};

struct AccessCredentialStorage
{
  uint32_t magic;
  char ssid[MAX_SSID_LENGTH + 1];
  char password[MAX_PASSWORD_LENGTH + 1];
  float humidityOffset;
  char deviceId[MAX_DEVICE_ID_LENGTH + 1];
  char location[MAX_LOCATION_LENGTH + 1];
  char ingestKey[MAX_INGEST_KEY_LENGTH + 1];
  float seaLevelPressureHpa;
  uint32_t seaLevelPressureDate;
  float altitude;
  char accessKey[ACCESS_KEY_LENGTH + 1];
};

constexpr size_t EEPROM_SIZE = sizeof(CredentialStorage);

// ============================================
// 表示設定
// ============================================
constexpr char LABEL_TEMP[] PROGMEM = "T:";
constexpr char LABEL_HUMID[] PROGMEM = "RH:";
constexpr char LABEL_PRESSURE[] PROGMEM = "Pressure";
constexpr char DATE_FORMAT[] PROGMEM = "%Y/%m/%d %a %H:%M";
constexpr uint8_t HEADER_TEXT_SIZE = 1;
constexpr uint8_t METRIC_TEXT_SIZE = 2;
constexpr uint8_t LABEL_TEXT_SIZE = 1;
constexpr uint8_t DATE_TEXT_SIZE = 1;

// ============================================
// ハードウェアオブジェクト
// ============================================
Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

#if (USEIIC)
Adafruit_BME280 bme;
#else
Adafruit_BME280 bme(SPI_CS, SPI_MOSI, SPI_MISO, SPI_SCK);
#endif

namespace
{
  CredentialStorage credentialStorage{};
  WebServer configServer(80);
  bool timeSynced = false;
  unsigned long lastWifiAttemptMs = 0;
  unsigned long lastNtpAttemptMs = 0;
  bool oledAvailable = false;
  bool bmeAvailable = false;
  uint8_t sensorInitRetries = 0;
  constexpr uint8_t SENSOR_INIT_MAX_RETRIES = 3;
  bool wifiDisconnected = false;
  uint8_t ntpSyncAttempts = 0;
  String csrfToken;
  unsigned long accessInfoStartedMs = 0;

  void onWiFiEvent(WiFiEvent_t event)
  {
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      wifiDisconnected = true;
      timeSynced = false;
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      wifiDisconnected = false;
      break;
    default:
      break;
    }
  }

  String generateCsrfToken()
  {
    String token;
    for (int i = 0; i < 16; i++)
    {
      token += String(random(0, 16), HEX);
    }
    return token;
  }

  String htmlEscape(const char *text)
  {
    String escaped;
    if (!text)
    {
      return escaped;
    }

    for (size_t i = 0; text[i] != '\0'; ++i)
    {
      const char c = text[i];
      switch (c)
      {
      case '&':
        escaped += "&amp;";
        break;
      case '<':
        escaped += "&lt;";
        break;
      case '>':
        escaped += "&gt;";
        break;
      case '"':
        escaped += "&quot;";
        break;
      case '\'':
        escaped += "&#39;";
        break;
      default:
        escaped += c;
        break;
      }
    }
    return escaped;
  }

  bool isValidDeviceId(const String &deviceId)
  {
    if (deviceId.length() > MAX_DEVICE_ID_LENGTH)
    {
      return false;
    }

    for (size_t i = 0; i < deviceId.length(); ++i)
    {
      const char c = deviceId[i];
      if (!((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z')))
      {
        return false;
      }
    }
    return true;
  }

  bool isAllowedLocation(const char *location)
  {
    return strcmp(location, LOCATION_HOME) == 0 ||
           strcmp(location, LOCATION_APARTMENT) == 0 ||
           strcmp(location, LOCATION_OUTDOOR) == 0 ||
           strcmp(location, LOCATION_OFFICE) == 0;
  }

  bool isValidLocation(const String &location)
  {
    return location.length() == 0 || isAllowedLocation(location.c_str());
  }

  void terminateCredentialStrings(CredentialStorage &credentials)
  {
    credentials.ssid[MAX_SSID_LENGTH] = '\0';
    credentials.password[MAX_PASSWORD_LENGTH] = '\0';
    credentials.deviceId[MAX_DEVICE_ID_LENGTH] = '\0';
    credentials.location[MAX_LOCATION_LENGTH] = '\0';
    credentials.ingestKey[MAX_INGEST_KEY_LENGTH] = '\0';
    credentials.accessKey[ACCESS_KEY_LENGTH] = '\0';
    credentials.grafanaApiKey[MAX_GRAFANA_API_KEY_LENGTH] = '\0';
  }

  bool isValidAccessKey(const char *accessKey)
  {
    if (strlen(accessKey) != ACCESS_KEY_LENGTH)
    {
      return false;
    }
    for (size_t i = 0; i < ACCESS_KEY_LENGTH; ++i)
    {
      if (strchr(ACCESS_KEY_ALPHABET, accessKey[i]) == nullptr)
      {
        return false;
      }
    }
    return true;
  }

  void generateAccessKey(char *accessKey)
  {
    for (size_t i = 0; i < ACCESS_KEY_LENGTH; ++i)
    {
      accessKey[i] = ACCESS_KEY_ALPHABET[esp_random() & 0x1f];
    }
    accessKey[ACCESS_KEY_LENGTH] = '\0';
  }

  void initCredentialStorage()
  {
    static bool initialized = false;
    if (initialized)
    {
      return;
    }

    EEPROM.begin(EEPROM_SIZE);

    uint32_t storedMagic = 0;
    EEPROM.get(0, storedMagic);
    bool needsCommit = false;
    if (storedMagic == CREDENTIAL_MAGIC)
    {
      EEPROM.get(0, credentialStorage);
      terminateCredentialStrings(credentialStorage);
    }
    else if (storedMagic == ACCESS_CREDENTIAL_MAGIC)
    {
      AccessCredentialStorage previousCredentials{};
      EEPROM.get(0, previousCredentials);

      memset(&credentialStorage, 0, sizeof(credentialStorage));
      memcpy(&credentialStorage, &previousCredentials, sizeof(previousCredentials));
      credentialStorage.magic = CREDENTIAL_MAGIC;
      credentialStorage.grafanaApiKey[0] = '\0';
      terminateCredentialStrings(credentialStorage);
      needsCommit = true;
    }
    else if (storedMagic == ALTITUDE_CREDENTIAL_MAGIC)
    {
      AltitudeCredentialStorage previousCredentials{};
      EEPROM.get(0, previousCredentials);

      memset(&credentialStorage, 0, sizeof(credentialStorage));
      memcpy(&credentialStorage, &previousCredentials, sizeof(previousCredentials));
      credentialStorage.magic = CREDENTIAL_MAGIC;
      terminateCredentialStrings(credentialStorage);
      needsCommit = true;
    }
    else if (storedMagic == SEA_LEVEL_CREDENTIAL_MAGIC)
    {
      SeaLevelCredentialStorage previousCredentials{};
      EEPROM.get(0, previousCredentials);

      memset(&credentialStorage, 0, sizeof(credentialStorage));
      memcpy(&credentialStorage, &previousCredentials, sizeof(previousCredentials));
      credentialStorage.magic = CREDENTIAL_MAGIC;
      credentialStorage.altitude = NAN;
      terminateCredentialStrings(credentialStorage);
      needsCommit = true;
    }
    else if (storedMagic == NEW_RELIC_CREDENTIAL_MAGIC)
    {
      NewRelicCredentialStorage previousCredentials{};
      EEPROM.get(0, previousCredentials);
      previousCredentials.ssid[MAX_SSID_LENGTH] = '\0';
      previousCredentials.password[MAX_PASSWORD_LENGTH] = '\0';
      previousCredentials.deviceId[MAX_DEVICE_ID_LENGTH] = '\0';
      previousCredentials.location[MAX_LOCATION_LENGTH] = '\0';
      previousCredentials.ingestKey[MAX_INGEST_KEY_LENGTH] = '\0';

      memset(&credentialStorage, 0, sizeof(credentialStorage));
      credentialStorage.magic = CREDENTIAL_MAGIC;
      memcpy(credentialStorage.ssid, previousCredentials.ssid, sizeof(credentialStorage.ssid));
      memcpy(credentialStorage.password, previousCredentials.password, sizeof(credentialStorage.password));
      credentialStorage.humidityOffset = previousCredentials.humidityOffset;
      memcpy(credentialStorage.deviceId, previousCredentials.deviceId, sizeof(credentialStorage.deviceId));
      memcpy(credentialStorage.location, previousCredentials.location, sizeof(credentialStorage.location));
      memcpy(credentialStorage.ingestKey, previousCredentials.ingestKey, sizeof(credentialStorage.ingestKey));
      credentialStorage.altitude = NAN;
      needsCommit = true;
    }
    else if (storedMagic == LEGACY_CREDENTIAL_MAGIC)
    {
      LegacyCredentialStorage legacyCredentials{};
      EEPROM.get(0, legacyCredentials);
      legacyCredentials.ssid[MAX_SSID_LENGTH] = '\0';
      legacyCredentials.password[MAX_PASSWORD_LENGTH] = '\0';

      memset(&credentialStorage, 0, sizeof(credentialStorage));
      credentialStorage.magic = CREDENTIAL_MAGIC;
      memcpy(credentialStorage.ssid, legacyCredentials.ssid, sizeof(credentialStorage.ssid));
      memcpy(credentialStorage.password, legacyCredentials.password, sizeof(credentialStorage.password));
      credentialStorage.humidityOffset = legacyCredentials.humidityOffset;
      credentialStorage.altitude = NAN;
      needsCommit = true;
    }
    else
    {
      memset(&credentialStorage, 0, sizeof(credentialStorage));
      credentialStorage.magic = CREDENTIAL_MAGIC;
      credentialStorage.humidityOffset = 0.0f; // Default offset
      credentialStorage.altitude = NAN;
      needsCommit = true;
    }

    if (!isValidAccessKey(credentialStorage.accessKey))
    {
      generateAccessKey(credentialStorage.accessKey);
      needsCommit = true;
    }
    if (needsCommit)
    {
      EEPROM.put(0, credentialStorage);
      EEPROM.commit();
    }

    initialized = true;
  }

  bool credentialsAvailable()
  {
    return credentialStorage.ssid[0] != '\0';
  }

  bool saveCredentialsToEeprom(const String &ssid, const String &password, float humidOffset,
                               const String &deviceId, const String &location, const String &ingestKey,
                               const String &grafanaApiKey)
  {
    CredentialStorage nextCredentials = credentialStorage;
    nextCredentials.magic = CREDENTIAL_MAGIC;
    memset(nextCredentials.ssid, 0, sizeof(nextCredentials.ssid));
    memset(nextCredentials.deviceId, 0, sizeof(nextCredentials.deviceId));
    memset(nextCredentials.location, 0, sizeof(nextCredentials.location));
    ssid.toCharArray(nextCredentials.ssid, sizeof(nextCredentials.ssid));
    if (password.length() > 0)
    {
      memset(nextCredentials.password, 0, sizeof(nextCredentials.password));
      password.toCharArray(nextCredentials.password, sizeof(nextCredentials.password));
    }
    nextCredentials.humidityOffset = humidOffset;
    deviceId.toCharArray(nextCredentials.deviceId, sizeof(nextCredentials.deviceId));
    location.toCharArray(nextCredentials.location, sizeof(nextCredentials.location));
    if (ingestKey.length() > 0)
    {
      memset(nextCredentials.ingestKey, 0, sizeof(nextCredentials.ingestKey));
      ingestKey.toCharArray(nextCredentials.ingestKey, sizeof(nextCredentials.ingestKey));
    }
    if (grafanaApiKey.length() > 0)
    {
      memset(nextCredentials.grafanaApiKey, 0, sizeof(nextCredentials.grafanaApiKey));
      grafanaApiKey.toCharArray(nextCredentials.grafanaApiKey, sizeof(nextCredentials.grafanaApiKey));
    }

    // 変更がない場合は書き込まない（EEPROM/Flash寿命対策）
    if (credentialStorage.magic == nextCredentials.magic &&
        strcmp(credentialStorage.ssid, nextCredentials.ssid) == 0 &&
        strcmp(credentialStorage.password, nextCredentials.password) == 0 &&
        abs(credentialStorage.humidityOffset - nextCredentials.humidityOffset) < 0.01f &&
        strcmp(credentialStorage.deviceId, nextCredentials.deviceId) == 0 &&
        strcmp(credentialStorage.location, nextCredentials.location) == 0 &&
        strcmp(credentialStorage.ingestKey, nextCredentials.ingestKey) == 0 &&
        strcmp(credentialStorage.grafanaApiKey, nextCredentials.grafanaApiKey) == 0)
    {
      return true; // 変更なし
    }

    EEPROM.put(0, nextCredentials);
    if (!EEPROM.commit())
    {
      return false;
    }
    credentialStorage = nextCredentials;
    return true;
  }

  bool clearCredentialsFromEeprom()
  {
    CredentialStorage emptyCredentials{};
    emptyCredentials.magic = CREDENTIAL_MAGIC;
    emptyCredentials.altitude = NAN;
    memcpy(emptyCredentials.accessKey, credentialStorage.accessKey, sizeof(emptyCredentials.accessKey));
    EEPROM.put(0, emptyCredentials);
    if (!EEPROM.commit())
    {
      return false;
    }
    credentialStorage = emptyCredentials;
    return true;
  }

  void startAccessPoint()
  {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID, credentialStorage.accessKey);
  }

  bool authenticateConfigRequest()
  {
    if (configServer.header(F("Authorization")).startsWith(configServer.AuthTypeDigest) &&
        configServer.authenticate(WEB_USERNAME, credentialStorage.accessKey))
    {
      return true;
    }
    configServer.requestAuthentication(DIGEST_AUTH, WEB_REALM);
    return false;
  }

  void handleRoot();
  void handleSave();
  void handleClear();
  void handleNotFound();

  void setupConfigServer()
  {
    configServer.on("/", HTTP_GET, handleRoot);
    configServer.on("/save", HTTP_POST, handleSave);
    configServer.on("/clear", HTTP_POST, handleClear);
    configServer.onNotFound(handleNotFound);
    configServer.begin();
  }

  bool connectWifiSta()
  {
    if (!credentialsAvailable())
    {
      return false;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }

    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect();
    delay(50);

    if (credentialStorage.password[0] == '\0')
    {
      WiFi.begin(credentialStorage.ssid);
    }
    else
    {
      WiFi.begin(credentialStorage.ssid, credentialStorage.password);
    }
    WiFi.setAutoReconnect(true);

    const unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) <= WIFI_CONNECT_TIMEOUT_MS)
    {
      configServer.handleClient();
      delay(WIFI_RETRY_DELAY_MS);
    }

    lastWifiAttemptMs = millis();
    return WiFi.status() == WL_CONNECTED;
  }

  bool syncTimeFromNtp()
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      return false;
    }

    // 最大試行回数に達している場合は諦める
    if (ntpSyncAttempts >= NTP_MAX_SYNC_ATTEMPTS)
    {
      return false;
    }

    configTime(JST_GMT_OFFSET_SEC, 0, PRIMARY_NTP, SECONDARY_NTP);
    setenv("TZ", TZ_JST, 1);
    tzset();

    struct tm timeinfo;
    for (uint8_t attempt = 0; attempt < NTP_RETRY_MAX; ++attempt)
    {
      if (getLocalTime(&timeinfo, 0))
      {
        lastNtpAttemptMs = millis();
        ntpSyncAttempts = 0; // 成功したらリセット
        return true;
      }
      configServer.handleClient();
      delay(NTP_RETRY_DELAY_MS);
    }
    lastNtpAttemptMs = millis();
    ntpSyncAttempts++;
    return false;
  }

  void ensureWifiConnection()
  {
    // 切断イベントを検知した場合は即座に再接続を試みる
    if (wifiDisconnected && credentialsAvailable())
    {
      const unsigned long now = millis();
      if ((now - lastWifiAttemptMs) >= WIFI_RECONNECT_INTERVAL_MS)
      {
        connectWifiSta();
      }
      return;
    }

    if (WiFi.status() == WL_CONNECTED || !credentialsAvailable())
    {
      return;
    }

    const unsigned long now = millis();
    if ((now - lastWifiAttemptMs) < WIFI_RECONNECT_INTERVAL_MS)
    {
      return;
    }

    connectWifiSta();
  }

  void ensureTimeSync()
  {
    if (timeSynced || WiFi.status() != WL_CONNECTED)
    {
      return;
    }

    const unsigned long now = millis();
    if ((now - lastNtpAttemptMs) < NTP_RESYNC_INTERVAL_MS)
    {
      return;
    }

    timeSynced = syncTimeFromNtp();
  }

  uint32_t dateKey(const struct tm &date)
  {
    return static_cast<uint32_t>(date.tm_year + 1900) * 10000UL +
           static_cast<uint32_t>(date.tm_mon + 1) * 100UL +
           static_cast<uint32_t>(date.tm_mday);
  }

  float computeAltitude(float temperatureC, float pressureHpa, float seaLevelPressureHpa);

  bool parseSeaLevelPressure(const char *valueText, const char *qualityText, float &pressureHpa)
  {
    char *end = nullptr;
    const float value = strtof(valueText, &end);
    if (end == valueText || *end != '\0')
    {
      return false;
    }
    char *qualityEnd = nullptr;
    const long quality = strtol(qualityText, &qualityEnd, 10);
    if (qualityEnd == qualityText || *qualityEnd != '\0' || (quality != 0 && quality != 1))
    {
      return false;
    }
    if (!isfinite(value) || value < JMA_PRESSURE_MIN_HPA || value > JMA_PRESSURE_MAX_HPA)
    {
      return false;
    }
    pressureHpa = value;
    return true;
  }

  bool saveAltitudeToEeprom(float pressureHpa, uint32_t sourceDate, float altitude)
  {
    CredentialStorage nextCredentials = credentialStorage;
    nextCredentials.magic = CREDENTIAL_MAGIC;
    nextCredentials.seaLevelPressureHpa = pressureHpa;
    nextCredentials.seaLevelPressureDate = sourceDate;
    nextCredentials.altitude = altitude;
    EEPROM.put(0, nextCredentials);
    if (!EEPROM.commit())
    {
      return false;
    }
    credentialStorage = nextCredentials;
    return true;
  }

  bool fetchTokyoSeaLevelPressure(const struct tm &sourceDate, float &pressureHpa)
  {
    char expectedTimestamp[20];
    snprintf(expectedTimestamp, sizeof(expectedTimestamp), "\"%04d%02d%02d120000\"",
             sourceDate.tm_year + 1900, sourceDate.tm_mon + 1, sourceDate.tm_mday);
    char url[128];
    snprintf(url, sizeof(url), JMA_AMEDAS_URL_FORMAT,
             sourceDate.tm_year + 1900, sourceDate.tm_mon + 1, sourceDate.tm_mday);

    NetworkClientSecure client;
    client.setCACert(GLOBALSIGN_ROOT_R3);
    client.setTimeout(JMA_HTTP_TIMEOUT_MS);

    HTTPClient http;
    http.setTimeout(JMA_HTTP_TIMEOUT_MS);
    if (!http.begin(client, url))
    {
      return false;
    }

    const int status = http.GET();
    if (status != HTTP_CODE_OK)
    {
      http.end();
      return false;
    }

    NetworkClient *stream = http.getStreamPtr();
    char valueText[16];
    char qualityText[4];
    bool parsed = false;
    if (stream != nullptr &&
        stream->find(expectedTimestamp) &&
        stream->find(JMA_SEA_LEVEL_FIELD))
    {
      const size_t valueLength = stream->readBytesUntil(',', valueText, sizeof(valueText) - 1);
      valueText[valueLength] = '\0';
      const size_t qualityLength = stream->readBytesUntil(']', qualityText, sizeof(qualityText) - 1);
      qualityText[qualityLength] = '\0';
      parsed = valueLength < sizeof(valueText) - 1 && qualityLength < sizeof(qualityText) - 1 &&
               parseSeaLevelPressure(valueText, qualityText, pressureHpa);
    }
    http.end();
    return parsed;
  }

  void ensureDailyAltitude()
  {
    static unsigned long lastAttemptMs = 0;
    if (!timeSynced || WiFi.status() != WL_CONNECTED || !bmeAvailable)
    {
      return;
    }

    struct tm now;
    if (!getLocalTime(&now, 0))
    {
      return;
    }

    const uint32_t today = dateKey(now);
    if (credentialStorage.seaLevelPressureDate == today && isfinite(credentialStorage.altitude))
    {
      return;
    }

    if (now.tm_hour < JMA_FETCH_HOUR_JST)
    {
      return;
    }

    const unsigned long nowMs = millis();
    if (lastAttemptMs != 0 && (nowMs - lastAttemptMs) < JMA_RETRY_INTERVAL_MS)
    {
      return;
    }
    lastAttemptMs = nowMs;

    float pressureHpa = 0.0f;
    if (!fetchTokyoSeaLevelPressure(now, pressureHpa))
    {
      return;
    }

    const float temperatureC = bme.readTemperature();
    const float measuredPressureHpa = bme.readPressure() / 100.0f;
    const float altitude = computeAltitude(temperatureC, measuredPressureHpa, pressureHpa);
    if (isfinite(altitude))
    {
      saveAltitudeToEeprom(pressureHpa, today, altitude);
    }
  }

  void appendLocationOption(String &page, const char *location)
  {
    page += F("<option value=\"");
    page += location;
    page += '"';
    if (strcmp(credentialStorage.location, location) == 0)
    {
      page += F(" selected");
    }
    page += '>';
    page += location;
    page += F("</option>");
  }

  void handleRoot()
  {
    initCredentialStorage();
    if (!authenticateConfigRequest())
    {
      return;
    }

    // CSRFトークンを生成
    csrfToken = generateCsrfToken();

    String page;
    page.reserve(1700);
    page += F("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>TempHumid Wi-Fi</title><style>body{font-family:sans-serif;margin:24px;}form{display:flex;flex-direction:column;gap:12px;max-width:320px;margin-bottom:16px;}label{display:flex;flex-direction:column;font-weight:600;}input,select{padding:8px;font-size:16px;}button{padding:10px;font-size:16px;}button.danger{background:#c62828;color:#fff;border:none;}button.secondary{background:#f0f0f0;border:1px solid #ccc;}</style></head><body>");
    page += F("<h1>Wi-Fi Settings</h1><form method=\"POST\" action=\"/save\"><input type=\"hidden\" name=\"csrf\" value=\"");
    page += csrfToken;
    page += F("\"><label>SSID<input type=\"text\" name=\"ssid\" maxlength=\"32\" required value=\"");
    page += htmlEscape(credentialStorage.ssid);
    page += F("\"></label><label>Password<input type=\"password\" name=\"password\" maxlength=\"64\" autocomplete=\"new-password\" placeholder=\"");
    page += credentialStorage.password[0] != '\0' ? F("Configured (leave blank to keep)") : F("Not configured");
    page += F("\"></label><label>Humidity Offset (%)<input type=\"number\" step=\"0.1\" name=\"humid_offset\" value=\"");
    page += String(credentialStorage.humidityOffset, 1);
    page += F("\"></label><h2>New Relic Settings</h2><label>Device ID<input type=\"text\" name=\"device_id\" maxlength=\"32\" pattern=\"[A-Za-z0-9]+\" value=\"");
    page += htmlEscape(credentialStorage.deviceId);
    page += F("\"></label><label>Location<select name=\"location\"><option value=\"\">Not selected</option>");
    appendLocationOption(page, LOCATION_HOME);
    appendLocationOption(page, LOCATION_APARTMENT);
    appendLocationOption(page, LOCATION_OUTDOOR);
    appendLocationOption(page, LOCATION_OFFICE);
    page += F("</select></label><label>Ingest Key<input type=\"password\" name=\"ingest_key\" maxlength=\"64\" autocomplete=\"new-password\" placeholder=\"");
    page += credentialStorage.ingestKey[0] != '\0' ? F("Configured (leave blank to keep)") : F("Not configured");
    page += F("\"></label><h2>Grafana Settings</h2><label>API Key<input type=\"password\" name=\"grafana_api_key\" maxlength=\"256\" autocomplete=\"new-password\" placeholder=\"");
    page += credentialStorage.grafanaApiKey[0] != '\0' ? F("Configured (leave blank to keep)") : F("Not configured");
    page += F("\"></label><button type=\"submit\">Save</button></form>");

    page += F("<form method=\"POST\" action=\"/clear\"><input type=\"hidden\" name=\"csrf\" value=\"");
    page += csrfToken;
    page += F("\">");
    if (credentialsAvailable())
    {
      page += F("<p>Saved credentials detected.</p>");
      page += F("<button type=\"submit\" class=\"danger\">Delete Stored Credentials</button>");
    }
    else
    {
      page += F("<p>No saved credentials.</p>");
      page += F("<button type=\"submit\" class=\"secondary\" disabled>Delete Stored Credentials</button>");
    }
    page += F("</form>");

    if (WiFi.status() == WL_CONNECTED)
    {
      page += F("<p>Current status: connected to <strong>");
      String currentStaSsid = WiFi.SSID();
      page += htmlEscape(currentStaSsid.c_str());
      page += F("</strong></p>");
    }
    else
    {
      page += F("<p>Current status: not connected</p>");
    }

    IPAddress apIp = WiFi.softAPIP();
    page += F("<p>AP IP: ");
    page += apIp.toString();
    page += F("</p></body></html>");

    configServer.send(200, "text/html", page);
  }

  void handleSave()
  {
    initCredentialStorage();
    if (!authenticateConfigRequest())
    {
      return;
    }

    // CSRFトークンチェック
    if (!configServer.hasArg("csrf") || configServer.arg("csrf") != csrfToken)
    {
      configServer.send(403, FPSTR(HTTP_HEADER_PLAIN), F("Invalid CSRF token"));
      return;
    }

    if (!configServer.hasArg("ssid") || !configServer.hasArg("password"))
    {
      configServer.send(400, FPSTR(HTTP_HEADER_PLAIN), FPSTR(ERR_MISSING_PARAMS));
      return;
    }

    String ssid = configServer.arg("ssid");
    String password = configServer.arg("password");
    String deviceId = configServer.arg("device_id");
    String location = configServer.arg("location");
    String ingestKey = configServer.arg("ingest_key");
    String grafanaApiKey = configServer.arg("grafana_api_key");
    ssid.trim();
    password.trim();
    deviceId.trim();
    location.trim();
    ingestKey.trim();
    grafanaApiKey.trim();

    if (ssid.length() == 0)
    {
      configServer.send(400, FPSTR(HTTP_HEADER_PLAIN), FPSTR(ERR_SSID_REQUIRED));
      return;
    }

    if (ssid.length() > MAX_SSID_LENGTH || password.length() > MAX_PASSWORD_LENGTH ||
        ingestKey.length() > MAX_INGEST_KEY_LENGTH ||
        grafanaApiKey.length() > MAX_GRAFANA_API_KEY_LENGTH)
    {
      configServer.send(400, FPSTR(HTTP_HEADER_PLAIN), FPSTR(ERR_INPUT_TOO_LONG));
      return;
    }

    if (!isValidDeviceId(deviceId))
    {
      configServer.send(400, FPSTR(HTTP_HEADER_PLAIN), FPSTR(ERR_INVALID_DEVICE_ID));
      return;
    }

    if (!isValidLocation(location))
    {
      configServer.send(400, FPSTR(HTTP_HEADER_PLAIN), FPSTR(ERR_INVALID_LOCATION));
      return;
    }

    float humidOffset = 0.0f; // default fallback
    if (configServer.hasArg("humid_offset"))
    {
      humidOffset = configServer.arg("humid_offset").toFloat();
    }

    if (!saveCredentialsToEeprom(ssid, password, humidOffset, deviceId, location, ingestKey,
                                 grafanaApiKey))
    {
      configServer.send(500, FPSTR(HTTP_HEADER_PLAIN), FPSTR(ERR_SAVE_FAILED));
      return;
    }

    bool connected = connectWifiSta();
    if (connected)
    {
      timeSynced = syncTimeFromNtp();
    }
    else
    {
      timeSynced = false;
    }

    String page;
    page.reserve(384);
    page += F("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>TempHumid Wi-Fi</title></head><body>");
    page += F("<h1>Configuration Saved</h1>");
    page += F("<p>SSID: ");
    page += htmlEscape(ssid.c_str());
    page += F("</p>");
    page += F("<p>Device ID: ");
    page += deviceId.length() > 0 ? htmlEscape(deviceId.c_str()) : F("Not configured");
    page += F("</p><p>Location: ");
    page += location.length() > 0 ? htmlEscape(location.c_str()) : F("Not configured");
    page += F("</p>");
    page += F("<p>Status: ");
    page += connected ? F("Connected") : F("Not connected yet");
    page += F("</p><p><a href=\"/\">Back</a></p></body></html>");

    configServer.send(200, "text/html", page);
  }

  void handleClear()
  {
    initCredentialStorage();
    if (!authenticateConfigRequest())
    {
      return;
    }

    // CSRFトークンチェック
    if (!configServer.hasArg("csrf") || configServer.arg("csrf") != csrfToken)
    {
      configServer.send(403, FPSTR(HTTP_HEADER_PLAIN), F("Invalid CSRF token"));
      return;
    }

    if (!clearCredentialsFromEeprom())
    {
      configServer.send(500, FPSTR(HTTP_HEADER_PLAIN), FPSTR(ERR_CLEAR_FAILED));
      return;
    }

    WiFi.disconnect(true);
    timeSynced = false;

    String page;
    page.reserve(320);
    page += F("<!DOCTYPE html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>TempHumid Wi-Fi</title></head><body>");
    page += F("<h1>Credentials Deleted</h1>");
    page += F("<p>Saved Wi-Fi, New Relic, Grafana, and weather settings have been removed from EEPROM.</p>");
    page += F("<p><a href=\"/\">Back</a></p>");
    page += F("</body></html>");

    configServer.send(200, "text/html", page);
  }

  void handleNotFound()
  {
    if (!authenticateConfigRequest())
    {
      return;
    }
    configServer.send(404, FPSTR(HTTP_HEADER_PLAIN), FPSTR(ERR_NOT_FOUND));
  }

  struct SensorData
  {
    float temperature;
    float humidity;
    float pressure;
    float altitude;
  };

  struct ComfortMetrics
  {
    float absoluteHumidity;
    float vaporPressureDeficit;
    float comfortIndex;
    float wbgt;
  };

  constexpr unsigned long DEFAULT_DELAY_MS = 1000UL;
  constexpr uint8_t HEADER_Y = 0;
  constexpr uint8_t HEADER_LINE_Y = 15;
  constexpr uint8_t COL_LEFT_X = 0;
  constexpr uint8_t COL_RIGHT_X = 80;
  constexpr uint8_t LEFT_TEMPERATURE_Y = 16;
  constexpr uint8_t LEFT_WBGT_Y = 32;
  constexpr uint8_t LEFT_DI_Y = 48;
  constexpr uint8_t RIGHT_PRESSURE_Y = 16;
  constexpr uint8_t RIGHT_ALTITUDE_Y = 25;
  constexpr uint8_t RIGHT_HUMIDITY_Y = 34;
  constexpr uint8_t RIGHT_ABSOLUTE_HUMIDITY_Y = 43;
  constexpr uint8_t RIGHT_VPD_Y = 52;
  constexpr uint8_t DIVIDER_LINE_X = 78;
  constexpr uint8_t DATE_Y = 8;
  constexpr uint8_t HEAT_INDEX_Y = 36;
  constexpr size_t DATE_BUFFER_SIZE = 21; // フォーマット長(20) + 終端
  constexpr uint8_t DIVIDER_LINE_TOP_Y = 16;
  constexpr uint8_t DIVIDER_LINE_BOTTOM_Y = 64;
  constexpr float KELVIN_OFFSET = 273.15f;
  constexpr float ABS_HUMID_NUMERATOR = 216.7f;
  constexpr float SATURATION_COEFF_A = 17.67f;
  constexpr float SATURATION_COEFF_B = 243.5f;
  constexpr float SATURATION_BASE = 6.112f;
  constexpr float HPA_TO_KPA = 0.1f;
  constexpr float DISCOMFORT_TEMP_COEFF = 0.81f;
  constexpr float DISCOMFORT_RH_COEFF = 0.01f;
  constexpr float DISCOMFORT_RH_TEMP_COEFF = 0.99f;
  constexpr float DISCOMFORT_TEMP_OFFSET = 14.3f;
  constexpr float DISCOMFORT_CONSTANT = 46.3f;
  constexpr float ALTITUDE_EXPONENT = 1.0f / 5.257f;
  constexpr float ALTITUDE_LAPSE_RATE = 0.0065f;

  unsigned long delayTime = DEFAULT_DELAY_MS;
  unsigned long lastLoopMs = 0;
  unsigned long lastEdgePostMs = 0;
  unsigned long lastNewRelicPostMs = 0;
  unsigned long lastGrafanaPostMs = 0;

  float computeSaturationVaporPressureCelsius(float tempC)
  {
    const float exponent = (SATURATION_COEFF_A * tempC) / (tempC + SATURATION_COEFF_B);
    return SATURATION_BASE * expf(exponent);
  }

  float computeAbsoluteHumidity(float tempC, float humidity)
  {
    const float saturationPressure = computeSaturationVaporPressureCelsius(tempC);
    const float vaporPressure = saturationPressure * humidity / 100.0f;
    return (ABS_HUMID_NUMERATOR * vaporPressure) / (tempC + KELVIN_OFFSET);
  }

  float computeVpdKPa(float tempC, float humidity)
  {
    const float saturationPressure = computeSaturationVaporPressureCelsius(tempC) * HPA_TO_KPA;
    return saturationPressure * (1.0f - humidity / 100.0f);
  }

  float computeComfortIndex(float tempC, float humidity)
  {
    const float discomfortIndex = DISCOMFORT_TEMP_COEFF * tempC + DISCOMFORT_RH_COEFF * humidity * (DISCOMFORT_RH_TEMP_COEFF * tempC - DISCOMFORT_TEMP_OFFSET) + DISCOMFORT_CONSTANT;
    return discomfortIndex;
  }

  float computeIndoorWbgt(float tempC, float relativeHumidity)
  {
    if (!isfinite(tempC) || !isfinite(relativeHumidity))
    {
      return NAN;
    }

    const float humidity = constrain(relativeHumidity, 0.0f, 100.0f);
    const float wetBulbC = tempC * atanf(0.151977f * sqrtf(humidity + 8.313659f)) +
                           atanf(tempC + humidity) - atanf(humidity - 1.676331f) +
                           0.00391838f * humidity * sqrtf(humidity) * atanf(0.023101f * humidity) - 4.686035f;
    return 0.7f * wetBulbC + 0.3f * tempC;
  }

  ComfortMetrics computeComfortMetrics(const SensorData &data)
  {
    ComfortMetrics metrics;
    metrics.absoluteHumidity = computeAbsoluteHumidity(data.temperature, data.humidity);
    metrics.vaporPressureDeficit = computeVpdKPa(data.temperature, data.humidity);
    metrics.comfortIndex = computeComfortIndex(data.temperature, data.humidity);
    metrics.wbgt = computeIndoorWbgt(data.temperature, data.humidity);
    return metrics;
  }

  float computeAltitude(float temperatureC, float pressureHpa, float seaLevelPressureHpa)
  {
    if (!isfinite(temperatureC) || !isfinite(pressureHpa) || pressureHpa <= 0.0f ||
        !isfinite(seaLevelPressureHpa) || seaLevelPressureHpa < JMA_PRESSURE_MIN_HPA ||
        seaLevelPressureHpa > JMA_PRESSURE_MAX_HPA)
    {
      return NAN;
    }
    return (powf(seaLevelPressureHpa / pressureHpa, ALTITUDE_EXPONENT) - 1.0f) *
           (temperatureC + KELVIN_OFFSET) / ALTITUDE_LAPSE_RATE;
  }

  SensorData readSensorData()
  {
    SensorData data;
    data.temperature = bme.readTemperature();
    data.humidity = bme.readHumidity() + credentialStorage.humidityOffset;
    data.pressure = bme.readPressure() / 100.0F;
    data.altitude = credentialStorage.altitude;

    // NaN検出 (センサーエラー時)
    if (isnan(data.temperature))
      data.temperature = 0.0f;
    if (isnan(data.humidity))
      data.humidity = 0.0f;
    if (isnan(data.pressure))
      data.pressure = 0.0f;
    return data;
  }

  void prepareCanvas()
  {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0, HEADER_Y);
  }

  void drawDateLine(uint8_t x, uint8_t y)
  {
    time_t epoch = time(nullptr);

    oled.setTextSize(DATE_TEXT_SIZE);
    oled.setCursor(x, y);

    if (epoch == EPOCH_UNINITIALIZED || !timeSynced)
    {
      oled.print("--:--"); // 時刻未同期時
      return;
    }

    struct tm now;
    if (!getLocalTime(&now, 0))
    {
      oled.print("--:--"); // 取得失敗時
      return;
    }

    char dateBuffer[DATE_BUFFER_SIZE];
    strftime(dateBuffer, sizeof(dateBuffer), DATE_FORMAT, &now);
    oled.print(dateBuffer); // YYYY/mm/dd aaa HH:MM 表示
  }

  void drawHeader()
  {
    oled.setTextSize(HEADER_TEXT_SIZE);
    oled.setCursor(0, HEADER_Y);
    if (WiFi.status() == WL_CONNECTED)
    {
      oled.println(WiFi.localIP());
    }
    drawDateLine(0, DATE_Y);
    oled.drawLine(0, HEADER_LINE_Y, OLED_WIDTH, HEADER_LINE_Y, WHITE);
  }

  void drawSensorLines(const SensorData &data, const ComfortMetrics &metrics)
  {
    oled.setCursor(COL_LEFT_X, LEFT_TEMPERATURE_Y);
    oled.setTextSize(1);
    oled.print("T:");
    oled.setTextSize(2);
    oled.print(data.temperature, 1);

    oled.setCursor(COL_LEFT_X, LEFT_WBGT_Y);
    oled.setTextSize(1);
    oled.print("WBGT:");
    oled.setTextSize(2);
    oled.print(metrics.wbgt, 1);

    oled.setCursor(COL_LEFT_X, LEFT_DI_Y);
    oled.setTextSize(1);
    oled.print("DI:");
    oled.setTextSize(2);
    oled.print(metrics.comfortIndex, 1);

    oled.setTextSize(1);
    oled.setCursor(COL_RIGHT_X, RIGHT_PRESSURE_Y);
    oled.print(data.pressure, 0);
    oled.print("hPa");

    oled.setCursor(COL_RIGHT_X, RIGHT_ALTITUDE_Y);
    if (isfinite(data.altitude))
    {
      oled.print(data.altitude, 0);
    }
    else
    {
      oled.print("--");
    }
    oled.print('m');

    oled.setCursor(COL_RIGHT_X, RIGHT_HUMIDITY_Y);
    oled.print(data.humidity, 1);
    oled.print('%');

    oled.setCursor(COL_RIGHT_X, RIGHT_ABSOLUTE_HUMIDITY_Y);
    oled.print(metrics.absoluteHumidity, 1);
    oled.print("g/m3");

    oled.setCursor(COL_RIGHT_X, RIGHT_VPD_Y);
    oled.print(metrics.vaporPressureDeficit, 1);
    oled.print("kPa");

    oled.setTextSize(1);
    oled.drawLine(DIVIDER_LINE_X, DIVIDER_LINE_TOP_Y,
                  DIVIDER_LINE_X, DIVIDER_LINE_BOTTOM_Y, WHITE);
  }

  void scrubRandomPixel()
  {
    const int scrubX = random(0, OLED_WIDTH);
    const int scrubY = random(0, OLED_HEIGHT);

    // ランダムな位置に黒い"@"を描画してビジュアルエフェクトを作成
    oled.setTextSize(2);
    oled.setTextColor(BLACK);
    oled.setCursor(scrubX, scrubY);
    oled.print("@");
  }

  void renderWeatherScreen(const SensorData &data, const ComfortMetrics metrics)
  {
    prepareCanvas();
    drawHeader();
    drawSensorLines(data, metrics);
    scrubRandomPixel(); // display()前に呼び出し
    oled.display();     // 一度だけ呼び出す
  }

  String buildClimatePayload(const SensorData &data, const ComfortMetrics &metrics)
  {
    char payload[196];
    snprintf(payload, sizeof(payload),
             "{\"temperature\":%.1f,\"humidity\":%.1f,\"absolute_humidity\":%.1f,\"discomfort_index\":%.1f,\"vpd\":%.2f}",
             data.temperature, data.humidity, metrics.absoluteHumidity, metrics.comfortIndex,
             metrics.vaporPressureDeficit);
    return String(payload);
  }

  bool postClimateData(const SensorData &data, const ComfortMetrics &metrics)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      return false;
    }

    WiFiClient client;
    HTTPClient http;
    http.setTimeout(EDGE_POST_TIMEOUT_MS);

    if (!http.begin(client, EDGE_SERVER_URL))
    {
      return false;
    }

    http.addHeader("Content-Type", "application/json");
    const String payload = buildClimatePayload(data, metrics);
    const int status = http.POST(payload);
    http.end();

    return status >= 200 && status < 300;
  }

  bool newRelicSettingsAvailable()
  {
    return credentialStorage.deviceId[0] != '\0' &&
           isAllowedLocation(credentialStorage.location) &&
           credentialStorage.ingestKey[0] != '\0';
  }

  bool telemetryMetricsValid(const SensorData &data, const ComfortMetrics &metrics)
  {
    return isfinite(data.temperature) &&
           isfinite(data.humidity) &&
           isfinite(data.pressure) &&
           isfinite(data.altitude) &&
           isfinite(metrics.absoluteHumidity) &&
           isfinite(metrics.vaporPressureDeficit) &&
           isfinite(metrics.wbgt) &&
           isfinite(metrics.comfortIndex);
  }

  bool buildNewRelicPayload(const SensorData &data, const ComfortMetrics &metrics,
                            char *payload, size_t payloadSize, size_t &payloadLength)
  {
    const time_t timestamp = time(nullptr);
    const int written = snprintf(
        payload, payloadSize,
        "[{\"common\":{\"timestamp\":%lld,\"attributes\":{\"deviceId\":\"%s\",\"location\":\"%s\",\"sensor\":\"%s\"}},"
        "\"metrics\":[{\"name\":\"home.weather.temperature\",\"type\":\"gauge\",\"value\":%.1f},"
        "{\"name\":\"home.weather.humidity\",\"type\":\"gauge\",\"value\":%.1f},"
        "{\"name\":\"home.weather.absoluteHumidity\",\"type\":\"gauge\",\"value\":%.1f},"
        "{\"name\":\"home.weather.pressure\",\"type\":\"gauge\",\"value\":%.1f},"
        "{\"name\":\"home.weather.altitude\",\"type\":\"gauge\",\"value\":%.1f},"
        "{\"name\":\"home.weather.vpd\",\"type\":\"gauge\",\"value\":%.2f},"
        "{\"name\":\"home.weather.wbgt\",\"type\":\"gauge\",\"value\":%.1f},"
        "{\"name\":\"home.weather.discomfortIndex\",\"type\":\"gauge\",\"value\":%.1f}]}]",
        static_cast<long long>(timestamp), credentialStorage.deviceId, credentialStorage.location,
        SENSOR_NAME, data.temperature, data.humidity, metrics.absoluteHumidity, data.pressure,
        data.altitude, metrics.vaporPressureDeficit, metrics.wbgt, metrics.comfortIndex);

    if (written < 0 || static_cast<size_t>(written) >= payloadSize)
    {
      payloadLength = 0;
      return false;
    }

    payloadLength = static_cast<size_t>(written);
    return true;
  }

  bool postNewRelicData(const SensorData &data, const ComfortMetrics &metrics)
  {
    const time_t timestamp = time(nullptr);
    if (!newRelicSettingsAvailable() || WiFi.status() != WL_CONNECTED || !timeSynced ||
        timestamp == EPOCH_UNINITIALIZED || !telemetryMetricsValid(data, metrics))
    {
      return false;
    }

    char payload[1152];
    size_t payloadLength = 0;
    if (!buildNewRelicPayload(data, metrics, payload, sizeof(payload), payloadLength))
    {
      return false;
    }

    NetworkClientSecure client;
    client.setCACert(DIGICERT_GLOBAL_ROOT_G2);

    HTTPClient http;
    http.setTimeout(NEW_RELIC_POST_TIMEOUT_MS);
    if (!http.begin(client, NEW_RELIC_URL))
    {
      return false;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Api-Key", credentialStorage.ingestKey);
    const int status = http.POST(reinterpret_cast<uint8_t *>(payload), payloadLength);
    http.end();

    return status >= 200 && status < 300;
  }

  bool grafanaSettingsAvailable()
  {
    return credentialStorage.deviceId[0] != '\0' &&
           isAllowedLocation(credentialStorage.location) &&
           credentialStorage.grafanaApiKey[0] != '\0';
  }

  bool buildGrafanaPayload(const SensorData &data, const ComfortMetrics &metrics,
                           char *payload, size_t payloadSize, size_t &payloadLength)
  {
    struct GrafanaMetric
    {
      const char *name;
      double value;
      int precision;
    };

    const GrafanaMetric grafanaMetrics[] = {
        {"home.weather.temperature", data.temperature, 1},
        {"home.weather.humidity", data.humidity, 1},
        {"home.weather.absoluteHumidity", metrics.absoluteHumidity, 1},
        {"home.weather.pressure", data.pressure, 1},
        {"home.weather.altitude", data.altitude, 1},
        {"home.weather.vpd", metrics.vaporPressureDeficit, 2},
        {"home.weather.wbgt", metrics.wbgt, 1},
        {"home.weather.discomfortIndex", metrics.comfortIndex, 1},
    };
    const long long timeUnixNano = static_cast<long long>(time(nullptr)) * 1000000000LL;

    int written = snprintf(payload, payloadSize,
                           "{\"resourceMetrics\":[{\"scopeMetrics\":[{\"metrics\":[");
    if (written < 0 || static_cast<size_t>(written) >= payloadSize)
    {
      payloadLength = 0;
      return false;
    }

    size_t used = static_cast<size_t>(written);
    for (size_t i = 0; i < sizeof(grafanaMetrics) / sizeof(grafanaMetrics[0]); ++i)
    {
      written = snprintf(
          payload + used, payloadSize - used,
          "%s{\"name\":\"%s\",\"gauge\":{\"dataPoints\":[{\"timeUnixNano\":\"%lld\","
          "\"asDouble\":%.*f,\"attributes\":["
          "{\"key\":\"deviceId\",\"value\":{\"stringValue\":\"%s\"}},"
          "{\"key\":\"location\",\"value\":{\"stringValue\":\"%s\"}},"
          "{\"key\":\"sensor\",\"value\":{\"stringValue\":\"%s\"}}]}]}}",
          i == 0 ? "" : ",", grafanaMetrics[i].name, timeUnixNano,
          grafanaMetrics[i].precision, grafanaMetrics[i].value,
          credentialStorage.deviceId, credentialStorage.location, SENSOR_NAME);
      if (written < 0 || static_cast<size_t>(written) >= payloadSize - used)
      {
        payloadLength = 0;
        return false;
      }
      used += static_cast<size_t>(written);
    }

    written = snprintf(payload + used, payloadSize - used, "]}]}]}");
    if (written < 0 || static_cast<size_t>(written) >= payloadSize - used)
    {
      payloadLength = 0;
      return false;
    }

    payloadLength = used + static_cast<size_t>(written);
    return true;
  }

  bool postGrafanaData(const SensorData &data, const ComfortMetrics &metrics)
  {
    const time_t timestamp = time(nullptr);
    if (!grafanaSettingsAvailable() || WiFi.status() != WL_CONNECTED || !timeSynced ||
        timestamp == EPOCH_UNINITIALIZED || !telemetryMetricsValid(data, metrics))
    {
      return false;
    }

    static char payload[4096];
    size_t payloadLength = 0;
    if (!buildGrafanaPayload(data, metrics, payload, sizeof(payload), payloadLength))
    {
      return false;
    }

    NetworkClientSecure client;
    client.setCACert(DIGICERT_GLOBAL_ROOT_G2);

    HTTPClient http;
    http.setTimeout(GRAFANA_POST_TIMEOUT_MS);
    if (!http.begin(client, GRAFANA_OTLP_URL))
    {
      return false;
    }

    http.addHeader("Content-Type", "application/json");
    http.setAuthorization(GRAFANA_INSTANCE_ID, credentialStorage.grafanaApiKey);
    const int status = http.POST(reinterpret_cast<uint8_t *>(payload), payloadLength);
    http.end();

    return status >= 200 && status < 300;
  }
} // namespace

void initializeSensors()
{
  // OLED初期化
  if (!oledAvailable)
  {
    oledAvailable = oled.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS);
    if (oledAvailable)
    {
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(0, 0);
      oled.println(F("OLED: OK"));
      oled.display();
    }
  }

  // BME280初期化
  if (!bmeAvailable && sensorInitRetries < SENSOR_INIT_MAX_RETRIES)
  {
    bmeAvailable = bme.begin();
    if (!bmeAvailable)
    {
      sensorInitRetries++;
      if (oledAvailable)
      {
        oled.setTextSize(1);
        oled.setCursor(0, 16);
        oled.print(F("BME280: Retry "));
        oled.print(sensorInitRetries);
        oled.print(F("/"));
        oled.println(SENSOR_INIT_MAX_RETRIES);
        oled.display();
      }
    }
    else
    {
      if (oledAvailable)
      {
        oled.setCursor(0, 16);
        oled.println(F("BME280: OK"));
        oled.display();
      }
    }
  }
}

void displaySensorError()
{
  if (!oledAvailable)
  {
    return;
  }

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 0);
  oled.println(F("Sensor Error"));
  oled.println();

  if (!bmeAvailable)
  {
    oled.println(F("BME280 failed"));
    oled.println(F("Check wiring"));
  }

  oled.println();
  oled.println(F("Web config: OK"));
  oled.print(F("IP: "));
  oled.println(WiFi.softAPIP());
  oled.display();
}

bool accessInfoVisible()
{
  return (millis() - accessInfoStartedMs) < ACCESS_INFO_DISPLAY_MS;
}

void displayAccessInfo()
{
  if (!oledAvailable)
  {
    return;
  }

  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 0);
  oled.println(F("AP: ESP32AP"));
  oled.println(F("PASS:"));
  oled.println(credentialStorage.accessKey);
  oled.println(F("URL: 192.168.4.1"));
  oled.println(F("USER: admin"));
  oled.display();
}

void setup()
{
  // WiFiイベントハンドラーを登録
  WiFi.onEvent(onWiFiEvent);

  initCredentialStorage();
  startAccessPoint();
  setupConfigServer();

  if (connectWifiSta())
  {
    timeSynced = syncTimeFromNtp();
  }
  else
  {
    timeSynced = false;
  }

  // センサー初期化（非ブロッキング）
  initializeSensors();
  accessInfoStartedMs = millis();
  displayAccessInfo();
}

void loop()
{
  configServer.handleClient();
  ensureWifiConnection();
  ensureTimeSync();
  ensureDailyAltitude();

  // 非ブロッキング遅延
  const unsigned long now = millis();
  if ((now - lastLoopMs) < delayTime)
  {
    return;
  }
  lastLoopMs = now;

  // センサーが利用不可能な場合は初期化を試みる
  if (!bmeAvailable || !oledAvailable)
  {
    initializeSensors();
    if (!bmeAvailable && sensorInitRetries >= SENSOR_INIT_MAX_RETRIES)
    {
      if (accessInfoVisible())
      {
        displayAccessInfo();
      }
      else
      {
        displaySensorError();
      }
      return;
    }
  }

  if (bmeAvailable)
  {
    SensorData data = readSensorData();
    const ComfortMetrics metrics = computeComfortMetrics(data);

    if (oledAvailable)
    {
      if (accessInfoVisible())
      {
        displayAccessInfo();
      }
      else
      {
        renderWeatherScreen(data, metrics); // scrubRandomPixel()とdisplay()は内部で呼ばれる
      }
    }

    const unsigned long nowMs = millis();
    if ((nowMs - lastEdgePostMs) >= EDGE_POST_INTERVAL_MS)
    {
      postClimateData(data, metrics);
      lastEdgePostMs = nowMs;
    }

    if ((nowMs - lastNewRelicPostMs) >= NEW_RELIC_POST_INTERVAL_MS)
    {
      postNewRelicData(data, metrics);
      lastNewRelicPostMs = nowMs;
    }

    if ((nowMs - lastGrafanaPostMs) >= GRAFANA_POST_INTERVAL_MS)
    {
      postGrafanaData(data, metrics);
      lastGrafanaPostMs = nowMs;
    }
  }
}
