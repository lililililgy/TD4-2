using System;

// ダッシュ中だけパーティクルを出す演出。プレイヤー Entity に付ける。
//
// パーティクルのプレハブは ParticleSystem2D を looping / playOnAwake で持っているので、
// ダッシュ開始で1体生成し、終了で破棄するだけで「ダッシュ中だけ出続ける」になる。
// 破棄しても既に出ているパーティクルは engine 側がゴーストとして寿命まで描き続けるため
// (EntityCollection が破棄時に RegisterGhost する)、末尾がぶつ切りにはならない。
//
// PlayerDashedEvent / PlayerDashEndedEvent は PlayerDashState の OnEnter()/OnExit() が
// 遷移確定時に1回だけ発行する。MessageBus は同期発行なので実行順に依存しない。
// 購読の作法は DashZoomSource と同じ。
public class DashParticle : MonoScript {

    // 生成するプレハブ名（Assets/Prefabs/ 配下のファイル名から .prefab を除いたもの）
    [SerializeField] private string particlePrefabName_ = "playerDashParticle";

    // プレイヤーから見たローカルのずらし量。負の z/x で後方に置くなど。
    // プレイヤーの向きで回してから足すので、旋回してもプレイヤーとの位置関係は保たれる。
    [SerializeField] private Vector3 offset_ = Vector3.zero;

    private Entity particleEntity_ = null;
    private bool   subscribed_     = false;

    // 初回の取りこぼしを避けるため Initialize() ではなく Awake() で購読する。
    public override void Awake() {
        if (!subscribed_) {
            MessageBus.Subscribe<PlayerDashedEvent>(OnDashed);
            MessageBus.Subscribe<PlayerDashEndedEvent>(OnDashEnded);
            subscribed_ = true;
        }
    }

    // 親子付けはしない。プレイヤーはスケールが大きく、付けるとパーティクルまで巨大化するため。
    // 代わりに座標を毎フレーム追従させる（FlapjackOctopus の泳ぎパーティクルと同じ方針）。
    public override void Update() {
        if (!particleEntity_ || particleEntity_.transform == null || transform == null) {
            return;
        }
        particleEntity_.transform.SetPositionImmediate(SpawnPosition());
    }

    public override void OnDestroy() {
        if (subscribed_) {
            MessageBus.Unsubscribe<PlayerDashedEvent>(OnDashed);
            MessageBus.Unsubscribe<PlayerDashEndedEvent>(OnDashEnded);
            subscribed_ = false;
        }

        DestroyParticle();
    }

    private void OnDashed(PlayerDashedEvent e) {
        // 連打で開始が二重に来ても増殖しないよう、先に前回のぶんを畳む
        DestroyParticle();

        if (String.IsNullOrEmpty(particlePrefabName_) || ecsGroup == null || transform == null) {
            return;
        }

        Entity particle = ecsGroup.CreateEntity(particlePrefabName_);
        if (!particle || particle.transform == null) {
            return;
        }

        particle.transform.SetPositionImmediate(SpawnPosition());
        particleEntity_ = particle;
    }

    private void OnDashEnded(PlayerDashEndedEvent e) {
        DestroyParticle();
    }

    private void DestroyParticle() {
        if (particleEntity_) {
            particleEntity_.Destroy();
        }
        particleEntity_ = null;
    }

    private Vector3 SpawnPosition() {
        return transform.position + (transform.rotate * offset_);
    }
}
