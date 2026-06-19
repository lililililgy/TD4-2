using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


public class PlayerMoveComponent
    : MonoScript {

    public override void Initialize() { }

    public override void Update() {
        Transform transform = entity.GetComponent<Transform>();
        PlayerInputComponent inputComp = entity.GetScript<PlayerInputComponent>();

        Vector3 moveDir;
        // 減衰
        if (inputComp.moveDir_.Length() == 0) {
            velocity_ = SpringDamper.SmoothDamp<float, FloatDampTraits>(
                velocity_, 0.0f, ref currentDecel_, smoothTime_, Time.deltaTime, maxSmoothSpeed_);

            moveDir = Quaternion.RotateVector(transform.rotate, Vector3.forward);

        } else {
            // 加速
            velocity_ += accel_ * Time.deltaTime;
            if (velocity_ > maxSpeed_) {
                velocity_ = maxSpeed_;
            }

            moveDir = new Vector3(inputComp.moveDir_.x, inputComp.moveDir_.y, 0.0f);
        }
        
        moveDir = moveDir.Normalized();

        transform.rotate = Quaternion.LookAt(Vector3.zero, moveDir, Vector3.up);
        transform.position += moveDir * velocity_ * Time.deltaTime;
    }

    [SerializeField] private float accel_ = 5.0f;
    [SerializeField] private float velocity_ = 0.0f;

    private float currentDecel_ = 0.0f;
    [SerializeField] private float smoothTime_ = 0.1f;
    [SerializeField] private float maxSmoothSpeed_ = 1000.0f;

    [SerializeField] private float maxSpeed_ = 36.0f;


}
