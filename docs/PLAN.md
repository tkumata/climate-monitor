# WBGT Stull 近似式への変更計画

## 目的

気温と相対湿度から Stull の近似式で湿球温度を求め、屋内 WBGT を算出する。

## フェーズ

### Phase 1: 契約更新

- 要件、仕様、設計、README、ADR を Stull の近似式へ同期する。
- 現行 ADR を `ADR-007-softap-web-authentication.md` へ退避し、ADR-008 を作成する。

状態: 完了

### Phase 2: 実装

- `computeIndoorWbgt()` を Stull の湿球温度近似式へ置き換える。
- 気圧、湿球温度、二分法にだけ使うコードを削除する。
- OLED と New Relic の既存データ経路を維持する。

状態: 完了

### Phase 3: 検証

- 代表入力に対する湿球温度と WBGT の自己検証を実行する。
- `git diff --check` を実行する。
- `arduino-cli compile --fqbn esp32:esp32:nano_nora .` を実行する。
- 実機で OLED の WBGT 表示を確認する。

状態: 自己検証、`git diff --check`、Nano ESP32 向けコンパイルは完了。実機確認は未実施。

## 成功基準

- 湿球温度が指定された Stull の近似式で算出される。
- WBGT が `0.7 × 湿球温度 + 0.3 × 気温` で算出される。
- WBGT 計算に気圧と反復処理を使用しない。
- OLED と New Relic が同じ `ComfortMetrics.wbgt` を使用する。
- 自己検証、差分検査、Nano ESP32 向けコンパイルが成功する。
- 実機で WBGT 表示を確認できる。
