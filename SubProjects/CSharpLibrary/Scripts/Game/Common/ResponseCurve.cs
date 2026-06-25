using System;
using System.Collections.Generic;

// 制御点リストを折れ線グラフとして評価する汎用ユーティリティ。
// points を「入力 t(0..1) を等間隔で区切ったサンプル」とみなし、点の間を線形補間する。
// 点を増やすほど自由な形のカーブを作れる（インスペクタで数値を並べる＝点でグラフを描く感覚）。
public static class ResponseCurve {

    // keys(昇順) と values をペアにした折れ線。入力 x に対応する値を線形補間で返す。
    // keys が x 軸の位置を明示するので「どの点がどこか」が一目で分かる。
    // 例: keys={0,300,1000}, values={0,10,50} → 「速度300でダメージ10、それ以上は50へ向かう」。
    // x が keys の範囲外なら端の値でクランプ。keys が values と揃っていなければ等間隔とみなす。
    public static float Evaluate(List<float> keys, List<float> values, float x) {
        if (values == null || values.Count == 0) {
            return 0.0f;
        }
        int n = values.Count;
        if (keys == null || keys.Count < n) {
            return Evaluate(values, x); // keys 不備 → 等間隔フォールバック
        }
        if (n == 1) {
            return values[0];
        }
        if (x <= keys[0]) {
            return values[0];
        }
        if (x >= keys[n - 1]) {
            return values[n - 1];
        }
        for (int i = 0; i < n - 1; i++) {
            if (x <= keys[i + 1]) {
                float seg = keys[i + 1] - keys[i];
                float f = seg > 0.0f ? (x - keys[i]) / seg : 0.0f;
                return Mathf.Lerp(values[i], values[i + 1], f);
            }
        }
        return values[n - 1];
    }

    // DamagePoint の折れ線。speed 昇順を前提に、入力 x に対応する damage を線形補間で返す。
    // x が範囲外なら端の値でクランプ。点が無ければ 0。
    // ※ 呼び出し側で points を speed 昇順にソートしておくこと。
    public static float Evaluate(List<DamagePoint> points, float x) {
        if (points == null || points.Count == 0) {
            return 0.0f;
        }
        int n = points.Count;
        if (n == 1) {
            return points[0].damage;
        }
        if (x <= points[0].speed) {
            return points[0].damage;
        }
        if (x >= points[n - 1].speed) {
            return points[n - 1].damage;
        }
        for (int i = 0; i < n - 1; i++) {
            if (x <= points[i + 1].speed) {
                float seg = points[i + 1].speed - points[i].speed;
                float f = seg > 0.0f ? (x - points[i].speed) / seg : 0.0f;
                return Mathf.Lerp(points[i].damage, points[i + 1].damage, f);
            }
        }
        return points[n - 1].damage;
    }

    // t(0..1) に対応する値を線形補間で返す。points が空なら 0、1点ならその値。
    // points は等間隔の制御点（x が暗黙）。x を明示したい場合は keys+values 版を使う。
    public static float Evaluate(List<float> points, float t) {
        if (points == null || points.Count == 0) {
            return 0.0f;
        }
        if (points.Count == 1) {
            return points[0];
        }

        t = Mathf.Clamp01(t);
        float scaled = t * (points.Count - 1);
        int i = (int)scaled;
        if (i >= points.Count - 1) {
            return points[points.Count - 1];
        }
        return Mathf.Lerp(points[i], points[i + 1], scaled - i);
    }
}
