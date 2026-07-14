# New Relic Metric API 送信仕様

## 設定項目

| Item | Input | Validation | Persistence |
| ---- | ----- | ---------- | ----------- |
| deviceId | Text | ASCII letters and digits, 1-32 characters | EEPROM |
| location | Select | `home`, `apartment`, `outdoor`, `office` | EEPROM |
| Ingest Key | Password | 1-64 characters when newly entered | EEPROM |

Ingest Key 入力が空の場合、保存済みのキーを維持する。保存済みキーは HTML の `value` に設定せず、設定済みかどうかだけを表示する。deviceId または location が未設定でも Wi-Fi 設定は保存できるが、New Relic 送信は行わない。

## EEPROM マイグレーション

旧レイアウトの magic を検出した場合、SSID、パスワード、湿度補正値を新レイアウトへコピーする。追加項目は空の状態で初期化し、新レイアウトの magic で一度だけ保存する。新旧いずれの magic でもない場合は、全項目を初期化して湿度補正値だけを既定値 `-7.0` とする。

## 送信条件

| Condition | Required state |
| --------- | -------------- |
| deviceId | Valid and non-empty |
| location | One of the four allowed values |
| Ingest Key | Non-empty |
| Wi-Fi | `WiFi.status() == WL_CONNECTED` |
| Time | `timeSynced == true` and epoch is initialized |
| Metrics | All eight values are finite |

インターネット到達性は New Relic ホストの DNS 解決、TLS 接続、HTTP 応答によって確認する。別ホストへの疎通確認は行わない。

## HTTP リクエスト

| Item | Value |
| ---- | ----- |
| Method | `POST` |
| URL | `https://metric-api.newrelic.com/metric/v1` |
| Content-Type | `application/json` |
| Api-Key | EEPROM に保存された Ingest Key |
| Timeout | 5000 ms |
| Success | HTTP status 200-299 |

TLS では New Relic エンドポイントの信頼チェーンを検証する。`setInsecure()` は使用しない。

## JSON ペイロード

```json
[
  {
    "common": {
      "timestamp": 1783987200,
      "attributes": {
        "deviceId": "weathernode01",
        "location": "outdoor",
        "sensor": "BME280"
      }
    },
    "metrics": [
      {"name":"home.weather.temperature","type":"gauge","value":27.4},
      {"name":"home.weather.humidity","type":"gauge","value":68.2},
      {"name":"home.weather.absoluteHumidity","type":"gauge","value":17.5},
      {"name":"home.weather.pressure","type":"gauge","value":1007.3},
      {"name":"home.weather.altitude","type":"gauge","value":49.8},
      {"name":"home.weather.vpd","type":"gauge","value":1.19},
      {"name":"home.weather.wbgt","type":"gauge","value":24.8},
      {"name":"home.weather.discomfortIndex","type":"gauge","value":76.0}
    ]
  }
]
```

`timestamp` は New Relic の予約フィールドであるため attributes には重複して格納しない。

## メトリクス

| Metric name | Source | Unit |
| ----------- | ------ | ---- |
| `home.weather.temperature` | `SensorData.temperature` | °C |
| `home.weather.humidity` | `SensorData.humidity` | %RH |
| `home.weather.absoluteHumidity` | `ComfortMetrics.absoluteHumidity` | g/m³ |
| `home.weather.pressure` | `SensorData.pressure` | hPa |
| `home.weather.altitude` | `SensorData.altitude` | m |
| `home.weather.vpd` | `ComfortMetrics.vaporPressureDeficit` | kPa |
| `home.weather.wbgt` | `ComfortMetrics.wbgt` | °C |
| `home.weather.discomfortIndex` | `ComfortMetrics.comfortIndex` | index |

## 既存 OLED 仕様

OLED の IP アドレス、日時、気温、WBGT、DI、気圧、相対湿度、絶対湿度、VPD の描画位置、文字サイズ、計算式は変更しない。
