public class MorayEel : MonoScript {
    /* ----- パラメータ ----- */


    private HP hp_;

    public override void Initialize() {
        // HPスクリプト取得
        hp_ = entity.GetScript<HP>();
    }

    public override void Update() {

        // 死んだら更新スキップ
        if (hp_ != null && hp_.currentHp <= 0) {
            return;
        }
    }

    private void FireMorayEel() {

    }
}
