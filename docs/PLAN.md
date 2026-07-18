# SoftAP・設定 Web 認証計画

## 目的

端末固有のアクセスキーで SoftAP と設定 Web サーバーを保護し、保存済み秘密情報を設定ページへ再表示しない。

## フェーズ

### Phase 1: 仕様更新

- SoftAP、Digest 認証、アクセスキー保存、STA 側アクセスの要件を定義する。
- 現行 ADR を退避し、新しいセキュリティ ADR を作成する。

状態: 完了

### Phase 2: アクセスキーと SoftAP

- 小文字と数字からなる12文字の端末固有アクセスキーを生成して EEPROM に保存する。
- 現行 EEPROM 形式から全設定値を維持して移行する。
- SoftAP を WPA2-PSK で起動する。

状態: 完了

### Phase 3: Web 認証

- SoftAP 側と STA 側の全ルートへ Digest 認証を適用する。
- Wi-Fi パスワードを再表示せず、空欄なら保存値を維持する。
- 既存の CSRF 検証を維持する。

状態: 完了

### Phase 4: OLED 表示

- 起動後120秒間、SoftAP SSID、アクセスキー、URL、Web ユーザー名を表示する。
- 120秒後に通常表示へ戻す。

状態: 完了

### Phase 5: 検証

- セキュリティ自己検証と既存自己検証を実行する。
- `git diff --check` を実行する。
- `arduino-cli compile --fqbn esp32:esp32:nano_nora .` を実行する。
- SoftAP、Digest 認証、STA 側アクセス、OLED を実機確認する。

状態: 自己検証、既存自己検証、`git diff --check`、Nano ESP32 向けコンパイルは完了。実機確認は未実施。

## 成功基準

- passphrase なしでは SoftAP に接続できない。
- SoftAP 側と STA 側の未認証 HTTP リクエストが `401` になる。
- 認証後も不正な CSRF トークンの POST が `403` になる。
- 再起動と設定削除の後も同じアクセスキーを使用できる。
- 現行 EEPROM の全設定値を失わずに移行する。
- 保存済み Wi-Fi パスワードと Ingest Key を HTML に含めない。
- Nano ESP32 向けコンパイルが成功する。
- 実機で SoftAP、SoftAP/STA 両側の Web 認証、OLED 表示を確認できる。
