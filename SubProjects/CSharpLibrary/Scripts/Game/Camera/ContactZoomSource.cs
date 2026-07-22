using System;

// 重なっている相手が持つ ZoomWeight の合計を強さとして返す寄与。
// コライダを持つカメラ Entity に付ける。
//
// 「重なっている間だけ」なので、相手が離れた・破棄された次のフレームには
// その分が自動で消える。蓄積はしないので Clear() のような後始末も要らない。
public class ContactZoomSource : CameraZoomSource {

    private float pending_;  // 衝突コールバックで積んでいる途中の値
    private float strength_; // 確定した合計。Strength() が返す

    public override float Strength() {
        return strength_;
    }

    protected override void UpdateStrength() {
        // 衝突コールバックは Update とは別フェーズで走るため、
        // 直前フェーズで積まれた分をここで確定し、次の集計用に空にする。
        // 実際に使われるのは 1 フレーム前の重なり具合だが、
        // 基底の Lerp が吸収するので見た目には出ない。
        strength_ = pending_;
        pending_ = 0.0f;
    }

    // Enter と Stay は同じペアで同じフレームには来ない（初回が Enter、以降が Stay）ため、
    // 両方で積んでも二重計上にはならない。Enter だけだと最初の 1 フレームが抜ける。
    public override void OnCollisionEnter(Entity collision) {
        Accumulate(collision);
    }

    public override void OnCollisionStay(Entity collision) {
        Accumulate(collision);
    }

    private void Accumulate(Entity collision) {
        if (collision == null || collision.Id == entity.Id) {
            return; // 自分自身は無視
        }

        ZoomWeight weight = collision.GetScript<ZoomWeight>();
        if (weight == null) {
            return; // 札を持たない相手は寄与しない
        }

        pending_ = Mathf.Clamp01(pending_ + weight.Weight);
    }
}
