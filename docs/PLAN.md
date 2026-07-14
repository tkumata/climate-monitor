# New Relic Metric API 送信計画

## 目的

BME280 の計測値と算出済み指標を、デバイス固有の属性とともに New Relic Metric API へ HTTPS POST する。deviceId、location、Ingest Key は設定 Web サーバーから入力し、EEPROM に保持する。

## フェーズ

### Phase 1: 要件と設計の確定

- New Relic Metric API のペイロード、送信条件、メトリクス名を定義する。
- EEPROM の既存設定を維持するマイグレーションを定義する。
- 設定 Web UI と Ingest Key の取り扱いを定義する。

状態: 完了

### Phase 2: 設定保存と Web UI

- deviceId、location、Ingest Key を保存領域へ追加する。
- 既存レイアウトから新レイアウトへ SSID、パスワード、湿度補正値を移行する。
- 設定ページへ deviceId 入力、location 選択、Ingest Key 入力を追加する。
- deviceId と location のサーバー側入力検証を追加する。

状態: 完了

### Phase 3: New Relic 送信

- 高度を BME280 から取得する。
- 8 個の gauge メトリクスを含む JSON を生成する。
- 必須設定、Wi-Fi、NTP、計測値を検証して HTTPS POST する。
- TLS サーバー証明書を検証し、Ingest Key を `Api-Key` ヘッダーへ設定する。

状態: 完了

### Phase 4: 検証

- `git diff --check` を実行する。
- `arduino-cli compile --fqbn esp32:esp32:nano_nora .` を実行する。
- 実機で設定保存、Wi-Fi 再接続、NTP 同期、New Relic への取り込みを確認する。

状態: `git diff --check` と Nano ESP32 向けコンパイルは完了。実機確認は未実施。

## 成功基準

- deviceId、location、Ingest Key が設定ページから保存でき、再起動後も保持される。
- 既存 EEPROM データの SSID、パスワード、湿度補正値が新レイアウトへ移行される。
- deviceId は半角英数字のみ受理され、location は定義済み 4 値のみ受理される。
- 全送信条件を満たす場合のみ、8 個の gauge メトリクスが New Relic Metric API へ送信される。
- `common.timestamp` に NTP 同期後の Unix 秒、`common.attributes` に deviceId、location、sensor が設定される。
- Ingest Key が設定ページの HTML、OLED、ログ、JSON 本文へ露出しない。
- 既存 OLED 表示、計算、Wi-Fi 設定、エッジサーバー送信が維持される。
- Nano ESP32 向けコンパイルが成功し、実機で New Relic 上の値と属性を確認できる。
