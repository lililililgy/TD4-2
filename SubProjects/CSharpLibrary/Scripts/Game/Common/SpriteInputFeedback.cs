using System.Collections.Generic;

// 指定したキー / ゲームパッドボタンの状態に応じて、同じ Entity のスプライトに演出を付ける。
// 演出は3項目あり、それぞれ独立に ON/OFF できる。使わない項目には一切書き込まない。
//   1. 色    … 元色 → pressedColor_ へ補間(Vector4: x=R, y=G, z=B, w=A)
//   2. scale … 元スケール → 元スケール * pressedScale_ へ補間
//   3. UV    … SpriteAnimation で分割したコマを差し替える(通常コマ ⇔ 押下コマ)
//
// 発火の仕方は triggerMode_ で切り替える。
//   Held … 押している間ずっと演出 ON。離すと元へ戻る(attackTime_ / releaseTime_ で補間)
//   Down … 押した瞬間に発火し、duration_ 秒かけて元へ戻るワンショット
//   Up   … 離した瞬間に発火し、duration_ 秒かけて元へ戻るワンショット
// どのモードでも内部的には強度 intensity_(0=通常, 1=演出全開)1本に落とし込み、
// 色と scale はその値で補間する。UV だけは中間値を作れないので intensity_ > 0 で切り替える。
//
// 時間は既定で Time.unscaledDeltaTime(実時間の差分)を積む。ヒットストップやポーズで
// timeScale が下がっていてもボタンの手応えは等速で返したいため。ゲーム内オブジェクトで
// スローの影響を受けさせたい場合は useUnscaledTime_ を false にする。
//
// UV について:
//   分割数は同じ Entity の SpriteAnimation があればそこから読む(rows / cols / invertY)。
//   無ければ下の rows_ / cols_ / invertY_ を使う。
//   SpriteAnimation が再生中だと毎フレーム uvTransform を上書きされて演出が潰れるので、
//   演出中は Pause() し、戻すときに Play() で再開させる(Play() 側が UV も戻してくれる)。
public class SpriteInputFeedback : MonoScript {

    public enum TriggerMode {
        Held, // 押している間
        Down, // 押した瞬間
        Up,   // 離した瞬間
    }

    // --- 入力 ---
    [SerializeField] private List<KeyCode> keys_    = new List<KeyCode>();
    [SerializeField] private List<Gamepad> buttons_ = new List<Gamepad>();

    // --- 発火 ---
    [SerializeField] private TriggerMode triggerMode_ = TriggerMode.Held;
    [SerializeField] private float attackTime_  = 0.05f; // Held: 演出が全開になるまでの秒数
    [SerializeField] private float releaseTime_ = 0.12f; // Held: 離してから元へ戻るまでの秒数
    [SerializeField] private float duration_    = 0.15f; // Down / Up: 全開から元へ戻るまでの秒数
    [SerializeField] private bool  useUnscaledTime_ = true;

    // --- 色 ---
    [SerializeField] private bool    enableColor_  = true;
    [SerializeField] private Vector4 pressedColor_ = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);

    // --- scale ---
    // 元スケールに対する倍率。(1.2, 0.9, 1.0) のように軸別に潰す/伸ばすこともできる。
    [SerializeField] private bool    enableScale_  = false;
    [SerializeField] private Vector3 pressedScale_ = new Vector3(1.2f, 1.2f, 1.0f);

    // --- UV(SpriteAnimation の分割コマ) ---
    // normalFrame_ が -1 のときは「元の UV に戻す」扱い(SpriteAnimation があれば再生を再開する)。
    [SerializeField] private bool enableUv_     = false;
    [SerializeField] private int  normalFrame_  = -1;
    [SerializeField] private int  pressedFrame_ = 0;
    // SpriteAnimation が無い Entity 用のフォールバック分割設定。
    [SerializeField] private int  rows_    = 1;
    [SerializeField] private int  cols_    = 1;
    [SerializeField] private bool invertY_ = false;

    private SpriteRenderer  renderer_;
    private SpriteAnimation animation_;

    // 演出前の状態(戻り先)。白/等倍とは限らないので実値を控える。
    private Vector4     baseColor_;
    private Vector3     baseScale_;
    private UVTransform baseUv_;

    private float intensity_ = 0.0f; // 0=通常, 1=演出全開
    private bool  oneShotActive_ = false;
    private bool  uvOverriding_  = false; // 今 UV を我々が握っているか
    private bool  animWasPlaying_ = false;

    public override void Initialize() {
        renderer_  = entity.GetComponent<SpriteRenderer>();
        animation_ = entity.GetScript<SpriteAnimation>();

        if (renderer_ == null) {
            Debug.LogError("SpriteInputFeedback: SpriteRenderer component not found on Entity ID: " + entity.Id);
            return;
        }

        baseColor_ = renderer_.color;
        baseScale_ = transform.scale;
        baseUv_    = renderer_.uvTransform;
    }

    public override void Update() {
        if (renderer_ == null) {
            return;
        }

        float dt = useUnscaledTime_ ? Time.unscaledDeltaTime : Time.deltaTime;

        UpdateIntensity(dt);
        Apply();
    }

    // 入力とモードから intensity_(0..1)を進める。
    private void UpdateIntensity(float dt) {
        if (triggerMode_ == TriggerMode.Held) {
            // 押下中は 1 へ、離したら 0 へ、それぞれの秒数で線形に寄せる。
            bool  pressed = InputUtil.AnyPressed(keys_, buttons_);
            float target  = pressed ? 1.0f : 0.0f;
            float span    = pressed ? attackTime_ : releaseTime_;

            // 秒数が 0 以下なら補間せず即座に切り替える(0除算回避も兼ねる)。
            if (span <= 0.0f) {
                intensity_ = target;
                return;
            }

            float step = dt / span;
            if (intensity_ < target) {
                intensity_ = Mathf.Clamp01(intensity_ + step);
            } else if (intensity_ > target) {
                intensity_ = Mathf.Clamp01(intensity_ - step);
            }
            return;
        }

        // ワンショット(Down / Up)。発火した時点で全開にし、あとは duration_ で 0 へ落とす。
        // 演出中に再度発火したら 1 に戻すだけなので、連打しても破綻しない。
        if (IsOneShotTriggered()) {
            intensity_     = 1.0f;
            oneShotActive_ = true;
            return;
        }

        if (!oneShotActive_) {
            return;
        }

        if (duration_ <= 0.0f) {
            intensity_     = 0.0f;
            oneShotActive_ = false;
            return;
        }

        intensity_ = Mathf.Clamp01(intensity_ - dt / duration_);
        if (intensity_ <= 0.0f) {
            oneShotActive_ = false;
        }
    }

    private bool IsOneShotTriggered() {
        if (triggerMode_ == TriggerMode.Down) {
            return InputUtil.AnyTriggered(keys_, buttons_);
        }
        return AnyReleased();
    }

    // InputUtil には離した瞬間の判定が無いのでここで持つ。
    private bool AnyReleased() {
        if (keys_ != null) {
            foreach (KeyCode key in keys_) {
                if (Input.ReleaseKey(key)) return true;
            }
        }

        if (buttons_ != null) {
            foreach (Gamepad button in buttons_) {
                if (Input.ReleaseGamepad(button)) return true;
            }
        }

        return false;
    }

    private void Apply() {
        if (enableColor_) {
            renderer_.color = Vector4.Lerp(baseColor_, pressedColor_, intensity_);
        }

        if (enableScale_) {
            Vector3 pressed = new Vector3(
                baseScale_.x * pressedScale_.x,
                baseScale_.y * pressedScale_.y,
                baseScale_.z * pressedScale_.z);
            transform.scale = Vector3.Lerp(baseScale_, pressed, intensity_);
        }

        if (enableUv_) {
            ApplyUv(intensity_ > 0.0f);
        }
    }

    // UV は補間できないので、演出中かどうかの2値でコマを切り替える。
    // 切り替わったフレームだけ書き込む(毎フレーム書くと SpriteAnimation と取り合いになる)。
    private void ApplyUv(bool active) {
        if (active == uvOverriding_) {
            return;
        }
        uvOverriding_ = active;

        if (active) {
            // 演出中はアニメを止めて UV を我々が握る。止めた事実を覚えておき、戻すときに再開する。
            if (animation_ != null) {
                animWasPlaying_ = animation_.IsPlaying;
                animation_.Pause();
            }
            renderer_.uvTransform = MakeUv(pressedFrame_);
            return;
        }

        // 通常へ戻す。アニメが動いていたなら Play() が自分のコマで UV を上書きしてくれる。
        if (animation_ != null && animWasPlaying_) {
            animation_.Play();
            animWasPlaying_ = false;
            return;
        }

        renderer_.uvTransform = normalFrame_ < 0 ? baseUv_ : MakeUv(normalFrame_);
    }

    // コマ番号 → UV 矩形。分割の解釈は SpriteAnimation.UpdateUV と同じにしてある。
    private UVTransform MakeUv(int frameIndex) {
        int rows = animation_ != null ? animation_.rows : rows_;
        int cols = animation_ != null ? animation_.cols : cols_;
        bool invertY = animation_ != null ? animation_.invertY : invertY_;

        if (rows <= 0 || cols <= 0 || frameIndex < 0) {
            return baseUv_;
        }

        int colIndex = frameIndex % cols;
        int rowIndex = frameIndex / cols;

        float uSize = 1f / (float)cols;
        float vSize = 1f / (float)rows;

        float uOffset = colIndex * uSize;
        // invertY: 左下(0,0)基準。1行目が一番上に来るように行を反転する。
        float vOffset = invertY ? (rows - 1 - rowIndex) * vSize : rowIndex * vSize;

        UVTransform uv = new UVTransform();
        uv.scale    = new Vector2(uSize, vSize);
        uv.position = new Vector2(uOffset, vOffset);
        uv.rotate   = 0.0f;
        return uv;
    }

    public override void OnDestroy() {
        // 演出途中で破棄されても、色/スケールは Entity ごと消えるので後始末は不要。
        // ただしアニメを止めたまま破棄されると他所から参照された場合に止まったままになるため戻す。
        if (uvOverriding_ && animation_ != null && animWasPlaying_) {
            animation_.Play();
        }
    }
}
