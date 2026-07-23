using System.Collections.Generic;

// 被弾したときの手触り演出をまとめて出す。HP と同じ Entity(Player=頭)にアタッチする。
//   1. 一定時間の無敵(i-frame)。連続ヒットで一気に溶けるのを防ぐ。
//   2. 無敵中はスプライトを点滅させる(点滅色は指定可能)。
//   3. コントローラーの振動(ラムブル)。
//   4. Time.timeScale を一定時間下げるヒットストップ。
//
// 見た目の本体は頭・胴体・触手で別 Entity に分かれている(親子関係なし。ライフは頭の HP が1つだけ持ち、
// 胴体/触手は DamageRelay で頭を参照する構成)。点滅は頭が1本のタイマーで全パーツをまとめて駆動する。
// こうすると全パーツの点滅位相が完全に揃う。対象は blinkEntityNames_ に Entity 名で列挙する。
//
// 検知は ShakeOnDamaged と同じく HP.TotalDamageTaken(累計)の差分で行う。理由:
//   ・CurrentHp の減少では代用できない。プレイヤーの HP は残機(卵数)と同期され、
//     発射で卵を消費したときにも減るため、被弾と区別が付かない。
//   ・「今フレーム被弾した」フラグ方式は、立てる側と読む側の実行順で取りこぼす。
//
// タイマーは Time.unscaledDeltaTime(timeScale の影響を受けない実フレーム差分)を積算して測る。
// ヒットストップ中は deltaTime が timeScale で縮むため、deltaTime を積むと無敵時間まで一緒に
// 伸びてしまう。実時間で進む unscaledDeltaTime を積めば、狙った秒数で確実に解ける。
public class DamageFeedback : MonoScript {

    // --- 無敵 ---
    [SerializeField] private float invincibleDuration_ = 1.0f; // 被弾後この秒数だけ無敵(実時間)

    // --- 点滅 ---
    // 無敵中、対象スプライトの色を元色とこの色で交互に切り替えて点滅させる。
    // 既定は赤寄せ(被弾らしさ)。x=R, y=G, z=B, w=A。
    [SerializeField] private Vector4 blinkColor_    = new Vector4(1.0f, 0.4f, 0.4f, 1.0f);
    [SerializeField] private float   blinkInterval_ = 0.08f; // 1回の点灯/消灯の長さ(秒・実時間)
    // 点滅させる Entity 名。自分(頭)に SpriteRenderer があればそれも自動で加える。
    [SerializeField] private List<string> blinkEntityNames_ = new List<string> {
        "Player_Body",
        "Player_Tentacle",
    };

    // --- ヒットストップ ---
    [SerializeField] private float hitstopTimeScale_ = 0.15f; // この倍率まで一時的に減速
    [SerializeField] private float hitstopDuration_  = 0.12f; // 減速を保つ秒数(実時間)

    // --- 振動 ---
    // PlayGamepadVibration(左モーター, 右モーター, 継続秒) は継続時間をエンジン側で管理するので、
    // ここでタイマーを持つ必要はない。値の意味はエンジン依存なので調整用に露出しておく。
    [SerializeField] private float rumbleLeftMotor_  = 12.5f;
    [SerializeField] private float rumbleRightMotor_ = 0.8f;
    [SerializeField] private float rumbleDuration_   = 0.25f;

    private HP hp_;
    private float lastTotalDamage_;

    // 点滅対象の SpriteRenderer と、その元色(戻り先)を平行に持つ。
    private readonly List<SpriteRenderer> blinkRenderers_ = new List<SpriteRenderer>();
    private readonly List<Vector4>        originalColors_ = new List<Vector4>();

    private bool  invincibleActive_ = false;
    private float invincibleTimer_  = 0.0f; // 無敵開始からの経過(実時間)

    private bool  hitstopActive_ = false;
    private float hitstopTimer_  = 0.0f;    // ヒットストップ開始からの経過(実時間)

    public override void Initialize() {
        hp_ = entity.GetScript<HP>();
        // 開始時点の累計を基準にする(初期化前の被弾で1発演出が出るのを防ぐ)
        lastTotalDamage_ = hp_ != null ? hp_.TotalDamageTaken : 0.0f;

        // 自分(頭)に描画があれば点滅対象に含める
        AddBlinkTarget(entity);
        // 胴体・触手など別 Entity のパーツを名前で解決して含める
        for (int i = 0; i < blinkEntityNames_.Count; i++) {
            AddBlinkTarget(ecsGroup.FindEntity(blinkEntityNames_[i]));
        }
    }

    private void AddBlinkTarget(Entity target) {
        if (target == null) {
            return;
        }
        SpriteRenderer r = target.GetComponent<SpriteRenderer>();
        if (r == null) {
            return;
        }
        blinkRenderers_.Add(r);
        originalColors_.Add(r.color); // 白とは限らないので実値を控える
    }

    public override void Update() {
        if (hp_ == null) {
            return;
        }

        // timeScale に依存しない実時間の刻み。ヒットストップ中もこれで正しく進む。
        float dt = Time.unscaledDeltaTime;

        float total = hp_.TotalDamageTaken;
        float damage = total - lastTotalDamage_;
        lastTotalDamage_ = total;

        // 実際に HP が削れたときだけ発火する。無敵中は TakeDamage が弾くので damage は増えず、
        // ここも自然に再発火しない(=既存の無敵を我々が誤って上書きすることもない)。
        if (damage > 0.0f) {
            TriggerFeedback();
        }

        // 無敵の進行(点滅と解除)
        if (invincibleActive_) {
            invincibleTimer_ += dt;
            UpdateBlink();
            if (invincibleTimer_ >= invincibleDuration_) {
                hp_.IsInvincible = false;
                invincibleActive_ = false;
                RestoreColors(); // 点滅を確実に元へ戻す
            }
        }

        // ヒットストップの進行(解除で元の等速へ戻す)
        if (hitstopActive_) {
            hitstopTimer_ += dt;
            if (hitstopTimer_ >= hitstopDuration_) {
                Time.timeScale = 1.0f;
                hitstopActive_ = false;
            }
        }
    }

    private void TriggerFeedback() {
        // 無敵(タイマーを 0 から測り直す)
        hp_.IsInvincible = true;
        invincibleActive_ = true;
        invincibleTimer_  = 0.0f;

        // ヒットストップ
        Time.timeScale = hitstopTimeScale_;
        hitstopActive_ = true;
        hitstopTimer_  = 0.0f;

        // 振動
        Input.PlayGamepadVibration(rumbleLeftMotor_, rumbleRightMotor_, rumbleDuration_);
    }

    private void UpdateBlink() {
        // interval が 0 以下だと割れるので、その場合は点滅させず元色のまま
        bool lit = blinkInterval_ > 0.0f && ((int)(invincibleTimer_ / blinkInterval_) & 1) == 1;

        for (int i = 0; i < blinkRenderers_.Count; i++) {
            SpriteRenderer r = blinkRenderers_[i];
            if (r == null) {
                continue;
            }
            r.color = lit ? blinkColor_ : originalColors_[i];
        }
    }

    private void RestoreColors() {
        for (int i = 0; i < blinkRenderers_.Count; i++) {
            SpriteRenderer r = blinkRenderers_[i];
            if (r != null) {
                r.color = originalColors_[i];
            }
        }
    }

    public override void OnDestroy() {
        // ヒットストップ中にプレイヤーが破棄されると Update が止まり、
        // timeScale が下がったままゲーム全体が減速する。破棄時に必ず戻す。
        if (hitstopActive_) {
            Time.timeScale = 1.0f;
            hitstopActive_ = false;
        }
    }
}
