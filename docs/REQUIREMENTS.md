# New Relic Metric API 送信要件

## 機能要件

1. New Relic Metric API の `https://metric-api.newrelic.com/metric/v1` へ HTTPS POST する。
2. 設定 Web サーバーで deviceId、location、New Relic Ingest Key を設定できる。
3. deviceId は 1〜32 文字の半角英数字とする。
4. location は `home`、`apartment`、`outdoor`、`office` のいずれかとする。
5. deviceId、location、Ingest Key を EEPROM に保存し、再起動後に復元する。
6. 既存 EEPROM レイアウトから SSID、パスワード、湿度補正値を失わずに移行する。
7. 以下をすべて満たす場合のみ New Relic 送信を試行する。
   - deviceId が有効である。
   - location が有効である。
   - Ingest Key が設定されている。
   - STA が Wi-Fi に接続されている。
   - NTP 時刻同期が成功している。
   - 送信対象の全計測値が有限値である。
8. 共通属性として deviceId、location、固定値 `BME280` の sensor を送信する。
9. NTP 同期済みの Unix 秒を `common.timestamp` として送信する。
10. 気温、相対湿度、絶対湿度、気圧、高度、VPD、WBGT、DI を gauge メトリクスとして送信する。
11. Ingest Key を `Api-Key` ヘッダーへ設定する。
12. 既存エッジサーバー送信を維持する。

## セキュリティ要件

- TLS サーバー証明書を検証し、検証を無効化しない。
- Ingest Key は設定ページへ保存値を再表示しない。
- Ingest Key を OLED、シリアル出力、HTTP 応答本文、New Relic JSON 本文へ出力しない。
- 設定 POST は既存の CSRF 検証を維持する。
- 全入力を固定長バッファへ収まる長さに制限する。

## 非機能要件

- New Relic 送信間隔は 5 分とする。既存エッジサーバーの送信間隔は 15 分のまま維持する。
- 反復経路に不要な抽象化や依存ライブラリを追加しない。
- 既存 OLED レイアウトおよび既存指標の計算式を変更しない。
- 高度は既存の基準海面気圧 `1013.25 hPa` を使用した BME280 の推定値とする。
- 送信失敗後も設定 Web サーバー、OLED 更新、Wi-Fi 再接続を継続する。
