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
        isShootButtonPressed_ = false;

        foreach (var key in shootKeys_) {
            if (Input.PressKey(key)) {
                isShootButtonPressed_ = true;
                break;
            }
        }

        if (isShootButtonPressed_) {
            foreach (var button in shotButtons_) {
                if (Input.PressGamepad(button)) {
                    isShootButtonPressed_ = true;
                    break;
                }
            }
        }

        // ダッシュ（押した瞬間のみ）
        isDashButtonPressed_ = false;

        foreach (var key in dashKeys_) {
            if (Input.PressKey(key)) {
                isDashButtonPressed_ = true;
                break;
            }
        }

        if (!isDashButtonPressed_) {
            foreach (var button in dashButtons_) {
                if (Input.PressGamepad(button)) {
                    isDashButtonPressed_ = true;
                    break;
                }
            }
        }

    }


    public Vector2 moveDir_;

    public bool isShootButtonPressed_ = false;
    public bool isDashButtonPressed_ = false;

    [SerializeField] private List<KeyCode> shootKeys_ = new List<KeyCode> { KeyCode.Space, KeyCode.Z, KeyCode.X };
    [SerializeField] private List<Gamepad> shotButtons_ = new List<Gamepad> { Gamepad.LeftShoulder, Gamepad.RightShoulder };

    [SerializeField] private List<KeyCode> dashKeys_ = new List<KeyCode> { KeyCode.LeftShift };
    [SerializeField] private List<Gamepad> dashButtons_ = new List<Gamepad> { Gamepad.A };

}