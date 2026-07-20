using System;

// プレイヤー（指定名の Entity）を追従するカメラ。カメラ用 Entity に付ける。
// シーン内の対象 Entity は ecsGroup.FindEntity(名前) で取得する。
public class PlayerFollowCamera : MonoScript {

    [SerializeField] private string  targetName_     = "Player";
    [SerializeField] private Vector3 offset_         = new Vector3(0.0f, 0.0f, -144.3f); // 対象からの相対位置
    [SerializeField] private float   smoothTime_     = 0.15f;        // 追従の滑らかさ（0で即時）
    [SerializeField] private float   maxSmoothSpeed_ = 100000.0f;    // 追従速度の上限（実質無制限）

    private Entity target_;
    private Vector3 smoothVel_ = Vector3.zero;

    // シェイクを含まない、追従だけした位置。shake_.Offset を混ぜた値を from に使うと
    // 揺れが追従の速度計算に混入して発振するため、素の追従位置は別で保持する。
    private Vector3 basePosition_ = Vector3.zero;
    private CameraShake shake_;

    public override void Initialize() {
        // シーン内の対象 Entity を名前で取得
        target_ = ecsGroup.FindEntity(targetName_);
        // 同じ Entity にアタッチされている CameraShake があれば連携する(無くても動作する)
        shake_ = entity.GetScript<CameraShake>();
        basePosition_ = transform.position;
    }

    public override void Update() {
        if (target_ == null) {
            return;
        }

        Transform targetTransform = target_.GetComponent<Transform>();
        if (targetTransform == null) {
            return;
        }

        // 対象の位置 ＋ オフセットへ滑らかに追従
        Vector3 desired = targetTransform.position + offset_;
        basePosition_ = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
            basePosition_, desired, ref smoothVel_, smoothTime_, Time.deltaTime, maxSmoothSpeed_);

        // 追従位置にシェイクのオフセットを上乗せして最終的な位置にする
        transform.position = shake_ != null ? basePosition_ + shake_.Offset : basePosition_;
    }
}
