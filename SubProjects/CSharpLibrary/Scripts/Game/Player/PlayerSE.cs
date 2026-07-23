using System;

// プレイヤーの効果音(SE)。ダッシュした瞬間・たまごを発射した瞬間・ダメージを受けた瞬間の3つを鳴らす。
// SEPlayer コンポーネントを持つプレイヤー Entity に付ける。
//
// 3種類を1つの SEPlayer で鳴らし分けるため、再生のたびに path を渡せる OneShotPlay を使う
// （SEPlayer は Entity に1つしか持てず、Play() はコンポーネントに設定された path しか鳴らせない）。
// path のクリップが読み込まれていなければ engine 側が黙って無視するので、
// 音源ファイルを置くまでは単に鳴らないだけで、エラーにはならない。
//
// ダッシュ・発射はイベントで拾う。どちらも「実際に成立した瞬間」にだけ発行されるので空振りしない
// （ダッシュは PlayerDashState.OnEnter() の遷移確定時、発射は PlayerShotComponent が
//   クールダウン・弾切れの early return を全て抜けて弾を生成したあと）。
// 初回の取りこぼしを避けるため購読は Awake() で行う（DashParticle と同じ作法）。
//
// 被弾だけは対応するイベントが無いので HP.TotalDamageTaken(累計)の差分で検知する。
// CurrentHp の減少では代用できない。プレイヤーの HP は残機(=卵数)と同期していて、
// 発射で卵を消費したときにも減るため被弾と区別が付かない（ShakeOnDamaged と同じ理由）。
public class PlayerSE : MonoScript {

    // 鳴らす音源のパス。空文字にするとその音は鳴らさない。
    [SerializeField] private string dashSePath_    = "./Assets/Sounds/se/player-dash.mp3";
    [SerializeField] private string shotSePath_    = "./Assets/Sounds/se/player-shot.mp3";
    [SerializeField] private string damagedSePath_ = "./Assets/Sounds/se/player-damaged.mp3";

    // 音量。音源ごとの録音レベルの差はここで吸収する
    [SerializeField] private float dashVolume_    = 0.2f;
    [SerializeField] private float shotVolume_    = 0.2f;
    [SerializeField] private float damagedVolume_ = 0.25f;

    [SerializeField] private float pitch_ = 1.0f;

    private HP hp_;
    private float lastTotalDamage_ = 0.0f;
    private bool subscribed_ = false;

    public override void Awake() {
        if (!subscribed_) {
            MessageBus.Subscribe<PlayerDashedEvent>(OnDashed);
            MessageBus.Subscribe<PlayerShotEvent>(OnShot);
            subscribed_ = true;
        }
    }

    public override void Initialize() {
        hp_ = entity.GetScript<HP>();
        // 開始時点の累計を基準にする（初期化前のぶんで1発鳴ってしまうのを防ぐ）
        lastTotalDamage_ = hp_ != null ? hp_.TotalDamageTaken : 0.0f;
    }

    public override void Update() {
        if (hp_ == null) {
            return;
        }

        float total = hp_.TotalDamageTaken;
        bool damaged = total > lastTotalDamage_;
        lastTotalDamage_ = total;

        // 同一フレームに複数回被弾しても累計の差分は1回ぶんにまとまるので、音も1回で済む
        if (damaged) {
            Play(damagedSePath_, damagedVolume_);
        }
    }

    public override void OnDestroy() {
        if (subscribed_) {
            MessageBus.Unsubscribe<PlayerDashedEvent>(OnDashed);
            MessageBus.Unsubscribe<PlayerShotEvent>(OnShot);
            subscribed_ = false;
        }
    }

    private void OnDashed(PlayerDashedEvent e) {
        Play(dashSePath_, dashVolume_);
    }

    private void OnShot(PlayerShotEvent e) {
        Play(shotSePath_, shotVolume_);
    }

    private void Play(string path, float volume) {
        if (String.IsNullOrEmpty(path) || !entity) {
            return;
        }

        SEPlayer sePlayer = entity.GetComponent<SEPlayer>();
        if (!sePlayer) {
            return;
        }

        // OneShotPlay は engine 側のリクエストキューに積むだけなので、
        // イベントのコールバック（＝他スクリプトの Update 中）から呼んでも問題ない。
        sePlayer.OneShotPlay(volume, pitch_, path);
    }
}
