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

        // 発射不可状態の検知 (プレイヤーのステートによるものや残弾数が0の場合など)
        PlayerStateComponent stateComponent = entity.GetScript<PlayerStateComponent>();
        Magazine magazine = entity.GetScript<Magazine>();
        if (!stateComponent.CanShoot() || magazine.IsEmpty) {
            return;
        }

        if (shootCooldown_ <= 0.0f) {
            // 弾薬の実体である幼生卵を1つ消費する。無ければ撃てない。
            Entity larva = roeManager_ != null ? roeManager_.TryConsumeLarvae() : null;
            if (larva == null) {
                return;
            }

            // Entity の 作成
            Entity bulletEnt = ecsGroup.CreateEntity("Bullet");

            Transform bullTrans = bulletEnt.GetComponent<Transform>();
            Transform playerTrans = entity.GetComponent<Transform>();
            Transform larvaTrans = larva.GetComponent<Transform>();

            // 弾の位置は消費した幼生卵の位置（隊列の卵が飛んでいく絵）。取れなければプレイヤー位置から。
            bullTrans.position = larvaTrans != null
                ? larvaTrans.position
                : playerTrans.position + Quaternion.RotateVector(playerTrans.rotate, new Vector3(bulletSpawnOffset_.x, bulletSpawnOffset_.y, 0.0f));
            //方向を設定
            Bullet bulletSC = bulletEnt.GetScript<Bullet>();
            bulletSC.SetMoveDir(Quaternion.RotateVector(playerTrans.rotate, Vector3.up));

            // 消費した卵を破棄
            larva.Destroy();

            shootCooldown_ = shootCooldownTime_;
        }
    }

    public bool isShot_ = false;

    private float shootCooldown_ = 0.0f;
    [SerializeField] private float shootCooldownTime_ = 0.2f;

    [SerializeField] private Vector2 bulletSpawnOffset_ = new Vector2(0.0f, 1.3f);
}

