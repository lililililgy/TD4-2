using System;

// 攻撃が命中したときに SE を鳴らす演出。AttackCollision と同じ Entity にアタッチする。
// 付いていない攻撃は鳴らない（＝敵の攻撃のプレハブには付けない）ので、
// 「プレイヤーが与えたダメージだけ鳴らす」といった切り分けはアタッチの有無で行う。
//
// ShakeOnHit / HitStopOnHit / EffectOnHit と同じ分担で、AttackCollision は
// 「ダメージが実際に通った」ことを伝えるだけ。何をどう鳴らすかはこのクラスの責務。
// 空振り（HP を持たない相手・弾かれた攻撃）では呼ばれないので、通った瞬間だけ鳴る。
//
// 鳴らすには同じ Entity に SEPlayer コンポーネントが必要（無ければ黙って鳴らない）。
public class SEOnHit : MonoScript {

    // 鳴らす音源のパス。空文字にすると鳴らさない。
    [SerializeField] private string sePath_ = "./Assets/Sounds/se/player-hit.mp3";

    [SerializeField] private float volume_ = 0.25f;
    [SerializeField] private float pitch_  = 1.0f;

    // 命中時に AttackCollision から呼ばれる。dealtDamage は会心倍率を掛けたあとの実ダメージ
    public void OnHit(float dealtDamage) {
        // ダメージ0では鳴らさない。プレイヤーの体当たりは速度依存で、
        // 低速の接触では 0 ダメージのまま命中扱いになるため（DamageShake と同じ足切り）。
        if (dealtDamage <= 0.0f) {
            return;
        }

        SEOneShot.Play(entity, sePath_, volume_, pitch_);
    }
}
