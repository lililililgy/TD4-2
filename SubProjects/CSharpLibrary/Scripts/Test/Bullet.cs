using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class Bullet : MonoScript {
    public override void Initialize() {
        lifeTimer_ = 0.0f;
    }

    public override void Update() {
        // life管理
        lifeTimer_ += Time.deltaTime;
        isDead_ |= lifeTimer_ >= lifeTime_;

        if (isDead_) {
            entity.Destroy();
            return;
        }

        // 移動
        Transform transform = entity.GetComponent<Transform>();
        transform.position += moveDir_ * speed_ * Time.deltaTime;
    }

    void SetMoveDir(Vector3 dir) {
        moveDir_ = dir.Normalized();
    }

    public override void OnCollisionEnter(Entity collision) {
        isDead_ = true;
    }

    private bool isDead_ = false;

    private Vector3 moveDir_ = new Vector3(0.0f, 0.0f, -1.0f);
    [SerializeField] public float speed_ = 10.0f;
    [SerializeField] public float lifeTime_ = 5.0f;
    private float lifeTimer_ = 0.0f;
}

