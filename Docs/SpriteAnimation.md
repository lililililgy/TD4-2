# SpriteAnimation コンポーネント使用マニュアル

`SpriteAnimation` は、1枚のスプライトシート（グリッド状の連番画像）を用いて、UV座標を制御することでコマ送りアニメーションを実現するコンポーネントです。

---

## 1. 前提条件

アニメーションさせたい Entity には、以下のコンポーネントがアタッチされている必要があります。
* `Script` コンポーネント（そこに `SpriteAnimation` スクリプトを追加）
* `SpriteRenderer` コンポーネント

---

## 2. エディタ（インスペクタ）での設定項目

`Script` コンポーネントに `SpriteAnimation` を追加すると、以下の `SerializeField` パラメータが表示されます。

| パラメータ | 型 | 説明 |
| :--- | :--- | :--- |
| **rows** | `int` | スプライトシートの**縦の分割数**（行数）。 |
| **cols** | `int` | スプライトシートの**横の分割数**（列数）。 |
| **fps** | `float` | 1秒間あたりの再生フレーム数（コマ数）。（例: `10.0`） |
| **isLoop** | `bool` | アニメーションをループ再生するかどうか。 |
| **isPlay** | `bool` | ゲーム開始時に自動で再生を開始するかどうか。 |
| **totalFrames** | `int` | 再生する総フレーム数。シートの全てのコマを使う場合は `0` に設定します。一部のコマのみ使う場合はその数（例: 12コマだけ使う場合は `12`）を設定します。 |
| **invertY** | `bool` | 画像のUV基準座標。DirectXなどの**左上(0,0)基準**の場合は `false`。OpenGLなどの**左下(0,0)基準**の場合は `true` にします。（※当エンジンは基本的に `false` 推奨です） |

> [!WARNING]
> **重要な注意点**
> エディタ上でパラメータを編集した後は、ゲームを再生（Play）する前に**必ずシーンの保存（`File -> Save Scene` または `Ctrl + S`）**を行ってください。保存せずに再生すると、設定値がディスク上の古い値（初期値 `1` 等）にリセットされてしまいます。

---

## 3. C# スクリプトからの制御 (API)

他のゲームスクリプトから `SpriteAnimation` コンポーネントを取得して、再生・停止などを制御できます。

### メソッド

* `Play()`
  アニメーションを一時停止から再開、または再生を開始します。
* `Pause()`
  アニメーションを現在のフレームで一時停止します。
* `Stop()`
  アニメーションを停止し、フレームを `0` にリセットします。
* `ResetAnimation()`
  再生タイマーと現在のフレームを `0` にリセットしてUV座標を更新します。
* `SetFrame(int frameIndex)`
  指定したフレームインデックス（`0` から始まる数値）へ強制的にコマをジャンプさせます。

### プロパティ

* `CurrentFrame` (`int`) [getter/setter]
  現在のフレームインデックスを取得、または設定します。
* `IsPlaying` (`bool`) [getter]
  現在アニメーションが再生中かどうかを取得します。
* `IsFinished` (`bool`) [getter]
  アニメーションが最後まで再生し終わったかどうかを取得します（`isLoop` が `false` の時のみ有効）。

---

## 4. イベントコールバック

アニメーションの進行状況に応じたイベント処理を行えます。

* `event Action OnAnimationFinished`
  アニメーションの再生が終了した瞬間に呼ばれます（`isLoop = false` の時のみ）。
* `event Action<int> OnFrameChanged`
  フレーム（コマ）が切り替わるたびに、新しいフレーム番号（`int`）を引数として呼ばれます。

---

## 5. 具体的なコード例

### 例1：エフェクトアニメーション終了時に自動で Entity を削除する
```csharp
using System;

public class AutoDestroyEffect : MonoScript {
    private SpriteAnimation anim;

    public override void Initialize() {
        anim = entity.GetScript<SpriteAnimation>();
        if (anim != null) {
            // アニメーション終了時のイベントを登録
            anim.OnAnimationFinished += OnPlaybackFinished;
        }
    }

    private void OnPlaybackFinished() {
        // イベントの登録解除
        anim.OnAnimationFinished -= OnPlaybackFinished;
        // Entityを削除する
        entity.Destroy();
    }
}
```

### 例2：特定のフレーム（足音や攻撃判定の発生）で処理を行う
```csharp
public class PlayerAttackController : MonoScript {
    private SpriteAnimation anim;
    private AudioSource audioSource;

    public override void Initialize() {
        anim = entity.GetScript<SpriteAnimation>();
        audioSource = entity.GetComponent<AudioSource>();

        if (anim != null) {
            anim.OnFrameChanged += OnFrameChanged;
        }
    }

    private void OnFrameChanged(int frameIndex) {
        // 例: 攻撃アニメーションの3コマ目（インデックス2）でSEを鳴らす
        if (frameIndex == 2 && audioSource != null) {
            audioSource.Play();
        }
    }
}
```
