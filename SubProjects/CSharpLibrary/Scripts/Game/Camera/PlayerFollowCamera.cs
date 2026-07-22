using System;

// プレイヤー（指定名の Entity）を追従するカメラ。カメラ用 Entity に付ける。
// シーン内の対象 Entity は ecsGroup.FindEntity(名前) で取得する。
// 揺れは CameraShake / ShakeSource が担当する。
public class PlayerFollowCamera : MonoScript {

    [SerializeField] private string  targetName_     = "Player";
    [SerializeField] private Vector3 offset_         = new Vector3(0.0f, 0.0f, -144.3f); // 対象からの相対位置
    [SerializeField] private float   smoothTime_     = 0.15f;        // 追従の滑らかさ（0で即時）
    [SerializeField] private float   maxSmoothSpeed_ = 100000.0f;    // 追従速度の上限（実質無制限）

    private Entity target_;
    private Vector3 smoothVel_ = Vector3.zero;

    public override void Initialize() {
        // シーン内の対象 Entity を名前で取得
        target_ = ecsGroup.FindEntity(targetName_);
    }

    public override void Update() {
        if (target_ == null) {
            return;
        }

        Transform targetTransform = target_.GetComponent<Transform>();
        if (targetTransform == null) {
            return;
        }

        Vector3 desired = targetTransform.position + offset_;
        transform.position = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
            transform.position, desired, ref smoothVel_, smoothTime_, Time.deltaTime, maxSmoothSpeed_);
    }
}
