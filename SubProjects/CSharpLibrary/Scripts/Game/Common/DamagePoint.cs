// 速度→ダメージ の折れ線グラフ上の1点。speed のとき damage を与える。
// 速度とダメージを束ねて1要素にすることで、別々のListのような要素数ズレを防ぐ。
public struct DamagePoint {
    public DamagePoint(float speed, float damage) {
        this.speed = speed;
        this.damage = damage;
    }

    [SerializeField] public float speed;  // この速度のとき
    [SerializeField] public float damage;  // このダメージを与える
}
