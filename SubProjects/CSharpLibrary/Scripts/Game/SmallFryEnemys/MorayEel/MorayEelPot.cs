using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


public class MorayEelPot : MonoScript {
    /* ----- パラメータ ----- */
    [SerializeField] private float firePower = 0.0f;     
    [SerializeField] private float rotateZSpeed = 0.0f;  
    [SerializeField] private float fireInterval = 0.0f;  
    [SerializeField] private string morayEelPrefabName = "MorayEel"; 

    /* ----- 実行時状態 ----- */
    private float fireTimer_ = 0.0f; // 発射間隔の計測
    private float angleZ_ = 0.0f;    // 現在のZ回転角

    public override void Initialize() {
        fireTimer_ = 0.0f;
        angleZ_ = 0.0f;
    }

    public override void Update() {
        // ツボを Z 軸回転させる
        angleZ_ += rotateZSpeed * Time.deltaTime;
        if (transform) {
            transform.rotate = Quaternion.FromEuler(new Vector3(0.0f, 0.0f, angleZ_));
        }

        // 一定間隔ごとに発射
        if (fireInterval <= 0.0f) {
            return;
        }

        fireTimer_ += Time.deltaTime;
        if (fireTimer_ >= fireInterval) {
            fireTimer_ -= fireInterval;
            FireMorayEel();
        }
    }

    private void FireMorayEel() {
        Entity eel = ecsGroup.CreateEntity(morayEelPrefabName);
        if (!eel) {
            return;
        }

        // ツボと同じ位置・向きから発射する
        eel.transform.position = transform.position;
        eel.transform.rotate = transform.rotate;

        // 発射方向
        Vector3 fireDir = Matrix4x4.Transform(Vector3.up, Matrix4x4.Rotate(transform.rotate));
        Vector3 initialVelocity = fireDir * firePower;

        // 生成したウツボの移動スクリプトに初速を渡し、発射(減衰)フェーズを開始させる
        MorayEelMovement movement = eel.GetScript<MorayEelMovement>();
        if (movement) {
            movement.Launch(initialVelocity);
        }
    }
}
