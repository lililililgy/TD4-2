using System;

// カメラエンティティの transform.scale から、画面に対応する world 座標系の
// half-extent（半サイズ）とズーム倍率を算出するユーティリティ。
// MonoScript ではなく素の C# クラス。エンティティ配置や .entity への追記を避けるため、
// 利用側（MonoScript）の Initialize() でインスタンス化し、保持してもらう想定。
public class ScreenBounds {
    // 基準スケール(referenceScale_)のときの画面 world half-extent
    private readonly Vector2 baseHalfExtent_;
    // ZoomRatio() が 1.0 になる基準の camera.transform.scale
    private readonly float referenceScale_;

    // ズーム追従対象のカメラ Entity。見つからない場合は null のままフォールバック値を返す。
    private Entity cameraEntity_;

    public ScreenBounds(ECSGroup ecsGroup, string cameraName, Vector2 baseHalfExtent, float referenceScale) {
        baseHalfExtent_ = baseHalfExtent;
        referenceScale_ = referenceScale;
        cameraEntity_ = ecsGroup?.FindEntity(cameraName);
    }

    public ScreenBounds(ECSGroup ecsGroup, string cameraName)
        : this(ecsGroup, cameraName, new Vector2(960.0f, 540.0f), 1.78699f) {
    }

    // 現在のカメラズームに応じた画面の world half-extent。
    // カメラが見つからない場合は baseHalfExtent * referenceScale を返す（安全側フォールバック）。
    public Vector2 HalfExtent() {
        if (cameraEntity_ == null || cameraEntity_.transform == null) {
            return baseHalfExtent_ * referenceScale_;
        }

        Vector3 scale = cameraEntity_.transform.scale;
        return new Vector2(baseHalfExtent_.x * scale.x, baseHalfExtent_.y * scale.y);
    }

    // 基準スケールに対する現在のズーム倍率。カメラが見つからない場合は 1.0（無補正）を返す。
    public float ZoomRatio() {
        if (cameraEntity_ == null || cameraEntity_.transform == null || referenceScale_ == 0.0f) {
            return 1.0f;
        }

        return cameraEntity_.transform.scale.x / referenceScale_;
    }
}
