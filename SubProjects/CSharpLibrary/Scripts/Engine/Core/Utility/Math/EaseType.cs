// Ease.cs の補間関数を「インスペクタで選べる列挙」にしたもの。
// Ease.cs 側は関数の集まりなので、SerializeField に出すにはこの enum を経由する。
//
// カメラ側には同じ役割の ZoomEaseType(Game/Camera/ZoomEase.cs)が既にあるが、
// あちらは既存のプレハブが値を保存済みなので触らずそのままにしてある。
// 新しく Ease を選ばせたいスクリプトは、Game 層に依存しないこちらを使う。
public enum EaseType {
    Linear,

    InSine,    InQuad,    InCubic,    InQuart,    InQuint,
    InExpo,    InCirc,    InBack,     InElastic,  InBounce,

    OutSine,   OutQuad,   OutCubic,   OutQuart,   OutQuint,
    OutExpo,   OutCirc,   OutBack,    OutElastic, OutBounce,

    InOutSine, InOutQuad, InOutCubic, InOutQuart, InOutQuint,
    InOutExpo, InOutCirc, InOutBack,  InOutElastic, InOutBounce,
}

// EaseType から Ease.cs の関数を引くだけの変換。
public static class EaseUtil {

    // t は 0〜1 を想定。Back / Elastic は戻り値が 0〜1 をはみ出す(行き過ぎてから戻る)ので、
    // はみ出して困る用途では呼び出し側でクランプする。
    public static float Evaluate(EaseType type, float t) {
        switch (type) {
        case EaseType.InSine:       return Ease.In.Sine(t);
        case EaseType.InQuad:       return Ease.In.Quad(t);
        case EaseType.InCubic:      return Ease.In.Cubic(t);
        case EaseType.InQuart:      return Ease.In.Quart(t);
        case EaseType.InQuint:      return Ease.In.Quint(t);
        case EaseType.InExpo:       return Ease.In.Expo(t);
        case EaseType.InCirc:       return Ease.In.Circ(t);
        case EaseType.InBack:       return Ease.In.Back(t);
        case EaseType.InElastic:    return Ease.In.Elastic(t);
        case EaseType.InBounce:     return Ease.In.Bounce(t);

        case EaseType.OutSine:      return Ease.Out.Sine(t);
        case EaseType.OutQuad:      return Ease.Out.Quad(t);
        case EaseType.OutCubic:     return Ease.Out.Cubic(t);
        case EaseType.OutQuart:     return Ease.Out.Quart(t);
        case EaseType.OutQuint:     return Ease.Out.Quint(t);
        case EaseType.OutExpo:      return Ease.Out.Expo(t);
        case EaseType.OutCirc:      return Ease.Out.Circ(t);
        case EaseType.OutBack:      return Ease.Out.Back(t);
        case EaseType.OutElastic:   return Ease.Out.Elastic(t);
        case EaseType.OutBounce:    return Ease.Out.Bounce(t);

        case EaseType.InOutSine:    return Ease.InOut.Sine(t);
        case EaseType.InOutQuad:    return Ease.InOut.Quad(t);
        case EaseType.InOutCubic:   return Ease.InOut.Cubic(t);
        case EaseType.InOutQuart:   return Ease.InOut.Quart(t);
        case EaseType.InOutQuint:   return Ease.InOut.Quint(t);
        case EaseType.InOutExpo:    return Ease.InOut.Expo(t);
        case EaseType.InOutCirc:    return Ease.InOut.Circ(t);
        case EaseType.InOutBack:    return Ease.InOut.Back(t);
        case EaseType.InOutElastic: return Ease.InOut.Elastic(t);
        case EaseType.InOutBounce:  return Ease.InOut.Bounce(t);

        default:                    return t; // Linear
        }
    }
}
