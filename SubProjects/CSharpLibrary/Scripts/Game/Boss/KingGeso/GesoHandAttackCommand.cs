
//=============================
// 攻撃時の触手の回転モード
//=============================
internal enum GesoHandRotationMode
{
    FaceAttackDirection,
    MatchTargetRotation,
}

//=============================
// 攻撃コマンドクラス
//=============================
internal sealed class GesoHandAttackCommand
{
    public Entity target;
    public float damage;
    public float attackDuration;
    public float moveDuration;
    public float passThroughDistance;
    public GesoHandRotationMode rotationMode;
}
