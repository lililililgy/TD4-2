using System;

// ダッシュに反応する寄与。値の動きは3段階。
//   1. ダッシュ開始の瞬間 → enterStrength_ へ跳ね上げる（スナップ）
//   2. ダッシュ中         → dashingStrength_ へ収束する（跳ねた分が落ち着く）
//   3. ダッシュ終了後     → 0 へ収束する
// カメラ Entity に付ける。
//
// PlayerDashedEvent / PlayerDashEndedEvent は PlayerDashState の OnEnter()/OnExit() が
// 遷移確定時に1回だけ発行するので、押しっぱなしでも連続加算にはならない。
// MessageBus は同期発行なので他スクリプトの実行順には依存しない。
//
// ここで作るのは 0〜1 の強さまで。倍率への変換（振れ幅・カーブ・追従）は基底が持つ。
public class DashZoomSource : CameraZoomSource
{

    [SerializeField] private float enterStrength_ = 1.0f; // 開始の瞬間に跳ねる値
    [SerializeField] private float dashingStrength_ = 0.4f; // ダッシュ中に落ち着く値
    [SerializeField] private float convergeSpeed_ = 0.9f; // 1秒で目標へ詰める割合（0=動かない、1に近いほど速い）

    private float strength_;
    private bool dashing_ = false;
    private bool subscribed_ = false;

    public override float Strength()
    {
        return strength_;
    }

    // 初回の取りこぼしを避けるため Initialize() ではなく Awake() で購読する。
    public override void Awake()
    {
        if (!subscribed_)
        {
            MessageBus.Subscribe<PlayerDashedEvent>(OnDashed);
            MessageBus.Subscribe<PlayerDashEndedEvent>(OnDashEnded);
            subscribed_ = true;
        }
    }

    // 静的な MessageBus に購読が残るとリークするので必ず解除する。
    protected override void OnSourceDestroy()
    {
        if (subscribed_)
        {
            MessageBus.Unsubscribe<PlayerDashedEvent>(OnDashed);
            MessageBus.Unsubscribe<PlayerDashEndedEvent>(OnDashEnded);
            subscribed_ = false;
        }
    }

    protected override void UpdateStrength()
    {
        // ダッシュ中は dashingStrength_、抜けたら 0 が目標。
        // 跳ね上げた直後は上から、静止状態からは下から、同じ式で目標へ寄っていく。
        float target = dashing_ ? Mathf.Clamp01(dashingStrength_) : 0.0f;

        float t = 1.0f;
        if (convergeSpeed_ > 0.0f && convergeSpeed_ < 1.0f)
        {
            // 詰める割合を deltaTime で累乗することでフレームレートに依存しない
            t = 1.0f - Mathf.Pow(1.0f - convergeSpeed_, Time.deltaTime);
        }
        else if (convergeSpeed_ <= 0.0f)
        {
            t = 0.0f; // 収束させない（跳ねた値を保持する）
        }

        strength_ = Mathf.Lerp(strength_, target, t);
    }

    // 開始の瞬間だけは収束を挟まず跳ねさせる。ここを Lerp にすると出足が鈍る。
    private void OnDashed(PlayerDashedEvent e)
    {
        dashing_ = true;
        strength_ = Mathf.Clamp01(enterStrength_);
    }

    private void OnDashEnded(PlayerDashEndedEvent e)
    {
        dashing_ = false;
    }
}
