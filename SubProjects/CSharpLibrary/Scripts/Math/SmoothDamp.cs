using System;

using V3 = global::Vector3;

/// <summary>
/// SmoothDamp を型 T に対して使えるようにするための特性定義。
/// Magnitude と ClampMagnitude を提供する。
/// </summary>
public interface IDampTraits<T> {
    /// <summary>変化量の大きさを返す</summary>
    float Magnitude(T v);

    /// <summary>大きさを max に制限する</summary>
    T ClampMagnitude(T v, float max);

    // T 上の最小限の線形演算（SmoothDamp 本体が必要とするもの）
    T Sub(T a, T b);          // a - b
    T Add(T a, T b);          // a + b
    T Scale(T v, float s);    // v * s
}

/// <summary>float 用の DampTraits</summary>
public readonly struct FloatDampTraits : IDampTraits<float> {
    public float Magnitude(float v) => Math.Abs(v);

    public float ClampMagnitude(float v, float max)
        => Math.Min(Math.Max(v, -max), max);

    public float Sub(float a, float b) => a - b;
    public float Add(float a, float b) => a + b;
    public float Scale(float v, float s) => v * s;
}

/// <summary>Vector3 用の DampTraits</summary>
public readonly struct Vector3DampTraits : IDampTraits<V3> {
    public float Magnitude(V3 v) => v.Length();

    public V3 ClampMagnitude(V3 v, float max) {
        float lenSq = V3.Dot(v, v);
        if (lenSq > max * max) {
            return v.Normalized() * max;
        }
        return v;
    }

    public V3 Sub(V3 a, V3 b) => a - b;
    public V3 Add(V3 a, V3 b) => a + b;
    public V3 Scale(V3 v, float s) => v * s;
}

public static class SpringDamper {
    private const float kEpsilon = 1e-5f;

    /// <summary>
    /// from から to へ、currentVelocity を考慮しながら smoothTime の時間をかけて移動させる。
    /// </summary>
    /// <typeparam name="T">移動対象の型</typeparam>
    /// <typeparam name="TTraits">T 用の DampTraits 実装</typeparam>
    /// <param name="from">移動前</param>
    /// <param name="to">目標値（実際にこの値になるとは限らない）</param>
    /// <param name="currentVelocity">速度。呼び出しをまたいで持ち越す（ref）</param>
    /// <param name="smoothTime">到達までのおおよその時間</param>
    /// <param name="deltaTime">フレーム時間</param>
    /// <param name="maxSpeed">最大速度</param>
    /// <returns>更新後の位置</returns>
    public static T SmoothDamp<T, TTraits>(
        T from,
        T to,
        ref T currentVelocity,
        float smoothTime,
        float deltaTime,
        float maxSpeed)
        where TTraits : struct, IDampTraits<T> {
        TTraits traits = default;

        // e^(-x) を近似するための係数（パデ近似）
        const float kExpCoeff2 = 0.48f;
        const float kExpCoeff3 = 0.235f;

        smoothTime = Math.Max(kEpsilon, smoothTime);

        float omega = 2f / smoothTime;
        float x = omega * deltaTime;
        float exp = 1f / (1f + x + kExpCoeff2 * x * x + kExpCoeff3 * x * x * x);

        T change = traits.Sub(from, to);              // from - to （後で符号を戻す）
                                                      // 元実装は change = to - from だが、Unity 同様 from-to で扱い最後に from 基準へ戻す
        change = traits.Sub(to, from);                // change = to - from

        float maxChange = maxSpeed * smoothTime;
        change = traits.ClampMagnitude(change, maxChange);

        // temp = (currentVelocity - omega * change) * deltaTime
        T temp = traits.Scale(
            traits.Sub(currentVelocity, traits.Scale(change, omega)),
            deltaTime);

        // currentVelocity = (currentVelocity - omega * temp) * exp
        currentVelocity = traits.Scale(
            traits.Sub(currentVelocity, traits.Scale(temp, omega)),
            exp);

        // return from + (change + temp) * exp
        return traits.Add(from, traits.Scale(traits.Add(change, temp), exp));
    }
}
