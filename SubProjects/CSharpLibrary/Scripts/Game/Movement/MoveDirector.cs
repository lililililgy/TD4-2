using System;

// 入力の「解釈」を担う素のクラス（コンポーネントではない）。
// 生の入力方向とステートの意図フラグ(canMove / moveForward)から、
// Mover に渡す「最終的な進行方向」を決める。
//
// これにより Mover は物理だけの汎用クラスに保てる：
//   ドライバ(例: PlayerMoveComponent)が Resolve() の結果を Mover.Move へ渡す。
//
//   - canMove     : この状態で移動入力を受け付けるか。false なら停止方向(長さ0)を返す。
//   - moveForward : true なら入力方向を無視し、記録済みの正面(forward)方向を返す
//                   （ダッシュ/ドッジなどの「正面へ走り続ける」挙動）。
//
// forward 方向は「通常移動中(canMove かつ入力あり)」に入力方向で更新し続ける。
// 例えばダッシュ直前まで向いていた方向が forward として保持され、ダッシュ中はそこへ突進する。
public class MoveDirector {

    // rawDir: 生の入力方向（正規化不要。長さ0で入力なし）。
    // 戻り値: Mover.Move に渡す最終的な進行方向（長さ0で停止）。
    public Vector2 Resolve(Vector2 rawDir, bool canMove, bool moveForward) {
        const float kThresholdSq = 0.0001f;

        // 通常移動中は入力方向を「正面」として記録しておく。
        if (canMove && rawDir.LengthSq() > kThresholdSq) {
            forwardDir_ = rawDir.Normalized();
        }

        if (moveForward) {
            return forwardDir_;             // 入力を無視して正面へ走り続ける
        }
        if (canMove) {
            return rawDir;                  // 入力方向へ移動
        }
        return new Vector2(0.0f, 0.0f);     // 移動不可（停止）
    }

    // 記録中の正面方向（参照用）。
    public Vector2 ForwardDir { get { return forwardDir_; } }

    private Vector2 forwardDir_ = new Vector2(0.0f, 1.0f);
}
