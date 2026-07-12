# OLED ヘッダー IP アドレス表示仕様

## OLED ヘッダー

| Condition | Position | Text size | Format |
| --------- | -------- | --------- | ------ |
| STA connected | x=0, y=0 | 1 | `<WiFi.localIP()>` |
| STA disconnected | x=0, y=0 | 1 | Empty |
| Date and time | x=0, y=8 | 1 | Existing format |

`Meteorology Info` と `http://` は表示しない。STA 接続の判定には `WiFi.status() == WL_CONNECTED` を使用する。IPv4 アドレスは最大 15 文字であり、文字サイズ 1 の OLED 横幅 128px に収まる。

## OLED 計測値レイアウト

| Item | Position | Text size | Format |
| ---- | -------- | --------- | ------ |
| Temperature | 左列 y=16 | ラベル: 1、値: 2 | `T:xx.x` |
| WBGT | 左列 y=32 | ラベル: 1、値: 2 | `WBGT:xx.x` |
| DI | 左列 y=48 | ラベル: 1、値: 2 | `DI:xx.x` |
| Pressure | 右列 y=16 | 1 | `xxxxhPa` |
| Relative humidity | 右列 y=28 | 1 | `xx.x%` |
| Absolute humidity | 右列 y=40 | 1 | `xx.xg/m3` |
| VPD | 右列 y=52 | 1 | `x.xkPa` |

左列は x=0、右列は x=80、列の区切り線は x=78 とする。左列は項目名を文字サイズ 1、値を文字サイズ 2 とする。右列は項目名を省略し、値と単位を文字サイズ 1 で表示する。

## WBGT 計算

1. 相対湿度を 0〜100% に丸める。
2. 飽和水蒸気圧を `6.112 × exp(17.62 × T / (243.12 + T))` hPa で求める。
3. 実際の水蒸気圧を `相対湿度 / 100 × 飽和水蒸気圧` で求める。
4. 湿球温度の探索範囲を -50°C から気温までとし、50 回の二分法で湿球温度を求める。乾湿計係数には湿球温度と気圧を用いる。
5. WBGT を `0.7 × 湿球温度 + 0.3 × 気温` で求める。

有限でない気温・相対湿度・気圧、または 0 以下の気圧では WBGT を NaN とする。OLED では既存の `print` 挙動に従って表示する。
