// 速度→ダメージ の折れ線グラフ上の1点。speed のとき damage を与える。
// 速度とダメージを束ねて1要素にすることで、別々のListのような要素数ズレを防ぐ。
//
// ※ class にしているのは意図的。エンジンの SerializeField 復元(Variables.cpp / Mono)が
//    「List<参照型>」かつ「引数なしコンストラクタ」を前提にしているため、struct にすると
//    リスト復元時に .ctor() が見つからず実行時クラッシュする。引数なし ctor は必須。
public class DamagePoint {
    public DamagePoint() { }

    public DamagePoint(float speed, float damage) {
        this.speed = speed;
        this.damage = damage;
    }

    [SerializeField] public float speed;  // この速度のとき
    [SerializeField] public float damage;  // このダメージを与える
}
