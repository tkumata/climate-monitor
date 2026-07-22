# 最新 Grafana ペイロード表示計画

## 目的

Grafana Cloud へ送信を試行した最新1件の OTLP JSON を RAM に保持し、端末の Web 画面で8個の気象値を Arc Gauge として表示する。

## フェーズ

### Phase 1: 契約更新

- 要件、仕様、設計、README、ADR を最新ペイロード表示へ同期する。
- 現行 ADR を `ADR-009-grafana-cloud-otlp.md` へ退避し、ADR-010 を作成する。

状態: 完了

### Phase 2: 最新ペイロード保持

- Grafana 送信用の既存固定長バッファを名前空間スコープへ移す。
- 送信条件を満たして生成した最新1件だけを RAM に保持する。
- EEPROM やファイルシステムへペイロードを書き込まない。

状態: 完了

### Phase 3: Arc Gauge 表示

- Digest 認証付き JSON endpoint と `/weather` を追加する。
- ブラウザ標準の SVG、CSS、JavaScript で8個の Arc Gauge を描画する。
- 1分間隔で最新JSONを再取得し、WBGT は既存基準に合わせて色分けする。

状態: 完了

### Phase 4: 検証

- 最新1件保持、認証、秘密値非表示、表示定義を自己検証する。
- 既存の自己検証、`git diff --check`、Nano ESP32 向けコンパイルを実行する。
- 実機ブラウザで Arc Gauge と5分周期の更新を確認する。

状態: 完了。自己検証、既存テスト、JavaScript 構文検査、`git diff --check`、Nano ESP32 向けコンパイル、実機ブラウザでの Arc Gauge と5分周期更新を確認済み。

## 成功基準

- 有効な Grafana OTLP JSON の生成時に、RAM 上の最新1件だけが上書きされる。
- `/weather` と JSON endpoint が既存の Digest 認証で保護される。
- 8個の値がラベル、単位、表示範囲付き Arc Gauge で表示される。
- Grafana API Key、Wi-Fi パスワード、New Relic Ingest Key が応答へ含まれない。
- 既存の送信周期、OLED、EEPROM レイアウトを変更しない。
- 自己検証、差分検査、Nano ESP32 向けコンパイルが成功する。
