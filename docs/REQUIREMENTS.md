# 気象観測・外部メトリクス送信要件

## 機能要件

1. New Relic Metric API の `https://metric-api.newrelic.com/metric/v1` へ HTTPS POST する。
2. 設定 Web サーバーで deviceId、location、New Relic Ingest Key を設定できる。
3. deviceId は 1〜32 文字の半角英数字とする。
4. location は `home`、`apartment`、`outdoor`、`office` のいずれかとする。
5. deviceId、location、Ingest Key を EEPROM に保存し、再起動後に復元する。
6. 既存 EEPROM レイアウトから SSID、パスワード、湿度補正値、New Relic 設定を失わずに移行する。
7. 以下をすべて満たす場合のみ New Relic 送信を試行する。
   - deviceId が有効である。
   - location が有効である。
   - Ingest Key が設定されている。
   - STA が Wi-Fi に接続されている。
   - NTP 時刻同期が成功している。
   - 送信対象の全計測値が有限値である。
8. 共通属性として deviceId、location、固定値 `BME280` の sensor を送信する。
9. NTP 同期済みの Unix 秒を `common.timestamp` として送信する。
10. 気温、相対湿度、絶対湿度、気圧、標高、VPD、WBGT、DI を gauge メトリクスとして送信する。
11. Ingest Key を `Api-Key` ヘッダーへ設定する。
12. 既存エッジサーバー送信を維持する。
13. 取得時点の BME280 の測定気温と測定気圧、気象庁の東京の当日12時の海面気圧から標高を1日1回算出する。
14. 算出した標高を OLED にメートル単位、小数なしで表示する。
15. OLED と New Relic では、翌日の更新まで保存済み標高を使用する。
16. 気温と相対湿度から Stull の近似式で湿球温度を求め、`0.7 × 湿球温度 + 0.3 × 気温` で WBGT を算出する。気圧は使用しない。
17. Grafana Cloud OTLP metrics endpoint へ New Relic と同じ8個の値を HTTPS POST する。
18. Grafana API Key を設定 Web サーバーから EEPROM に保存し、空欄保存では現在値を維持する。
19. deviceId、location、Grafana API Key、Wi-Fi 接続、NTP 同期、有限な8個の値が揃う場合だけ Grafana 送信を試行する。
20. Grafana の各データポイントへ deviceId、location、`sensor=BME280` を属性として設定する。
21. Grafana 送信の失敗は New Relic と既存エッジサーバーの送信を妨げない。
22. Grafana 送信条件を満たして生成した最新1件の OTLP JSON を RAM に保持し、次回生成時に上書きする。
23. 最新 OTLP JSON を Digest 認証付き Web endpoint から取得できる。
24. 最新 OTLP JSON の8個の値を、ラベルと単位を持つ Arc Gauge として Web 画面に表示する。
25. Arc Gauge 画面は1分間隔で最新 OTLP JSON を再取得する。

## 気象庁データ取得要件

1. 東京観測所番号 `44132` の AMeDAS JSON を HTTPS GET する。
2. 当日 13:00 JST 以降に取得し、12:00 の `normalPressure` を海面気圧として使用する。
3. 取得成功後は当日中に再取得しない。
4. 取得失敗時は保存値を維持し、1時間間隔で再試行する。
5. JSON 全体を動的文字列へ保持せず、ストリームから対象箇所を抽出する。
6. 観測日時が JST の当日12:00であることを検証する。
7. 通常値と準正常値 `)` を受理し、資料不足値 `]`、欠測 `×`、空欄を拒否する。
8. `850.0～1100.0 hPa` の有限値だけを受理する。
9. 海面気圧、対象日、算出標高を EEPROM に保存し、取得または計算の失敗時は保存値を上書きしない。

## セキュリティ要件

- SoftAP は端末固有の12文字のアクセスキーを passphrase とする WPA2-PSK で保護する。アクセスキーは小文字と数字で構成し、`l`、`o`、`0`、`1` は使用しない。
- アクセスキーはハードウェア乱数から初回に1回だけ生成して EEPROM に保存し、再起動と設定削除の後も維持する。
- 設定 Web サーバーは SoftAP 側と STA 側の両方から利用可能とし、全ルートでユーザー名 `admin` とアクセスキーによる HTTP Digest 認証を要求する。
- 未認証リクエストでは設定内容を返さない。
- 保存済み Wi-Fi パスワード、New Relic Ingest Key、Grafana API Key は設定ページへ再表示しない。空欄で保存した場合はそれぞれの保存値を維持する。
- 起動後120秒間、OLED に SoftAP SSID、アクセスキー、URL、Web ユーザー名を表示する。
- New Relic、Grafana Cloud、気象庁の TLS サーバー証明書を検証し、検証を無効化しない。
- Ingest Key は設定ページへ保存値を再表示しない。
- Ingest Key を OLED、シリアル出力、HTTP 応答本文、New Relic JSON 本文へ出力しない。
- Grafana API Key を OLED、シリアル出力、HTTP 応答本文、OTLP JSON 本文へ出力しない。
- 最新 OTLP JSON endpoint と Arc Gauge 画面でも既存の HTTP Digest 認証を要求する。
- 設定 POST は既存の CSRF 検証を維持する。
- 全入力を固定長バッファへ収まる長さに制限する。
- STA 側の設定 Web 通信は平文 HTTP であり、信頼できるローカル LAN 内からの不正操作防止を対象とする。LAN 内の盗聴対策は対象外とする。

## 非機能要件

- New Relic と Grafana Cloud の送信間隔はそれぞれ 5 分とする。既存エッジサーバーの送信間隔は 15 分のまま維持する。
- 反復経路に不要な抽象化や依存ライブラリを追加しない。
- OLED のヘッダーと左列の気温、WBGT、DI を維持する。
- BME280 ライブラリの固定基準気圧による高度取得は使用しない。
- 標高入力が有限値でない、測定気圧が 0 以下、海面気圧が不正な場合は保存値を更新しない。
- 通信失敗後も設定 Web サーバー、OLED 更新、Wi-Fi 再接続を継続する。
- 最新 OTLP JSON は EEPROM やファイルシステムへ保存せず、再起動時に消去する。
- Arc Gauge は外部 JavaScript、CSS、フォント、チャートライブラリへ依存しない。
