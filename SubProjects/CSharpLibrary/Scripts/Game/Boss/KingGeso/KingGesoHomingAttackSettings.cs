public class KingGesoHomingAttackSettings : MonoScript
{
    [SerializeField] public string projectilePrefabName = "KingGesoHomingBullet"; //プレハブの名前
    [SerializeField] public float selectionWeight = 1.0f;         //攻撃選択時の重み
    [SerializeField] public float tellDuration = 0.8f;            //攻撃前の予備動作時間
    [SerializeField] public int projectileCount = 3;              //発射する弾の数
    [SerializeField] public float launchInterval = 1.0f;         //弾を発射する間隔
    [SerializeField] public float projectileSpeed = 200.0f;       //弾の速度
    [SerializeField] public float turnSpeed = 2.0f;               //弾の回転速度
    [SerializeField] public float projectileLifeTime = 8.0f;      //弾の寿命
    [SerializeField] public float projectileDamage = 1.0f;        //弾のダメージ
    [SerializeField] public Vector2 spawnOffset = Vector2.zero;   //弾の生成位置のオフセット
    [SerializeField] public string swimParticlePrefabName = "enemySwimParticle";
    [SerializeField] public int swimParticleEmitCount = 1;
    [SerializeField] public float swimParticleEmitInterval = 0.15f;
    [SerializeField] public string launchSePath = "./Assets/Sounds/se/boss/KingGeso_homingBullet.mp3";
    [SerializeField] public float launchSeVolume = 1.0f;
    [SerializeField] public float launchSePitch = 1.0f;
}
