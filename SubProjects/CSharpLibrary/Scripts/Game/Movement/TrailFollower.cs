using System;

// 隊列の1ノード。リーダー(TrailLeader コンポーネント)に自分を登録し、追従の計算は TrailLeader に委ねる。
// 自分は「chain 名 / order / 追従距離レンジ / 追従感」のパラメータを持つだけの薄いメンバー。
//
// PBD(FTL)なので保持するのは現在位置と速度バッファだけ。経路履歴もノード間ポインタも持たない。
// 目標は TrailLeader.Solve() から「前ノードの現在位置 prev」を受け取って計算する。
// 前ノードとの距離を [min, max] のレッシュ(首ひも)として拘束する：
//   ・min より近づいたら min まで押し戻す（潰れて重ならない）
//   ・max より離れたら max まで引き戻す（離れすぎない＝前進中はここで曳かれる）
//   ・min〜max の範囲内は拘束しない（自由に動ける）
public class TrailFollower : MonoScript {

    [SerializeField] private string leaderName_     = "Player";    // 追従するリーダー名
    [SerializeField] private string chainName_      = "Default";   // 同じ TrailLeader 上で独立した隊列を分ける名前
    [SerializeField] private int    order_          = 1;           // 隊列の順番（1が先頭）
    [SerializeField] private float  leadMinOffset_  = 4.0f;        // リーダー → 先頭ノード の最小距離
    [SerializeField] private float  leadMaxOffset_  = 8.0f;        // リーダー → 先頭ノード の最大距離
    [SerializeField] private float  unitMinOffset_  = 4.0f;        // ノード間（前ノード → 自分）の最小距離
    [SerializeField] private float  unitMaxOffset_  = 8.0f;        // ノード間（前ノード → 自分）の最大距離
    [SerializeField] private float  smoothTime_     = 0.08f;       // 追従の滑らかさ（0で即時）
    [SerializeField] private float  maxSmoothSpeed_ = 100000.0f;   // 追従速度の上限

    private Vector3 smoothVel_  = Vector3.zero;
    private bool    registered_ = false;

    public string ChainName { get { return chainName_; } }
    public int    Order      { get { return order_; } }

    // 動的な隊列（卵など）は外部マネージャが SetOrder() で順番を差し込む。
    public void SetOrder(int order) { order_ = order; }

    public override void Initialize() {
        TryRegister();
    }

    public override void Update() {
        // リーダー(や自分)の生成順に強くするため、登録できるまで毎フレーム試す。
        // 登録後は TrailLeader.Update が位置を駆動するので、ここでは何もしない。
        if (!registered_) {
            TryRegister();
        }
    }

    // TrailLeader から order 順に呼ばれる。前ノードの「更新後の現在位置」prev を基準に、
    // 距離を [min, max] に拘束した点へスムーズに寄せ、更新後の自分の位置を返す(次ノードの prev になる)。
    public Vector3 Solve(Vector3 prev, bool isFront, float dt) {
        // 先頭(生存ノードの最前)はリーダーに、それ以外は前ノードに対するレンジを使う。
        float minD = isFront ? leadMinOffset_ : unitMinOffset_;
        float maxD = isFront ? leadMaxOffset_ : unitMaxOffset_;
        if (maxD < minD) {
            maxD = minD; // min/max 逆転の設定ミスは max=min に丸める
        }
        Vector3 cur = transform.position;

        // 前ノード → 自分 の方向。重なって長さが出ないフレームは真下にフォールバック
        // （ゼロ方向だと target が prev に潰れて重なるのを防ぐ）。
        Vector3 dir = cur - prev;
        float   dl  = dir.Length();
        Vector3 unit = (dl > kEps) ? dir * (1.0f / dl) : new Vector3(0.0f, -1.0f, 0.0f);

        // レッシュ拘束：範囲内(min〜max)は現在距離を保つ＝拘束しない。範囲外だけ境界へ引き戻す。
        float   targetDist = (dl < minD) ? minD : ((dl > maxD) ? maxD : dl);
        Vector3 target     = prev + unit * targetDist;

        Vector3 pos = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
            cur, target, ref smoothVel_, smoothTime_, dt, maxSmoothSpeed_);
        transform.position = pos;

        // 進行方向を向く（2D・Z軸回り）。ほぼ静止しているフレームは今の向きを保持する。
        Vector3 move = pos - cur;
        if (move.x * move.x + move.y * move.y > FaceEpsilonSq) {
            float roll = Mathf.Atan2(move.x, move.y);
            transform.rotate = Quaternion.MakeFromAxis(Vector3.back, roll);
        }

        return pos;
    }

    private void TryRegister() {
        if (registered_) {
            return;
        }
        Entity le = ecsGroup.FindEntity(leaderName_);
        if (le == null) {
            return;
        }
        TrailLeader leader = le.GetScript<TrailLeader>();
        if (leader == null) {
            return;
        }
        leader.Register(this);
        registered_ = true;
    }

    // ベクトル長のゼロ割り回避しきい値
    private const float kEps = 1e-4f;

    // この距離(二乗)未満しか進んでいないフレームは回転を更新しない（向きのちらつき防止）
    private const float FaceEpsilonSq = 0.0001f;
}
