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
// さらに「曲げ角」も拘束できる(maxBendRad_)：このセグメント(前ノード→自分)の向きを、
// 基準向き refDir(前セグメントの向き／先頭はリーダー後方)から ±maxBendRad_ 以内に収める。
// = 隊列が急角度に折れ曲がらない＝ロープの曲げ剛性。π(≈3.14159) で無制限(従来どおり)。
public class TrailFollower : MonoScript {

    [SerializeField] private string leaderName_ = "Player";    // 追従するリーダー名
    [SerializeField] private string chainName_ = "Default";   // 同じ TrailLeader 上で独立した隊列を分ける名前
    [SerializeField] private bool rotateToFace_ = true;        // 進行方向を向くかどうか
    [SerializeField] private int order_ = 1;           // 隊列の順番（1が先頭）
    [SerializeField] private float leadMinOffset_ = 4.0f;        // リーダー → 先頭ノード の最小距離
    [SerializeField] private float leadMaxOffset_ = 8.0f;        // リーダー → 先頭ノード の最大距離
    [SerializeField] private float unitMinOffset_ = 4.0f;        // ノード間（前ノード → 自分）の最小距離
    [SerializeField] private float unitMaxOffset_ = 8.0f;        // ノード間（前ノード → 自分）の最大距離
    [SerializeField] private float maxBendRad_ = Mathf.PI;    // 基準向きから許す曲げ角(rad)。π(≈3.14159)で無制限
    [SerializeField] private float smoothTime_ = 0.08f;       // 追従の滑らかさ（0で即時）
    [SerializeField] private float maxSmoothSpeed_ = 100000.0f;   // 追従速度の上限

    private Vector3 smoothVel_ = Vector3.zero;
    private bool registered_ = false;

    public string ChainName { get { return chainName_; } }
    public int Order { get { return order_; } }

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

    // TrailLeader から order 順に呼ばれる。前ノードの「更新後の現在位置」prev と
    // 基準向き refDir(前セグメントの向き／先頭はリーダー後方、いずれも単位ベクトル)を受け取り、
    // 距離と曲げ角を拘束した点へスムーズに寄せ、更新後の自分の位置を返す(次ノードの prev になる)。
    public Vector3 Solve(Vector3 prev, Vector3 refDir, bool isFront, float dt) {
        // 先頭(生存ノードの最前)はリーダーに、それ以外は前ノードに対するレンジを使う。
        float minD = isFront ? leadMinOffset_ : unitMinOffset_;
        float maxD = isFront ? leadMaxOffset_ : unitMaxOffset_;
        if (maxD < minD) {
            maxD = minD; // min/max 逆転の設定ミスは max=min に丸める
        }
        Vector3 cur = transform.position;

        // 前ノード → 自分 の方向。重なって長さが出ないフレームは基準向き→真下にフォールバック
        // （ゼロ方向だと target が prev に潰れて重なるのを防ぐ）。
        Vector3 dir = cur - prev;
        float dl = dir.Length();
        Vector3 unit;
        if (dl > kEps) {
            unit = dir * (1.0f / dl);
        } else if (refDir.x * refDir.x + refDir.y * refDir.y > kEps * kEps) {
            unit = refDir;
        } else {
            unit = new Vector3(0.0f, -1.0f, 0.0f);
        }

        // 曲げ角拘束：unit を基準向き refDir から ±maxBendRad_ 以内へクランプ（2D・Z軸回り）。
        // refDir が無効(長さ0)なら拘束しない。maxBendRad_=π は実質無制限。
        if (maxBendRad_ < Mathf.PI - kEps &&
            refDir.x * refDir.x + refDir.y * refDir.y > kEps * kEps) {
            float dot = refDir.x * unit.x + refDir.y * unit.y;
            float cross = refDir.x * unit.y - refDir.y * unit.x;
            float ang = Mathf.Atan2(cross, dot); // refDir→unit の符号付き角 [-π,π]
            if (ang > maxBendRad_ || ang < -maxBendRad_) {
                float clamped = (ang > 0.0f) ? maxBendRad_ : -maxBendRad_;
                float c = Mathf.Cos(clamped);
                float s = Mathf.Sin(clamped);
                // refDir を clamped だけ回した向きを採用
                unit = new Vector3(refDir.x * c - refDir.y * s,
                                   refDir.x * s + refDir.y * c, 0.0f);
            }
        }

        // レッシュ拘束：範囲内(min〜max)は現在距離を保つ＝拘束しない。範囲外だけ境界へ引き戻す。
        float targetDist = (dl < minD) ? minD : ((dl > maxD) ? maxD : dl);
        Vector3 target = prev + unit * targetDist;

        Vector3 pos = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
            cur, target, ref smoothVel_, smoothTime_, dt, maxSmoothSpeed_);
        transform.position = pos;

        // 進行方向を向く（2D・Z軸回り）。ほぼ静止しているフレームは今の向きを保持する。
        if (rotateToFace_) {
            Vector3 move = pos - cur;
            if (move.x * move.x + move.y * move.y > FaceEpsilonSq) {
                float roll = Mathf.Atan2(move.x, move.y);
                transform.rotate = Quaternion.MakeFromAxis(Vector3.back, roll);
            }
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
