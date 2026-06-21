using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class PlayerShotComponent : MonoScript {

    List<Entity> bullets_ = new List<Entity>();

    public override void Update() {

        // Update the shoot cooldown timer
        shootCooldown_ -= Time.deltaTime;
        shootCooldown_ = Math.Max(shootCooldown_, 0.0f);

        // ダッシュ中などは発射不可
        PlayerStateComponent stateComponent = entity.GetScript<PlayerStateComponent>();
        if (!stateComponent.CanShoot()) {
            return;
        }

        PlayerInputComponent inputComponent = entity.GetScript<PlayerInputComponent>();

        if (!inputComponent.isShootButtonPressed_) {
            return;
        }

        if (shootCooldown_ <= 0.0f) {
            // Entity の 作成
            Entity bulletEnt = ecsGroup.CreateEntity("Bullet");
            bullets_.Add(bulletEnt);

            Transform bullTrans = bulletEnt.GetComponent<Transform>();
            Transform playerTrans = entity.GetComponent<Transform>();

            // 弾の位置を設定
            bullTrans.position = playerTrans.position + new Vector3(bulletSpawnOffset_.x, 0.0f, bulletSpawnOffset_.y);

            shootCooldown_ = shootCooldownTime_;
        }
    }

    public bool isShot_ = false;

    private float shootCooldown_ = 0.0f;
    [SerializeField] private float shootCooldownTime_ = 0.2f;

    [SerializeField] private Vector2 bulletSpawnOffset_ = new Vector2(0.0f, 1.3f);
}

