// HitStop（減速の状態）の時間を進める担当。寿命の長い Entity（GameController など）にアタッチする。
// この担当が居ないシーンでは減速の要求そのものが捨てられる（HitStop.Request 参照）ので、
// 演出が出ないだけで、減速したまま固まる事故にはならない。
//
// 現状 GameController と Player の両方に付けてある。GameController を置いていない
// 検証用シーンでもプレイヤーの演出が動くようにするためで、重複しても下記の通り害はない。
//
// 減速を要求する側（攻撃がヒットした弾、被弾したプレイヤー）は途中で破棄されうる。
// 要求元が自分でタイマーを持つと、破棄された瞬間に解除が走らず減速したまま固まるため、
// 「進める役」を要求元から切り離してここに置く。
//
// 二重にアタッチされても時間が倍速で進まないよう、先に名乗り出た1つだけが進める（残りは待機）。
// 進めている側が破棄されたら、次のフレームに待機側が引き継ぐ。
public class HitStopSystem : MonoScript {

    // 実際に時間を進めているインスタンス。null なら空席。
    private static HitStopSystem active_;

    public override void Initialize() {
        if (active_ == null) {
            active_ = this;
            HitStop.SetDriverPresent(true);
        }
    }

    public override void Update() {
        // 空席（前任が破棄された）なら引き継ぐ
        if (active_ == null) {
            active_ = this;
            HitStop.SetDriverPresent(true);
        }
        if (active_ != this) {
            return; // 待機側は何もしない
        }

        // timeScale に依存しない実時間の刻みで測る
        HitStop.Tick(Time.unscaledDeltaTime);
    }

    public override void OnDestroy() {
        if (active_ != this) {
            return;
        }
        active_ = null;
        // 減速中に破棄されるとゲーム全体が減速したまま残る。担当を降りるときは必ず等速へ戻す。
        HitStop.SetDriverPresent(false);
    }
}
