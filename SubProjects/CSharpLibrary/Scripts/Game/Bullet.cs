using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class Bullet : MonoScript
{
    public override void Initialize()
    {
        lifeTimer_ = 0.0f;
    }

    public override void Update()
    {
        // life管理
        lifeTimer_ += Time.deltaTime;

        // 移動
        Transform transform = entity.GetComponent<Transform>();
        transform.position += moveDir_ * speed_ * Time.deltaTime;

        // 進行方向を向く（2D・Z軸回り）。ほぼ静止しているフレームは今の向きを保持する。
        float roll = Mathf.Atan2(moveDir_.x, moveDir_.y);
        transform.rotate = Quaternion.MakeFromAxis(Vector3.back, roll);
    }

    public void SetMoveDir(Vector3 dir)
    {
        moveDir_ = dir.Normalized();
    }

    private Vector3 moveDir_ = new Vector3(0.0f, 0.0f, 1.0f);
    [SerializeField] public float speed_ = 10.0f;
    [SerializeField] public float lifeTime_ = 5.0f;
    private float lifeTimer_ = 0.0f;
}

