// 被弾したときに「その Entity 自身」を小刻みに揺らす演出。HP と同じ Entity にアタッチする。
// FlashOnDamaged と対になる被弾リアクションで、こちらも無敵は持たない。
//
// 同じ「被弾で揺れる」でも ShakeOnDamaged とは揺らす対象が違うので注意:
//   ・ShakeOnDamaged      … カメラを揺らす（画面全体が揺れる）
//   ・ShakeSelfOnDamaged  … 自分の transform だけを揺らす（その敵だけが震える）
// 雑魚敵にカメラシェイクを足すと、攻撃側の ShakeOnHit と二重に揺れて画面が破綻するため、
// 敵側はこちらを使う。
//
// 揺れの計算はカメラと同じ ShakeSource（ノイズ＋残り時間による減衰）を実行時に1つ持って使う。
// 揺れ方を揃えたいだけなので、アタッチはせず new して時間送り(AdvanceTime)も自分で行う。
//
// transform への反映は「前フレームに足した分を引いてから今フレーム分を足す」方式。
// 敵は移動スクリプトが毎フレーム position を書くので、単純に足すだけだと揺れが溜まって
// 本来の位置からずれていく。この方式なら移動スクリプトとの実行順にも依存しない。
//
// タイマーは FlashOnDamaged と同じく Time.deltaTime(減速の影響を受ける方)。
// ヒットストップ中は揺れも止まってほしい（止まった絵の中で1体だけ震え続けないように）。
public class ShakeSelfOnDamaged : MonoScript {

    // 揺れ幅（ワールド単位）。雑魚敵の見た目は数百単位の大きさなので、
    // 数十で「ぶるっと震える」程度になる。0 以下で無効。
    [SerializeField] private float amplitude_ = 24.0f;

    // 揺れの継続時間(秒)。残り時間に比例して自動で収束する。
    [SerializeField] private float duration_ = 0.12f;

    // 揺れの速さ。大きいほど細かく震える。
    [SerializeField] private float frequency_ = 35.0f;

    private HP    hp_;
    private float lastTotalDamage_;

    private ShakeSource shake_;

    // 前フレームに transform へ足したオフセット
    private Vector3 appliedOffset_    = Vector3.zero;
    private bool    hasAppliedOffset_ = false;

    // 連続被弾で毎回同じ揺れ方にならないよう、発生のたびにパターンをずらす
    private float noiseSeedCounter_ = 0.0f;

    public override void Initialize() {
        hp_ = entity.GetScript<HP>();
        // 開始時点の累計を基準にする（初期化前の分で1発揺れてしまうのを防ぐ）
        lastTotalDamage_ = hp_ != null ? hp_.TotalDamageTaken : 0.0f;

        shake_ = new ShakeSource();
        // アタッチしていない ShakeSource は既定で有効なままなので、被弾までは止めておく
        shake_.StopShake();
    }

    public override void Update() {
        if (hp_ == null || shake_ == null) {
            return;
        }

        // 撃破された Entity は Entity.Destroy() の中で transform が null になる。
        // HP.Update() はこのスクリプトより先に走って死亡時の破棄を行うため、
        // とどめの一撃のフレームは「破棄済みの自分」でここへ来る。触る前に必ず見る。
        if (entity == null || entity.transform == null) {
            return;
        }

        // 前フレームの揺れを打ち消してから、今フレームの揺れを乗せ直す
        RemoveAppliedOffset();

        float total  = hp_.TotalDamageTaken;
        float damage = total - lastTotalDamage_;
        lastTotalDamage_ = total;

        // 実際に HP が削れたときだけ揺れる。揺れている最中の再被弾は先頭から測り直す。
        if (damage > 0.0f) {
            TriggerShake();
        }

        // 持続時間を過ぎていれば false が返り、このフレームは揺らさない
        if (!shake_.AdvanceTime(Time.deltaTime)) {
            return;
        }

        Vector3 offset = shake_.Evaluate();
        if (offset.x == 0.0f && offset.y == 0.0f && offset.z == 0.0f) {
            return;
        }

        appliedOffset_     = offset;
        hasAppliedOffset_  = true;
        transform.position = transform.position + offset;
    }

    private void TriggerShake() {
        if (amplitude_ <= 0.0f || duration_ <= 0.0f) {
            return;
        }

        shake_.Setup(
            ShakeSourceType.Noise,
            duration_,
            amplitude_,
            frequency_ > 0.0f ? frequency_ : 1.0f,
            noiseSeedCounter_);
        shake_.StartShake();

        noiseSeedCounter_ += 13.1f;
    }

    private void RemoveAppliedOffset() {
        if (!hasAppliedOffset_) {
            return;
        }

        transform.position = transform.position - appliedOffset_;
        appliedOffset_     = Vector3.zero;
        hasAppliedOffset_  = false;
    }
}
