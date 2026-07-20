# ADR-009: Grafana Cloud OTLP endpoint へ端末から直接送信する

## 状態

採用

## 背景

New Relic へ送信している気象メトリクスを Grafana Cloud でも利用する。Grafana API Key は秘密情報であり、ファームウェアへ固定せず端末ごとに設定できる必要がある。

## 決定

端末から Grafana Cloud の OTLP/HTTP metrics endpoint へ5分間隔で JSON を直接送信する。New Relic と同じ deviceId、location、sensor 属性と8個の gauge 値を使い、NTP 同期済み Unix 時刻をナノ秒へ変換する。

Grafana API Key は設定 Web サーバーから EEPROM に保存する。Basic 認証は ESP32 `HTTPClient` の標準機能を使用し、Instance ID は接続先と同じ固定設定とする。TLS は検証を無効化せず、既存の DigiCert Global Root G2 を使用する。

## 結果

- New Relic と Grafana Cloud の両方で同じ計測値を利用できる。
- 外部 Collector や追加ライブラリを必要としない。
- JSON OTLP は低頻度の8メトリクスに限定する。
- Grafana の OTLP 変換により、メトリクス名の `.` は保存時に `_` へ変換される。
- endpoint、Instance ID、証明書チェーンの変更時はファームウェア更新が必要になる。
