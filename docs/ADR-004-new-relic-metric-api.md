# ADR-004: New Relic Metric API へ端末から直接 HTTPS 送信する

## 状態

採用

## 背景

BME280 の計測値と算出済み指標を New Relic で時系列監視する必要がある。送信元を識別する deviceId と設置場所、認証用 Ingest Key は端末ごとに異なるため、ファームウェアへ固定値として埋め込まず設定 Web サーバーから保存する必要がある。

## 決定

端末から New Relic Metric API の US エンドポイントへ 5 分間隔で直接 HTTPS POST する。共通属性に deviceId、location、sensor を設定し、NTP 同期済み Unix 秒を共通 timestamp とする。8 個の値は gauge メトリクスとして 1 リクエストにまとめる。

設定は既存 `CredentialStorage` を拡張して EEPROM に保存し、magic を更新する。旧 magic は旧構造体として読み取り、新構造体へ明示的に移行する。Ingest Key は保存値を Web ページへ再表示せず、TLS 証明書検証を有効にする。

## 結果

- 外部の中継サーバーを追加せず New Relic へメトリクスを送信できる。
- 端末ごとの属性でメトリクスを絞り込める。
- NTP 未同期の誤った timestamp と、不正な数値の送信を防止できる。
- ルート CA の期限や New Relic の証明書チェーン変更時にはファームウェア更新が必要になる可能性がある。
- Ingest Key は EEPROM に平文で保持されるため、物理的に端末へアクセスできる主体からの秘匿は保証しない。
