using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class PlayerShotComponent : MonoScript {

    private RoeManager roeManager_;

    public override void Initialize() {
        roeManager_ = entity.GetScript<RoeManager>();
    }

    public override void Update() {

        // Update the shoot cooldown timer
        shootCooldown_ -= Time.deltaTime;
        shootCooldown_ = Math.Max(shootCooldown_, 0.0f);

        PlayerInputComponent inputComponent = entity.GetScript<PlayerInputComponent>();
        if (!inputComponent.IsShootButtonPressed) {
            return;
        }

        // 発射不可状態の検知 (プレイヤーのステートによるものなど)
        PlayerStateComponent stateComponent = entity.GetScript<PlayerStateComponent>();
        if (!stateComponent.CanShoot()) {
            return;
        }

        if (shootCooldown_ <= 0.0f) {
            // 弾の実体である成熟卵を1つ消費する。無ければ撃てない。
            Entity matureRoe = roeManager_ != null ? roeManager_.TryConsumeMature() : null;
            if (matureRoe == null) {
                return;
            }

            // Entity の 作成
            Entity bulletEnt = ecsGroup.CreateEntity("Bullet");

            Transform bullTrans = bulletEnt.GetComponent<Transform>();
            Transform playerTrans = entity.GetComponent<Transform>();

            // 弾はプレイヤーの向いている方向へ bulletSpawnOffset_ ぶんずらした位置から出す。
            bullTrans.position = playerTrans.position
                + Quaternion.RotateVector(playerTrans.rotate, new Vector3(bulletSpawnOffset_.x, bulletSpawnOffset_.y, 0.0f));
            //方向を設定
            Bullet bulletSC = bulletEnt.GetScript<Bullet>();
            bulletSC.SetMoveDir(Quaternion.RotateVector(playerTrans.rotate, Vector3.up));

            // 消費した卵を破棄
            matureRoe.Destroy();

            // 弾が実際に発射された（early return を全て抜けた）ことを通知する。
            // チュートリアルの「射撃した」検知用。
            MessageBus.Publish(new PlayerShotEvent());

            shootCooldown_ = shootCooldownTime_;
        }
    }

    public bool isShot_ = false;

    private float shootCooldown_ = 0.0f;
    [SerializeField] private float shootCooldownTime_ = 0.2f;

    [SerializeField] private Vector2 bulletSpawnOffset_ = new Vector2(0.0f, 1.3f);
}

