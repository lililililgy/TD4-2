using System;

// 強さ → 倍率の変化の付け方。Ease.cs の関数に対応する。
public enum ZoomEaseType {
    Linear,
    InSine,    InQuad,    InCubic,    InExpo,    InBack,
    OutSine,   OutQuad,   OutCubic,   OutExpo,   OutBack,
    InOutSine, InOutQuad, InOutCubic, InOutExpo, InOutBack,
}

// ZoomEaseType から Ease.cs の関数を引くだけの変換。
public static class ZoomEase {

    // Back 系は 0〜1 をはみ出す（行き過ぎてから戻る）。呼び出し側でクランプしない限りそのまま返る。
    public static float Evaluate(ZoomEaseType type, float t) {
        switch (type) {
        case ZoomEaseType.InSine:     return Ease.In.Sine(t);
        case ZoomEaseType.InQuad:     return Ease.In.Quad(t);
        case ZoomEaseType.InCubic:    return Ease.In.Cubic(t);
        case ZoomEaseType.InExpo:     return Ease.In.Expo(t);
        case ZoomEaseType.InBack:     return Ease.In.Back(t);

        case ZoomEaseType.OutSine:    return Ease.Out.Sine(t);
        case ZoomEaseType.OutQuad:    return Ease.Out.Quad(t);
        case ZoomEaseType.OutCubic:   return Ease.Out.Cubic(t);
        case ZoomEaseType.OutExpo:    return Ease.Out.Expo(t);
        case ZoomEaseType.OutBack:    return Ease.Out.Back(t);

        case ZoomEaseType.InOutSine:  return Ease.InOut.Sine(t);
        case ZoomEaseType.InOutQuad:  return Ease.InOut.Quad(t);
        case ZoomEaseType.InOutCubic: return Ease.InOut.Cubic(t);
        case ZoomEaseType.InOutExpo:  return Ease.InOut.Expo(t);
        case ZoomEaseType.InOutBack:  return Ease.InOut.Back(t);

        default:                      return t; // Linear
        }
    }
}
