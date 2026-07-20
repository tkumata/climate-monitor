# Grafana Cloud OTLP メトリクス送信計画

## 目的

New Relic へ送っている8個の気象メトリクスを、同じ5分周期で Grafana Cloud にも送信する。

## フェーズ

### Phase 1: 契約更新

- 要件、仕様、設計、README、ADR を Grafana Cloud 対応へ同期する。
- 現行 ADR を `ADR-008-wbgt-stull.md` へ退避し、ADR-009 を作成する。

状態: 完了

### Phase 2: 設定保存

- Grafana API Key を設定 Web サーバーから入力し、EEPROM に保存する。
- 現行 EEPROM 形式から既存設定を維持して移行する。
- 保存済み API Key は Web ページへ再表示しない。

状態: 完了

### Phase 3: OTLP 送信

- 既存の deviceId、location、8個の計測値を OTLP/HTTP JSON に変換する。
- Grafana Cloud OTLP metrics endpoint へ Basic 認証付き HTTPS POST を行う。
- New Relic と既存エッジサーバーの送信を維持する。

状態: 完了

### Phase 4: 検証

- Grafana 設定、EEPROM 移行、OTLP payload の自己検証を実行する。
- `git diff --check` を実行する。
- `arduino-cli compile --fqbn esp32:esp32:nano_nora .` を実行する。
- 実機で Grafana Cloud への送信を確認する。

状態: 自己検証、`git diff --check`、Nano ESP32 向けコンパイルは完了。実機確認は未実施。

## 成功基準

- Grafana API Key が Web から保存・維持・削除でき、HTTP 応答へ露出しない。
- 既存 EEPROM の Wi-Fi、New Relic、気象、アクセスキー設定が移行後も維持される。
- 8個の有限なメトリクスが deviceId、location、sensor 属性付きで5分間隔に送信される。
- Grafana 未設定時も New Relic と既存エッジサーバーの送信が変わらない。
- 自己検証、差分検査、Nano ESP32 向けコンパイルが成功する。
- 実機から Grafana Cloud でメトリクスを確認できる。
