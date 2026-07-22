using System;

// カメラの transform.scale に自分の分を足し込む演出の抽象基底。カメラ Entity に付ける。
// position には触れない（PlayerFollowCamera と競合しないため）。
//
// 各ソースは「自分の強さ(0〜1)」と「足す量の min/max・イージング・追従の速さ」を自分で持つ。
// そのため衝突とダッシュのように、同じカメラに対して別々の効き方をさせられる。
//
// 書き込みは "自分が今足している分" との差分だけを加算する方式。
// 直接代入すると後に走ったソースが他を消してしまうが、差分加算なら
// スクリプトの実行順に関係なく全ソースの合計になる。
// 寄与が無いときの倍率は、Transform に設定した scale がそのまま基準になる。
//
// 新しい演出条件を足したいときは、このクラスを継承して Strength() を返すだけでよい。
public abstract class CameraZoomSource : MonoScript
{

    // 派生クラスからも触れるよう protected。private だとシリアライズ登録
    // （EntityComponentSystem.InitializeSerializeFieldCache の GetFields）が派生型で拾えない。
    [SerializeField] protected float minAddScale_ = 0.0f;                  // Strength()=0 のとき足す量
    [SerializeField] protected float maxAddScale_ = 0.6f;                  // Strength()=1 のとき足す量（+で引き、-で寄り）
    [SerializeField] protected ZoomEaseType easeType_ = ZoomEaseType.OutCubic; // 0→1 の効き方
    [SerializeField] protected float smoothing_ = 0.9f;                  // 1秒で目標へ詰める割合（0=即時、1に近いほど速い）

    private float applied_; // 今 transform.scale に足し込んである自分の分

    // このソースの現在の強さ。0〜1 で返す。
    public abstract float Strength();

    // 派生はこちらを実装する。足し込みの手順を固定するため Update() は sealed。
    protected virtual void UpdateStrength() { }

    // 破棄時の後始末。購読解除などがあれば派生はこちらに書く。
    protected virtual void OnSourceDestroy() { }

    public sealed override void Initialize()
    {
        // 起動直後にガクッと動かないよう、平滑を通さず現在値へ合わせる
        UpdateStrength();
        Apply(TargetAdd());
    }

    public override void Update()
    {
        UpdateStrength();

        float t = 1.0f;
        if (smoothing_ > 0.0f && smoothing_ < 1.0f)
        {
            // 詰める割合を deltaTime で累乗することでフレームレートに依存しない
            t = 1.0f - Mathf.Pow(1.0f - smoothing_, Time.deltaTime);
        }

        // 前フレームの自分の値 → 今回の目標 を Lerp で緩やかに繋ぐ
        Apply(Mathf.Lerp(applied_, TargetAdd(), t));
    }

    // 自分の寄与を戻してから消える。残すと足した分が乗りっぱなしになる。
    public sealed override void OnDestroy()
    {
        Apply(0.0f);
        OnSourceDestroy();
    }

    private float TargetAdd()
    {
        // Back 系は 0〜1 をはみ出す（行き過ぎてから戻る）ので、
        // ここではクランプせず min/max の外まで振らせる。
        float shaped = ZoomEase.Evaluate(easeType_, Mathf.Clamp01(Strength()));
        return Mathf.Lerp(minAddScale_, maxAddScale_, shaped);
    }

    // 差分だけ足す。他のソースが書いた分には触れない。
    private void Apply(float add)
    {
        float delta = add - applied_;
        applied_ = add;

        if (delta == 0.0f)
        {
            return;
        }

        Vector3 scale = transform.scale;
        transform.scale = new Vector3(scale.x + delta, scale.y + delta, scale.z + delta);
    }
}
