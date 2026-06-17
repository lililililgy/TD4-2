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

    }


    public Vector2 moveDir_;

    public bool isShootButtonPressed_ = false;

    private KeyCode[] shootKeys_ = { KeyCode.Space, KeyCode.Z, KeyCode.X };
    private Gamepad[] shotButtons_ = { Gamepad.LeftShoulder, Gamepad.RightShoulder };

}