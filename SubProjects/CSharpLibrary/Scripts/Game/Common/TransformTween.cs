// Transform(position / rotate / scale)を from → to へ時間で補間する汎用アニメーション。
// 3項目はそれぞれ独立に ON/OFF でき、OFF の項目には一切書き込まない。
//
// 他のスクリプトを一切参照しない(疎結合)。動かし方は次の2つだけ:
//   ・playOnInitialize_ … Initialize() のタイミングで自動再生(出現演出用)
//   ・Play() / Stop() / Pause() / Resume() … 他スクリプトから任意のタイミングで叩く
// 「終わったか」を外から知りたいときは CompletedCount(累計)の差分を見る。
// 毎フレーム立てるフラグにすると、読む側のスクリプト実行順で取りこぼすため用意していない。
//
// relative_(既定 ON)は「基準値からの相対」で動かす指定:
//   position … 基準位置 + 補間値(オフセット)
//   rotate   … 基準回転 * 補間値(度→クォータニオン)
//   scale    … 基準スケール * 補間値(軸別の倍率)
// 基準値は Play() の瞬間の transform から取る。既定値(位置0 / 回転0 / スケール1)のままなら
// 何も変わらないので、使う項目だけ値を入れればよい。
// relative_ を OFF にすると from / to をワールド値としてそのまま書き込む。
//
// position だけは「前フレームに足した分を引いてから今フレーム分を足す」方式で反映する。
// 移動スクリプトが毎フレーム position を書く Entity でも、揺れが溜まって本来の位置から
// ずれていかず、スクリプトの実行順にも依存しない(ShakeSelfOnDamaged と同じ考え方)。
// rotate / scale は基準値からの絶対書き込みなので、他が毎フレーム書く Entity には向かない。
//
// 時間は既定で Time.deltaTime(ヒットストップやスローの影響を受ける方)。
// UI など常に等速で動かしたいものは useUnscaledTime_ を ON にする。
//
// 【注意】enable=false のスクリプトは Update() 自体が呼ばれない(ECSGroup.UpdateEntities)。
// そのため「無効 → 有効」の瞬間をこのスクリプト単体では検知できない。無効化中は時間も
// 止まり、有効化後は続きから再生される。再表示のたびに頭から流したい場合は、
// 表示側から Play() を呼ぶか、replayOnResume_ を ON にする(下のコメント参照)。
// 再生の繰り返し方。
public enum TweenLoopMode
{
    Once,     // 1回で止まる(終了値で固定)
    Loop,     // from → to を繰り返す(毎回 from に飛び戻る)
    PingPong, // from → to → from を繰り返す
}

public class TransformTween : MonoScript
{

    // --- 再生 ---

    // Initialize() で自動再生する。スポーンやシーン開始と同時に流したいときに使う。
    [SerializeField] private bool playOnInitialize_ = true;

    // 「Update が飛んでいた = 自分か Entity が無効だった」とみなして頭から再生し直す。
    // 無効化を挟む UI の出現演出用。時間の跳びで判定する近似なので、極端な処理落ちが
    // あった直後に再生し直してしまうことがある。確実に制御したい場合は Play() を使う。
    [SerializeField] private bool replayOnResume_ = false;

    [SerializeField] private float duration_ = 0.3f; // from → to にかける秒数
    [SerializeField] private float delay_ = 0.0f; // 再生開始から動き出すまでの待ち(この間は from で固定)

    [SerializeField] private EaseType ease_ = EaseType.OutCubic;
    [SerializeField] private TweenLoopMode loop_ = TweenLoopMode.Once;
    [SerializeField] private bool useUnscaledTime_ = false;

    // 基準値(Play() 時の transform)からの相対で動かすか。OFF ならワールド値をそのまま書く。
    [SerializeField] private bool relative_ = true;

    // --- position(relative_ ON ならオフセット) ---
    [SerializeField] private bool animatePosition_ = false;
    [SerializeField] private Vector3 fromPosition_ = Vector3.zero;
    [SerializeField] private Vector3 toPosition_ = Vector3.zero;

    // --- rotate(度。relative_ ON なら基準回転からの回転量) ---
    [SerializeField] private bool animateRotation_ = false;
    [SerializeField] private Vector3 fromRotation_ = Vector3.zero;
    [SerializeField] private Vector3 toRotation_ = Vector3.zero;

    // --- scale(relative_ ON なら基準スケールに対する軸別の倍率) ---
    [SerializeField] private bool animateScale_ = false;
    [SerializeField] private Vector3 fromScale_ = Vector3.one;
    [SerializeField] private Vector3 toScale_ = Vector3.one;

    private bool playing_ = false;
    private float elapsed_ = 0.0f; // delay_ 込みの再生開始からの経過秒
    private int completedCount_ = 0;
    private int lastCycle_ = 0;    // ループ回数の数え漏れ防止(CompletedCount 用)

    // 基準値。relative_ のときだけ使う(position はオフセット方式なので基準を持たない)。
    private Quaternion baseRotation_ = Quaternion.identity;
    private Vector3 baseScale_ = Vector3.one;
    private bool hasBase_ = false;

    // 前フレームに position へ足したオフセット
    private Vector3 appliedOffset_ = Vector3.zero;
    private bool hasAppliedOffset_ = false;

    // replayOnResume_ 用。前回 Update した時刻(Time.time は累計のゲーム時間)。
    private float lastUpdateTime_ = 0.0f;
    private bool hasUpdatedOnce_ = false;

    /// =================================
    /// 外部から叩く API
    /// =================================

    public bool IsPlaying => playing_;

    // 再生が最後まで到達した回数の累計。ループ時は1周ごと、PingPong は片道ごとに増える。
    // 外からは「前フレームの値との差 > 0」で完了を検知する。
    public int CompletedCount => completedCount_;

    // 頭から再生する。基準値もこの瞬間の transform から取り直す。
    public void Play()
    {
        if (entity == null || entity.transform == null)
        {
            return;
        }

        // 前回の反映を消してから基準値を取る(演出中に再生し直しても基準がずれない)
        RemoveAppliedOffset();
        RestoreBase();

        baseRotation_ = transform.rotate;
        baseScale_ = transform.scale;
        hasBase_ = true;

        elapsed_ = 0.0f;
        lastCycle_ = 0;
        playing_ = true;

        // 開始値を即座に当てる。1フレームだけ元の見た目が出るのを防ぐ。
        Apply(0.0f);
    }

    // 止めて基準値へ戻す。
    public void Stop()
    {
        playing_ = false;
        elapsed_ = 0.0f;

        RemoveAppliedOffset();
        RestoreBase();
    }

    // 今の見た目のまま止める。Resume() で続きから。
    public void Pause()
    {
        playing_ = false;
    }

    public void Resume()
    {
        if (hasBase_ || !relative_)
        {
            playing_ = true;
        }
        else
        {
            Play(); // 一度も再生していないなら頭から
        }
    }

    /// =================================
    /// MonoScript
    /// =================================

    public override void Initialize()
    {
        if (playOnInitialize_)
        {
            Play();
        }
    }

    public override void Update()
    {
        // 撃破などで破棄された Entity は transform が null になる。
        // 破棄したフレームも Update は走るので、触る前に必ず見る。
        if (entity == null || entity.transform == null)
        {
            return;
        }

        if (CheckResumed())
        {
            Play();
        }

        if (!playing_)
        {
            return;
        }

        elapsed_ += useUnscaledTime_ ? Time.unscaledDeltaTime : Time.deltaTime;

        Apply(CalcProgress());
    }

    /// =================================
    /// 内部
    /// =================================

    // 経過時間 → 進行度 0〜1。ループの周回数え上げと Once の停止もここで行う。
    private float CalcProgress()
    {
        float active = elapsed_ - delay_;
        if (active <= 0.0f)
        {
            return 0.0f; // 待機中は from で固定
        }

        if (duration_ <= 0.0f)
        {
            // 秒数指定なしは即座に終了値へ(0除算回避も兼ねる)
            if (loop_ == TweenLoopMode.Once)
            {
                playing_ = false;
            }
            completedCount_++;
            return 1.0f;
        }

        float cycles = active / duration_;

        if (loop_ == TweenLoopMode.Once)
        {
            if (cycles >= 1.0f)
            {
                playing_ = false;
                completedCount_++;
                return 1.0f;
            }
            return cycles;
        }

        int cycleIndex = (int)cycles;
        float fraction = cycles - cycleIndex;

        // 1フレームで複数周した場合も数え落とさないように差分で足す
        if (cycleIndex > lastCycle_)
        {
            completedCount_ += cycleIndex - lastCycle_;
            lastCycle_ = cycleIndex;
        }

        // PingPong は奇数周を逆再生にして from → to → from にする
        if (loop_ == TweenLoopMode.PingPong && (cycleIndex % 2) != 0)
        {
            return 1.0f - fraction;
        }

        return fraction;
    }

    private void Apply(float t)
    {
        // Back / Elastic は 0〜1 をはみ出して返る。行き過ぎ表現なのでそのまま使う。
        float e = EaseUtil.Evaluate(ease_, Mathf.Clamp01(t));

        if (animatePosition_)
        {
            RemoveAppliedOffset();

            Vector3 value = Vector3.Lerp(fromPosition_, toPosition_, e);
            if (relative_)
            {
                // 移動スクリプトと取り合わないよう、毎フレーム「引いてから足す」
                transform.position = transform.position + value;
                appliedOffset_ = value;
                hasAppliedOffset_ = true;
            }
            else
            {
                transform.position = value;
            }
        }

        if (animateRotation_)
        {
            Vector3 degrees = Vector3.Lerp(fromRotation_, toRotation_, e);
            Quaternion value = Quaternion.FromEuler(degrees * Mathf.Deg2Rad);
            transform.rotate = relative_ ? baseRotation_ * value : value;
        }

        if (animateScale_)
        {
            Vector3 value = Vector3.Lerp(fromScale_, toScale_, e);
            transform.scale = relative_
                ? new Vector3(baseScale_.x * value.x, baseScale_.y * value.y, baseScale_.z * value.z)
                : value;
        }
    }

    private void RemoveAppliedOffset()
    {
        if (!hasAppliedOffset_)
        {
            return;
        }

        transform.position = transform.position - appliedOffset_;
        appliedOffset_ = Vector3.zero;
        hasAppliedOffset_ = false;
    }

    // rotate / scale を基準値へ戻す。position はオフセット方式なので別(RemoveAppliedOffset)。
    private void RestoreBase()
    {
        if (!hasBase_ || !relative_)
        {
            return;
        }

        if (animateRotation_)
        {
            transform.rotate = baseRotation_;
        }

        if (animateScale_)
        {
            transform.scale = baseScale_;
        }
    }

    // Update が飛んでいた(= enable が落ちていた)かを時間の跳びで判定する。
    // enable=false の間は Update 自体が呼ばれないため、これ以外に検知手段がない。
    private bool CheckResumed()
    {
        float now = Time.time;
        bool resumed = false;

        if (replayOnResume_ && hasUpdatedOnce_)
        {
            float gap = now - lastUpdateTime_;
            resumed = gap > Time.deltaTime * 2.0f + 0.05f;
        }

        lastUpdateTime_ = now;
        hasUpdatedOnce_ = true;
        return resumed;
    }
}
