using System;

// 「0.0〜1.0 の割合を持っているもの」の共通口。SpriteGauge がこれを毎フレーム引いて棒の長さにする。
//
// HP・経験値・チャージ時間…と、バーで見せたい値は型も名前もバラバラなので、
// 表示側がそれぞれの具象型（HP なら CurrentHpRatio、経験値なら GetExpProgress）を
// 知りに行くと、値の種類が増えるたびに表示側を書き換えることになる。
// 「割合を答えられる」ことだけを約束させて、表示側は中身を知らないままにする。
//
// 実装側ですること: MonoScript に : IGaugeSource を足して GetGaugeRatio() を書くだけ。
// 表示側の設定（どのエンティティを見るか）は SpriteGauge の [SerializeField] で行う。
//
// 注意: Entity.GetScript<T>() は型名一致で引くのでインターフェースでは取れない。
// SpriteGauge は GetScripts() を回して as で拾っている（ObjectiveSystem が Objective を集めるのと同じ）。
public interface IGaugeSource {

    // 0.0（空）〜1.0（満タン）。範囲外を返しても SpriteGauge 側で丸めるので、
    // 実装側で厳密にクランプしなくてよい。
    float GetGaugeRatio();
}
