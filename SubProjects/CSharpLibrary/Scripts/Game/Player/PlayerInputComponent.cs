using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


public class PlayerInputComponent
    : MonoScript {
    public override void Initialize() { }

    public override void Update() {
        // 移動
        moveDir_ = Input.KeyboardAxis(KeyboardAxis.WASD);
        moveDir_ += Input.KeyboardAxis(KeyboardAxis.Arrow);
        moveDir_ += Input.GamepadThumb(GamepadAxis.LeftThumb);
        moveDir_ = moveDir_.Normalized();

        //発射
        isShootButtonPressed_ = InputUtil.AnyPressed(shootKeys_, shotButtons_);

        // ダッシュ
        isDashButtonPressed_ = InputUtil.AnyPressed(dashKeys_, dashButtons_);
    }

    public Vector2 MoveDir {
        get { return moveDir_; }
    }

    public bool IsShootButtonPressed {
        get { return isShootButtonPressed_; }
    }
    public bool IsDashButtonPressed {
        get { return isDashButtonPressed_; }
    }

    // 移動方向（正規化済み）
    private Vector2 moveDir_;

    // 発射ボタンが押されているか（押している間 true）
    private bool isShootButtonPressed_ = false;
    private bool isDashButtonPressed_ = false;

    // 弾の発射ボタン（キーボードとゲームパッドの両方をサポート）
    [SerializeField] private List<KeyCode> shootKeys_ = new List<KeyCode> { KeyCode.Space, KeyCode.Z, KeyCode.X };
    [SerializeField] private List<Gamepad> shotButtons_ = new List<Gamepad> { Gamepad.LeftShoulder, Gamepad.RightShoulder };

    // ダッシュボタン（キーボードとゲームパッドの両方をサポート）
    [SerializeField] private List<KeyCode> dashKeys_ = new List<KeyCode> { KeyCode.LeftShift };
    [SerializeField] private List<Gamepad> dashButtons_ = new List<Gamepad> { Gamepad.A };

}