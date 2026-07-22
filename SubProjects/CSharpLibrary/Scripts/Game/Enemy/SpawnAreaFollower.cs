using System;

// EnemySpawnSystem の「敵の生存維持範囲」用コライダー(BoxCollider2D, useOwnerScale)を、
// カメラズームに合わせて毎フレーム追従させる。EnemySpawnSystem 側の spawnAreaScale_ と
// 同じ係数を使うことで、スポーン範囲とデスポーン抑制範囲を一致させる。
//
// 縮める方向には追従させない（これまでに到達した最大値を保持する）。
// 寄った瞬間に範囲外になった敵が DespawnTimer で消え、引き戻すと画面が空になる事故を防ぐため。
public class SpawnAreaFollower : MonoScript {
    [SerializeField] private string cameraName_ = "MainCamera";
    [SerializeField] private float areaScale_ = 1.30f;
    // アタッチ先の BoxCollider2D の size。useOwnerScale により size × owner scale が実効の全幅になる。
    [SerializeField] private float colliderBaseSize_ = 1024.0f;

    private ScreenBounds screenBounds_;

    public override void Initialize() {
        screenBounds_ = new ScreenBounds(ecsGroup, cameraName_);
    }

    public override void Update() {
        if (screenBounds_ == null) return;

        // size(1024) × owner scale = 範囲の全幅。half は half-extent なので 2 倍して全幅に揃える。
        // 検算: half=(1715.5, 964.97), areaScale_=1.30 → scale=(4.356, 2.450)。
        // シーン実データの scale=(4.3699, 2.4508) と一致する。
        Vector2 half = screenBounds_.HalfExtent();
        float targetScaleX = half.x * 2.0f * areaScale_ / colliderBaseSize_;
        float targetScaleY = half.y * 2.0f * areaScale_ / colliderBaseSize_;

        Vector3 scale = transform.scale;
        scale.x = Math.Max(scale.x, targetScaleX);
        scale.y = Math.Max(scale.y, targetScaleY);
        transform.scale = scale;
    }
}
