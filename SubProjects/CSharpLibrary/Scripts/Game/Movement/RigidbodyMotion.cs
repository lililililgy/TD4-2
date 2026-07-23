using System;

// Rigidbody2D 経由の移動ヘルパー。移動スクリプトが1つ保持し、毎フレーム Apply を呼ぶ。
//
// 各スクリプトは従来どおり「進みたい速度(desired)」を自前で計算するだけでよい。
// エンジン(C++)は衝突時に Rigidbody2D.velocity を書き換えて C# へ逆同期するので、
// 前フレームに書き出した値との差分を「外力(衝突の撃力)」として取り出し、
// desired に足し込んだ上で指数減衰させる。これで
//   ・衝突が無い間は従来どおり desired そのままの挙動
//   ・ぶつかった瞬間だけ弾かれ、externalDamp_ の時定数で収まる
// となる。position の積分はエンジンが行わないため、ここで行う。
//
// 呼び出し側は Apply の結果を自分の velocity_ に書き戻してはいけない。
// 書き戻すと外力が次フレームの desired に混ざり、二重に積み上がる。
public class RigidbodyMotion {

    // externalDamp: 衝突で受けた外力の減衰の強さ(1/秒)。大きいほど早く収まる。
    public RigidbodyMotion(float externalDamp) {
        externalDamp_ = externalDamp > 0.0001f ? externalDamp : 0.0001f;
    }

    public void Attach(Entity entity) {
        if (entity == null) { return; }
        rigidbody_ = entity.GetComponent<Rigidbody2D>();
    }

    // desired: スクリプトが決めた「進みたい速度」。
    // 外力を足した実速度で transform.position を積分し、Rigidbody2D へ書き出す。
    public void Apply(Transform transform, Vector3 desired) {
        if (rigidbody_ != null) {
            // エンジンが書き換えた分だけを外力として拾う
            Vector2 now = rigidbody_.velocity;
            external_ += new Vector3(now.x - lastWritten_.x, now.y - lastWritten_.y, 0.0f);
        }

        // 外力は指数減衰で収める
        external_ *= (float)Math.Exp(-externalDamp_ * Time.deltaTime);
        if (external_.LengthSq() < 0.0001f) {
            external_ = Vector3.zero;
        }

        velocity_ = desired + external_;

        if (transform != null) {
            transform.position += velocity_ * Time.deltaTime;
        }

        // setter が native を叩くので、フレームの最後に1回だけ書く
        if (rigidbody_ != null) {
            lastWritten_ = new Vector2(velocity_.x, velocity_.y);
            rigidbody_.velocity = lastWritten_;
        }
    }

    // 外力込みの実速度(直近の Apply で確定した値)
    public Vector3 Velocity { get { return velocity_; } }

    private Rigidbody2D rigidbody_;
    private Vector3 velocity_ = Vector3.zero;
    private Vector3 external_ = Vector3.zero;
    private Vector2 lastWritten_ = new Vector2(0.0f, 0.0f);

    private readonly float externalDamp_;
}
