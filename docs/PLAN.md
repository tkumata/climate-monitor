# OLED ヘッダー URL 表示計画

## 目的

OLED の `Meteorology Info` タイトルを廃止し、STA 接続中に設定ページの STA 側 IP アドレスを最上段へ表示する。

## フェーズ

### Phase 1: 要件と設計の確定

- 要件、仕様、設計、ADR を作成する。
- STA 接続時と未接続時のヘッダー表示を確定する。

状態: 完了

### Phase 2: 実装

- `Meteorology Info` を削除する。
- `WiFi.localIP()` を表示する。
- `README.md` と設計ドキュメントを実装と同期する。

状態: 完了

### Phase 3: 検証

- `arduino-cli compile --fqbn arduino:esp32:nano_nora .` を実行する。
- 実機で STA 接続時の IP アドレス表示、未接続時の IP アドレス非表示、および Wi-Fi 再接続を確認する。

状態: 差分の静的確認完了。Arduino ESP32 ボード定義がこの環境にないため、コンパイルおよび実機確認は未実施。

## 成功基準

- `Meteorology Info` が OLED に表示されない。
- STA 接続中は最上段に STA 側 IP アドレスが表示される。
- コンパイルが成功し、実機表示が仕様どおりである。
