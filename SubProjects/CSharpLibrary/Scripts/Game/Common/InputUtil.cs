using System;
using System.Collections.Generic;

// キーボードとゲームパッドの割り当てをまとめて引くための共通処理。
//
// 入力スクリプトはどれも「[SerializeField] の List<KeyCode> と List<Gamepad> を順に見て、
// どれか1つでも反応していたら true」という同じループを書くことになるので、ここに集約する。
// リストが null でも落ちないので、[SerializeField] を .entity 側で持っていない場合も安全。
public static class InputUtil {

    // どれか1つでも押されているか（押している間ずっと true）。移動・射撃のような継続入力向け。
    public static bool AnyPressed(List<KeyCode> keys, List<Gamepad> buttons) {
        if (keys != null) {
            foreach (KeyCode key in keys) {
                if (Input.PressKey(key)) return true;
            }
        }

        if (buttons != null) {
            foreach (Gamepad button in buttons) {
                if (Input.PressGamepad(button)) return true;
            }
        }

        return false;
    }

    // どれか1つでも押された瞬間か（押した最初の1フレームだけ true）。
    // ポーズや決定のような、押しっぱなしで連続実行されると困る入力向け。
    public static bool AnyTriggered(List<KeyCode> keys, List<Gamepad> buttons) {
        if (keys != null) {
            foreach (KeyCode key in keys) {
                if (Input.TriggerKey(key)) return true;
            }
        }

        if (buttons != null) {
            foreach (Gamepad button in buttons) {
                if (Input.TriggerGamepad(button)) return true;
            }
        }

        return false;
    }
}
