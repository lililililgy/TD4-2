using System;

// 単一のリーダーを、自分の order_ に応じた距離だけ後ろで追従する追従コンポーネント。
// プレイヤー(頭)に対する胴体・触手や、母体に対する卵など、1人のリーダーを共有する隊列に使う。
//
// 各ノードは「自分の order_」を自分で持つ。ノード同士をポインタで連結しないので、
// 途中のノードが消えても他のノードはリーダーだけを見て追従し続ける（連結が壊れない）。
// 動的な隊列（卵など）は管理側が SetOrder() で順番を差し込む。
//
// 追従距離 = leadOffset_ + unitOffset_ * (order_ - 1)
//   leadOffset_ … リーダー → 先頭ノード の距離
//   unitOffset_ … ノード間の距離（リーダーとノードで別々の距離を指定できる）
// 経路履歴は持たず、毎フレーム「自分とリーダーの現在位置」だけから目標を計算する(FTL)。
public class TrailFollower : MonoScript {

    [SerializeField] private string leaderName_     = "Player";  // 追従するリーダー名
    [SerializeField] private int    order_          = 1;         // 隊列の順番（1が先頭）
    [SerializeField] private float  leadOffset_     = 8.0f;      // リーダー → 先頭 の距離
    [SerializeField] private float  unitOffset_     = 8.0f;      // ノード間の距離
    [SerializeField] private float  smoothTime_     = 0.08f;     // 追従の滑らかさ（0で即時）
    [SerializeField] private float  maxSmoothSpeed_ = 100000.0f; // 追従速度の上限

    private Entity  leader_;
    private Vector3 smoothVel_ = Vector3.zero;

    // 隊列の順番。動的な隊列（卵など）は外部マネージャが SetOrder() で push する。
    public int  Order { get { return order_; } }
    public void SetOrder(int order) { order_ = order; }

    public override void Initialize() {
        ResolveLeader();
    }

    public override void Update() {
        // リーダー未解決なら遅延解決（生成順や再生成に強くする）。
        if (leader_ == null) {
            ResolveLeader();
            if (leader_ == null) {
                return;
            }
        }

        Transform lt = leader_.GetComponent<Transform>();
        if (lt == null) {
            return;
        }

        // リーダーの「後方」へ distance だけ離れた点を目標にする（リーダー基準で決定的に決まる）。
        // 自分の現在位置に依存しないので、重なっていても必ず distance ぶん後ろへ出るし、
        // distance を変えれば素直に間隔が変わる。order ごとに距離が違うので重ならず順番も保たれる。
        float   distance = leadOffset_ + unitOffset_ * (order_ - 1);
        Vector3 prev     = transform.position;

        // 後ろ向き。リーダーの向き(2D・+Yが前)から取るが、向きが未確定(ゼロ回転など)で
        // 長さが出ないときは、いま自分が居る方向→なければ真下、にフォールバックする。
        // こうしないと behind がゼロになり target がリーダー位置に潰れて重なる（distance も効かない）。
        Vector3 behind = -Quaternion.RotateVector(lt.rotate, Vector3.up);
        float   bl     = behind.Length();
        if (bl > kEps) {
            behind = behind * (1.0f / bl);
        } else {
            Vector3 fromLeader = prev - lt.position;
            float   fl         = fromLeader.Length();
            behind = (fl > kEps) ? fromLeader * (1.0f / fl) : new Vector3(0.0f, -1.0f, 0.0f);
        }
        Vector3 target = lt.position + behind * distance;

        Vector3 pos = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
            prev, target, ref smoothVel_, smoothTime_, Time.deltaTime, maxSmoothSpeed_);
        transform.position = pos;

        // 進行方向を向く（2D・Z軸回り）。ほぼ静止しているフレームは今の向きを保持する。
        Vector3 move = pos - prev;
        if (move.x * move.x + move.y * move.y > FaceEpsilonSq) {
            float roll = Mathf.Atan2(move.x, move.y);
            transform.rotate = Quaternion.MakeFromAxis(Vector3.back, roll);
        }
    }

    private void ResolveLeader() {
        leader_ = ecsGroup.FindEntity(leaderName_);
    }

    // ベクトル長のゼロ割り回避しきい値
    private const float kEps = 1e-4f;

    // この距離(二乗)未満しか進んでいないフレームは回転を更新しない（向きのちらつき防止）
    private const float FaceEpsilonSq = 0.0001f;
}
