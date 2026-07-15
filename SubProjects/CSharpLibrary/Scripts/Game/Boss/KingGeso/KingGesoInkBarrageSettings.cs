public class KingGesoInkBarrageSettings : MonoScript
{
    [SerializeField] public string bulletPrefabName = "KingGesoInkBullet"; //プレハブの名前
    [SerializeField] public float selectionWeight = 1.0f;         //攻撃選択時の重み
    [SerializeField] public int bulletCountPerWave = 10;          //1回の波で発射する弾の数
    [SerializeField] public int waveCount = 7;                    //発射する波の数
    [SerializeField] public int poolSize = 120;                   //0なら1攻撃分の弾数を自動確保
    [SerializeField] public float waveInterval = 0.25f;           //波を発射する間隔
    [SerializeField] public float angleOffset = 0.14f;            //弾の発射角度のオフセット
    [SerializeField] public bool reverseHalfway = true;           //途中で回転方向を反転するかどうか
    [SerializeField] public float bulletSpeed = 220.0f;           //弾の速度
    [SerializeField] public float bulletLifeTime = 7.0f;          //弾の寿命
    [SerializeField] public float bulletDamage = 8.0f;            //弾のダメージ
    [SerializeField] public float recoveryDuration = 0.6f;        //回復時間
    [SerializeField] public Vector2 spawnOffset = Vector2.zero;   //弾の生成位置のオフセット
}
