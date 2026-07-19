# ADR-007: 端末固有キーで SoftAP と設定 Web を保護する

## 状態

採用

## 背景

SoftAP が passphrase なしで起動し、SoftAP 側と STA 側の設定 Web サーバーへ認証なしでアクセスできる。設定ページは Wi-Fi、New Relic、湿度補正などを変更・削除できるため、近隣端末や同一 LAN の利用者による不正操作を防ぐ必要がある。

## 決定

初回に小文字と数字からなる12文字の端末固有アクセスキーをハードウェア乱数から生成して EEPROM に保存する。同じキーを SoftAP の WPA2-PSK passphrase と、ユーザー名 `admin` の HTTP Digest 認証パスワードに使用する。

設定 Web サーバーは SoftAP 側と STA 側の両方から利用可能とし、すべてのルートを Digest 認証で保護する。既存の CSRF 検証を維持し、保存済み Wi-Fi パスワードと Ingest Key は HTML へ再表示しない。アクセスキーは起動後120秒間 OLED に表示し、設定削除後も維持する。

## 結果

- passphrase を知らない端末は SoftAP に接続できない。
- SoftAP と STA のどちらでも設定変更前に利用者認証が必要になる。
- 独自セッション管理を追加せず、ESP32 WebServer の既存 Digest 認証を再利用できる。
- SoftAP と Web のキーが共通であるため、管理する秘密情報は1つで済む。
- STA 側は平文 HTTP のため、HTTP 本文の盗聴は防止しない。信頼できない LAN から使用する場合は HTTPS 化が必要になる。
