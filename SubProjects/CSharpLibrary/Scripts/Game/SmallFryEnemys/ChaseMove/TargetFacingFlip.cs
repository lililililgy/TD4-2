using System;

// 指定した方向へ向きを合わせる共通コンポーネント。

public class TargetFacingFlip : MonoScript
{
    [SerializeField] private float turnLerp = 0.3f;
    // baseDirからどこまで傾けて良いか。90°だとSpikeFishと同じ挙動。
    [SerializeField] private float maxTiltAngleDeg = 90.0f;

    private SpriteRenderer spriteRenderer_;

    // 反転後の絵の正面方向移動方向の計算などに使う。
    private Vector3 baseDir_ = Vector3.right;
    private bool isRight_ = true;
    public Vector3 FacingDirection => Matrix4x4.Transform(baseDir_, Matrix4x4.Rotate(transform.rotate));

    public override void Initialize()
    {
        spriteRenderer_ = entity.GetComponent<SpriteRenderer>();
    }

    // 指定した方向へ向きを合わせる
 
    public void FaceDirection(Vector3 dir, bool immediate = false)
    {
        if (dir.LengthSq() <= 0.0001f) { return; }
        dir = dir.Normalized();

        // 向く方向が右か左かで、UVを反転するかを決める
        bool isRight = Vector3.Dot(Vector3.right, dir) >= 0.0f;
        if (isRight != isRight_)
        {
        
            transform.rotate = transform.rotate * Quaternion.MakeFromAxis(Vector3.forward, Mathf.PI);
        }
        isRight_ = isRight;
        baseDir_ = isRight ? Vector3.right : -Vector3.right;

        // 元絵は左向きなので、右を向かせる時だけUVを反転する
        UpdateFlip(isRight);

        // baseDir から見た方向の符号付き角度
        float angle = Mathf.Atan2(dir.y, dir.x) - Mathf.Atan2(baseDir_.y, baseDir_.x);
        angle = WrapPi(angle);

        // ±maxTiltAngleDeg にクランプする(これ以上はUV反転側で表現するため)
        float maxTiltRad = maxTiltAngleDeg * Mathf.PI / 180.0f;
        float clampedAngle = Mathf.Clamp(angle, -maxTiltRad, maxTiltRad);

        Quaternion targetRotate = Quaternion.MakeFromAxis(Vector3.forward, clampedAngle);
        transform.rotate = immediate ? targetRotate : Quaternion.Slerp(transform.rotate, targetRotate, turnLerp);
    }

    // 左右専用の絵にUVを切り替える
    private void UpdateFlip(bool isRight)
    {
        if (spriteRenderer_ == null) { return; }

        UVTransform uv = spriteRenderer_.uvTransform;

        // 既に反転済みの値であっても崩れないよう、一旦「反転前(正のスケール)」の基準値に戻す
        float baseScaleX = Mathf.Abs(uv.scale.x);
        float basePositionX = uv.scale.x >= 0.0f ? uv.position.x : uv.position.x - baseScaleX;

        uv.scale.x = isRight ? -baseScaleX : baseScaleX;
        uv.position.x = isRight ? basePositionX + baseScaleX : basePositionX;

        spriteRenderer_.uvTransform = uv;
    }

    // 角度差を (-PI, PI] に正規化
    private static float WrapPi(float angle)
    {
        float twoPi = 2.0f * Mathf.PI;
        while (angle > Mathf.PI) angle -= twoPi;
        while (angle < -Mathf.PI) angle += twoPi;
        return angle;
    }
}
