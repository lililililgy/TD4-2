using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;


public class PlayerMoveComponent
    : MonoScript {

    public override void Initialize() { }

    public override void Update() {
        Transform transform = entity.GetComponent<Transform>();
        PlayerInputComponent inputComp = entity.GetScript<PlayerInputComponent>();

        float cullRoll = 0f;
        float prevRoll = 0f;
        // 減衰
        if (inputComp.moveDir_.Length() <= 0f) {
            prevRoll = Mathf.Atan2(velocity_.x, velocity_.y);

            const float kThresholdSpeed = 0.1f;

            if (velocity_.Length() < kThresholdSpeed) {
                velocity_ = Vector3.zero;
                return;
            }

            velocity_ = SpringDamper.SmoothDamp<Vector3, Vector3DampTraits>(
                velocity_, Vector3.zero, ref currentDecelSmoothSpeed_, decelSmoothTime_, Time.deltaTime, decelMaxSmoothSpeed_);

            cullRoll = Mathf.Atan2(velocity_.x, velocity_.y);

        } else {
            Vector3 moveDir = new Vector3(inputComp.moveDir_.x, inputComp.moveDir_.y, 0.0f);

            cullRoll += Mathf.Atan2(moveDir.x, moveDir.y);
            prevRoll = Mathf.Atan2(velocity_.x, velocity_.y);

            // 加速
            velocity_ += moveDir * (accel_ * Time.deltaTime);
            if (velocity_.Length() > maxSpeed_) {
                velocity_ = velocity_.Normalized() * maxSpeed_;
            }
        }


        transform.rotate = Quaternion.MakeFromAxis(Vector3.back, SpringDamper.SmoothDamp<float, FloatDampTraits>(
            cullRoll, prevRoll, ref currentRotateSmoothSpeed_, currentRotateSmoothTime_, Time.deltaTime, currentRotateMaxSmoothSpeed_));

        transform.position += velocity_ * Time.deltaTime;
    }

    private float currentRotateSmoothSpeed_ = 0.0f;
    [SerializeField] private float currentRotateSmoothTime_ = 0.0f;
    [SerializeField] private float currentRotateMaxSmoothSpeed_ = 0.0f;


    [SerializeField] private float accel_ = 5.0f;
    [SerializeField] private Vector3 velocity_ = new Vector3(0.0f, 0.0f, 0.0f);

    private Vector3 currentDecelSmoothSpeed_ = Vector3.zero;
    [SerializeField] private float decelSmoothTime_ = 0.1f;
    [SerializeField] private float decelMaxSmoothSpeed_ = 1000.0f;

    [SerializeField] private float maxSpeed_ = 36.0f;


}
