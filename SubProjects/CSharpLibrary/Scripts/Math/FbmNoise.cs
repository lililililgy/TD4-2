// fractal brownian motion ノイズ。
// 参考: https://thebookofshaders.com/13/?lan=jp
// 乱数ではなく座標から値が決まるので、同じ座標を渡せば必ず同じ値が返る（連続的に変化する）。
public static class FbmNoise {

    // 小数部分を取得
    public static float Fract(float x) {
        return x - Mathf.Floor(x);
    }

    // 各要素を切り捨て
    public static Vector2 Floor(Vector2 v) {
        return new Vector2(Mathf.Floor(v.x), Mathf.Floor(v.y));
    }

    // 各要素の小数部分を取得
    public static Vector2 Fract(Vector2 v) {
        return new Vector2(Fract(v.x), Fract(v.y));
    }

    // 疑似乱数生成（座標をハッシュして 0〜1 を返す）
    public static float Random(Vector2 st) {
        return Fract(Mathf.Sin(st.x * 12.9898f + st.y * 78.233f) * 43758.5453123f);
    }

    // 2D ノイズ（格子点の乱数を滑らかに補間）
    public static float Noise(Vector2 st) {
        Vector2 i = Floor(st);
        Vector2 f = Fract(st);

        float a = Random(i);
        float b = Random(new Vector2(i.x + 1.0f, i.y));
        float c = Random(new Vector2(i.x, i.y + 1.0f));
        float d = Random(new Vector2(i.x + 1.0f, i.y + 1.0f));

        Vector2 u = new Vector2(
            f.x * f.x * (3.0f - 2.0f * f.x),
            f.y * f.y * (3.0f - 2.0f * f.y));

        return Mathf.Lerp(a, b, u.x)
             + (c - a) * u.y * (1.0f - u.x)
             + (d - b) * u.x * u.y;
    }

    // fractal brownian motion ノイズ。戻り値はおおよそ 0〜1
    public static float Fbm(Vector2 st) {
        const int kOctaves = 6;
        float value = 0.0f;
        float amplitude = 0.5f;

        for (int i = 0; i < kOctaves; i++) {
            value += amplitude * Noise(st);
            st.x *= 2.0f;
            st.y *= 2.0f;
            amplitude *= 0.5f;
        }
        return value;
    }

    // Fbm を -1〜1 に変換したもの。揺れなど正負が必要な用途向け
    public static float Fbm11(Vector2 st) {
        return (Fbm(st) - 0.5f) * 2.0f;
    }
}
