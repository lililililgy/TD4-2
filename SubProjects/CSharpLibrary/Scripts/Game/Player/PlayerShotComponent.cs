using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class PlayerShotComponent : MonoScript {

    public override void Update() {

        // Update the shoot cooldown timer
        shootCooldown_ -= Time.deltaTime;
        shootCooldown_ = Math.Max(shootCooldown_, 0.0f);

        PlayerInputComponent inputComponent = entity.GetScript<PlayerInputComponent>();
        if (!inputComponent.IsShootButtonPressed) {
            return;
        }

        // 発射不可状態の検知 (プレイヤーのステートによるものや残弾数が0の場合など)
        PlayerStateComponent stateComponent = entity.GetScript<PlayerStateComponent>();
        Magazine magazine = entity.GetScript<Magazine>();
        if (!stateComponent.CanShoot() || magazine.IsEmpty) {
            return;
        }

        if (shootCooldown_ <= 0.0f) {
            // Entity の 作成
            Entity bulletEnt = ecsGroup.CreateEntity("Bullet");

            Transform bullTrans = bulletEnt.GetComponent<Transform>();
            Transform playerTrans = entity.GetComponent<Transform>();

            // 弾の位置を設定
            bullTrans.position = playerTrans.position + Quaternion.RotateVector(playerTrans.rotate, new Vector3(bulletSpawnOffset_.x, bulletSpawnOffset_.y , 0.0f));
            //方向を設定
            Bullet bulletSC = bulletEnt.GetScript<Bullet>();
            bulletSC.SetMoveDir(Quaternion.RotateVector(playerTrans.rotate, Vector3.up));

            shootCooldown_ = shootCooldownTime_;
        }
    }

    public bool isShot_ = false;

    private float shootCooldown_ = 0.0f;
    [SerializeField] private float shootCooldownTime_ = 0.2f;

    [SerializeField] private Vector2 bulletSpawnOffset_ = new Vector2(0.0f, 1.3f);
}

