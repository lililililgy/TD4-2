// 被弾したときにスプライトを一瞬だけ光らせる（フラッシュさせる）演出。
// HP と同じ Entity にアタッチする。与えた側の ShakeOnHit / HitStopOnHit に対する、受けた側の演出。
//
// 無敵(i-frame)は一切持たない。プレイヤーの DamageFeedback は「連続ヒットで一気に溶けるのを防ぐ」
// ために無敵とセットだが、雑魚敵は速度を乗せて何度も殴れることが手触りの中心なので、
// ここでは見た目だけを変える。当たった回数だけダメージは通り、フラッシュはそのたびに測り直す。
//
// 検知は ShakeOnDamaged と同じく HP.TotalDamageTaken(累計)の差分で行う。理由:
//   ・「今フレーム被弾した」フラグ方式は、立てる側と読む側の実行順で取りこぼす。
//   ・CurrentHp の減少では代用できない（回復や外部からの同期と区別が付かない）。
//
// タイマーは Time.deltaTime(timeScale の影響を受ける方)で測る。ヒットストップ中はフラッシュも
// 一緒に止まってほしいため。実時間で測ると、止まっている絵の中でフラッシュだけが先に消えて
// 「止めた瞬間の光った絵」が見えなくなる。
public class FlashOnDamaged : MonoScript {

    // 被弾した瞬間に置き換える色。x=R, y=G, z=B, w=A。
    // 既定は白飛び気味の明るい色（元の色味を選ばず、殴った手応えとして読み取りやすい）。
    [SerializeField] private Vector4 flashColor_ = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);

    // フラッシュの長さ(秒)。短く鋭いほど連打の1発1発が分離して見える。
    [SerializeField] private float duration_ = 0.08f;

    // true: フラッシュ色から元色へ減衰しながら戻る（滑らか）
    // false: duration_ の間は点きっぱなしで、終わった瞬間に元へ戻る（硬い・強い）
    [SerializeField] private bool fadeOut_ = true;

    private HP             hp_;
    private SpriteRenderer renderer_;
    private Vector4        baseColor_;      // 戻り先（白とは限らないので実値を控える）
    private float          lastTotalDamage_;

    private bool  flashing_ = false;
    private float elapsed_  = 0.0f;

    public override void Initialize() {
        hp_       = entity.GetScript<HP>();
        renderer_ = entity.GetComponent<SpriteRenderer>();
        if (renderer_ != null) {
            baseColor_ = renderer_.color;
        }

        // 開始時点の累計を基準にする（初期化前の分で1発光ってしまうのを防ぐ）
        lastTotalDamage_ = hp_ != null ? hp_.TotalDamageTaken : 0.0f;
    }

    public override void Update() {
        if (hp_ == null || renderer_ == null) {
            return;
        }

        // 撃破された Entity は Entity.Destroy() で components が外れる（transform も null になる）。
        // HP.Update() が先に走って破棄するため、とどめの一撃のフレームはここへ来る。
        // 描画の実体はもう無いので、色を書いても意味が無い＝そのまま抜ける。
        if (entity == null || entity.transform == null) {
            return;
        }

        float total  = hp_.TotalDamageTaken;
        float damage = total - lastTotalDamage_;
        lastTotalDamage_ = total;

        // 実際に HP が削れたときだけ光る。無敵の敵やダメージ0の当たりでは光らない。
        // フラッシュ中の再被弾は、消えかけから点け直す（先頭から測り直す）。
        if (damage > 0.0f) {
            flashing_ = true;
            elapsed_  = 0.0f;
        }

        if (!flashing_) {
            return;
        }

        elapsed_ += Time.deltaTime;

        // duration_ が 0 以下だと割れるうえ演出にならないので、その場合は光らせない
        if (duration_ <= 0.0f || elapsed_ >= duration_) {
            renderer_.color = baseColor_;
            flashing_ = false;
            return;
        }

        renderer_.color = fadeOut_
            ? LerpColor(flashColor_, baseColor_, elapsed_ / duration_)
            : flashColor_;
    }

    private static Vector4 LerpColor(Vector4 from, Vector4 to, float t) {
        return new Vector4(
            Mathf.Lerp(from.x, to.x, t),
            Mathf.Lerp(from.y, to.y, t),
            Mathf.Lerp(from.z, to.z, t),
            Mathf.Lerp(from.w, to.w, t));
    }
}
