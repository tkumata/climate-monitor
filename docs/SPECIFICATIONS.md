# 気象観測・New Relic 仕様

## SoftAP・設定 Web 認証

| Item | Value |
| --- | --- |
| SoftAP SSID | `ESP32AP` |
| SoftAP security | WPA2-PSK |
| Access key | Hardware-random 12 lowercase letters or digits, persisted in EEPROM |
| Web username | `admin` |
| Web authentication | HTTP Digest, realm `Climate Monitor` |
| Web interfaces | SoftAP and STA |
| OLED guidance | First 120 seconds after boot |

アクセスキーには小文字と数字から紛らわしい `l`、`o`、`0`、`1` を除いた32文字を使用し、各文字を5bitの乱数から選ぶ。ソースコード、MAC アドレス、他の保存資格情報から導出しない。

`/`、`POST /save`、`POST /clear`、未定義パスを含むすべての HTTP リクエストで Digest 認証を先に検証する。POST は認証後に既存の CSRF トークンを検証する。保存済み Wi-Fi パスワードと Ingest Key は HTML に含めず、各入力が空の場合は保存値を維持する。

設定削除では Wi-Fi、湿度補正、New Relic、気象データを消去するが、アクセスキーは維持する。STA 側アクセスは HTTP 本文を暗号化しないため、信頼できる LAN でのみ使用する。

## 設定項目

| Item | Input | Validation | Persistence |
| ---- | ----- | ---------- | ----------- |
| deviceId | Text | ASCII letters and digits, 1-32 characters | EEPROM |
| location | Select | `home`, `apartment`, `outdoor`, `office` | EEPROM |
| Ingest Key | Password | 1-64 characters when newly entered | EEPROM |

Ingest Key 入力が空の場合、保存済みのキーを維持する。保存済みキーは HTML の `value` に設定せず、設定済みかどうかだけを表示する。deviceId または location が未設定でも Wi-Fi 設定は保存できるが、New Relic 送信は行わない。

## 気象庁データ

| Item | Value |
| --- | --- |
| URL | `https://www.jma.go.jp/bosai/amedas/data/point/44132/YYYYMMDD_12.json` |
| Station | Tokyo `44132` |
| Element | `YYYYMMDD120000.normalPressure` |
| Unit | hPa |
| Normal schedule | First opportunity at or after 13:00 JST |
| Frequency | One successful update per JST day |
| Retry | Every hour after a failed attempt |
| Accepted quality | Normal value or quasi-normal value `)` |
| Rejected quality | Insufficient value `]`, missing `×`, empty cell |
| Accepted range | `850.0～1100.0 hPa` |

URL には東京観測所番号 `44132` と当日の日付を指定する。JST の当日12:00を表すキーの `normalPressure` を取得し、品質コードが通常または準正常の場合だけ採用する。

## 標高計算

```text
altitude =
  ((seaLevelPressure / measuredPressure)^(1 / 5.257) - 1)
  * (temperature + 273.15)
  / 0.0065
```

| Input | Source | Unit |
| --- | --- | --- |
| `temperature` | BME280 at daily fetch | °C |
| `measuredPressure` | BME280 at daily fetch | hPa |
| `seaLevelPressure` | JMA Tokyo at 12:00 on current day | hPa |
| `altitude` | Calculated and persisted result | m |

入力が有限値でない、測定気圧が 0 以下、海面気圧が範囲外の場合は保存値を更新しない。保存した標高は翌日の更新まで再計算しない。

## EEPROM マイグレーション

現在の保存構造へ `accessKey` を追加して magic を更新する。直前の標高対応構造を含む既存形式から SSID、パスワード、湿度補正値、deviceId、location、Ingest Key、海面気圧、対象日、標高を維持して移行し、アクセスキーだけを新規生成する。

日付は `YYYYMMDD` の整数で保存する。取得と計算に成功した場合だけ海面気圧、`seaLevelPressureDate`、標高を一括更新する。設定 Web UI から資格情報を保存する場合は取得済み値を維持する。

## 送信条件

| Condition | Required state |
| --------- | -------------- |
| deviceId | Valid and non-empty |
| location | One of the four allowed values |
| Ingest Key | Non-empty |
| Wi-Fi | `WiFi.status() == WL_CONNECTED` |
| Time | `timeSynced == true` and epoch is initialized |
| Metrics | All eight values are finite |

インターネット到達性は送信先ホストの DNS 解決、TLS 接続、HTTP 応答によって確認する。別ホストへの疎通確認は行わない。

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
      {"name":"home.weather.altitude","type":"gauge","value":23.6},
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
| `home.weather.altitude` | Daily persisted `SensorData.altitude` | m |
| `home.weather.vpd` | `ComfortMetrics.vaporPressureDeficit` | kPa |
| `home.weather.wbgt` | `ComfortMetrics.wbgt` | °C |
| `home.weather.discomfortIndex` | `ComfortMetrics.comfortIndex` | index |

標高が無効な場合は、全メトリクス有限値チェックにより New Relic 送信を行わない。

## WBGT 計算

```text
Tw = T × atan(0.151977 × sqrt(RH + 8.313659))
     + atan(T + RH) - atan(RH - 1.676331)
     + 0.00391838 × RH^(3/2) × atan(0.023101 × RH) - 4.686035
WBGT = 0.7 × Tw + 0.3 × T
```

| Input | Source | Unit |
| --- | --- | --- |
| `T` | `SensorData.temperature` | °C |
| `RH` | `SensorData.humidity`, constrained to 0-100 | %RH |
| `Tw` | Wet-bulb temperature calculated with the Stull approximation | °C |

気温または相対湿度が有限値でない場合は `NAN` とする。WBGT の計算には気圧を使用しない。

## OLED

左列の気温、WBGT、DI とヘッダーを維持する。右列は文字サイズ1で次の5行を表示する。

| Row | Value | Format |
| --- | --- | --- |
| 1 | Measured pressure | integer + `hPa` |
| 2 | Calculated altitude | integer + `m`; unavailable is `--m` |
| 3 | Relative humidity | one decimal + `%` |
| 4 | Absolute humidity | one decimal + `g/m3` |
| 5 | VPD | one decimal + `kPa` |
