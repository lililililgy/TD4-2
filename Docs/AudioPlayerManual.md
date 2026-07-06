# BGMPlayer / SEPlayer コンポーネント利用マニュアル

音響再生処理の改善に伴い、従来の全般的な `AudioSource` コンポーネントを廃止し、BGM再生専用の **`BGMPlayer`** およびSE（効果音）再生専用の **`SEPlayer`** に役割を分割しました。
本ドキュメントでは、それぞれの特徴、インスペクターでの設定項目、およびスクリプト（C# / C++）からの制御方法についてまとめています。

---

## 1. 概要と役割の違い

BGMとSEで求められる動作仕様・リソース管理の違いに応じて、コンポーネントを分離しています。

| 機能 / 特徴 | `BGMPlayer` (BGM専用) | `SEPlayer` (SE効果音専用) |
| :--- | :--- | :--- |
| **同時再生数** | **1音のみ（システム全体で排他制御）**<br>※新しいBGM再生時、古いBGMは自動で停止・破棄されます。 | **無制限（重ねて再生・マルチプレイ可能）**<br>※同一プレイヤーから何回でも重ねて鳴らせます。 |
| **ループ再生** | **サポート（無限ループ前提）**<br>※再生中にループ設定を動的にON/OFFできます。 | **非推奨（ワンショット再生が基本）** |
| **Play On Awake** | **サポート**<br>※シーン開始時にデフォルトで自動再生する設定が可能。 | **非対応** (スクリプト等のイベント契機で再生) |
| **自動メモリ解放** | 再生終了時にボイスを自動破棄。 | 再生完了したソースボイスを毎フレーム検知し自動破棄。 |

---

## 2. BGMPlayer の使い方

### インスペクター（Editor）での設定項目
- **Audio Path**: 再生する音声ファイル（`.mp3` / `.wav` / `.ogg`）のアセットパス。アセットをドラッグ＆ドロップで設定可能です。
- **Volume**: 音量（`0.0` 〜 `1.0`）。再生中にリアルタイムに変更が反映されます。
- **Pitch**: ピッチ・再生速度（`0.0` 〜 `3.0`）。再生中にリアルタイムに変更が反映されます。
- **Loop**: 有効な場合、音声が終了した際に最初からループ再生します。
  - *再生中にこのチェックを外すと、現在の周回が終わった時点で自動で停止します。*
- **Play On Awake**: 有効な場合、シーン遷移やゲーム再生（ランタイム）が始まった最初のフレームで自動でBGMを再生します。

---

### C# スクリプトからの制御

#### API リファレンス (`BGMPlayer.cs`)
```csharp
public class BGMPlayer : Component {
    public float volume = 1f;
    public float pitch = 1f;
    public bool loop = true;
    public bool playOnAwake = true;
    public string path = "";

    // 再生を開始する
    public void Play();
    
    // 再生を停止する
    public void Stop();

    // 複数のパラメータをまとめて同期設定する
    public void SetParams(float volume, float pitch, bool loop, bool playOnAwake);
}
```

#### C# 実装サンプル
```csharp
using System;

public class GameBGMController : Component {
    private BGMPlayer bgmPlayer;

    public override void Start() {
        // 同一エンティティからBGMPlayerを取得
        bgmPlayer = entity.GetComponent<BGMPlayer>();
        
        if (bgmPlayer != null) {
            // パラメータを設定して再生
            bgmPlayer.SetParams(0.5f, 1.0f, true, true);
            bgmPlayer.Play();
        }
    }

    public override void Update() {
        // 例: 特定のキーでBGMを止める
        if (Input.GetKeyDown(KeyCode.Space)) {
            bgmPlayer?.Stop();
        }
    }
}
```

---

### C++ 側での制御（エンジン実装向け）
```cpp
// BGMPlayerの取得と再生
if (auto* bgm = ecsGroup->GetComponent<BGMPlayer>(entityId)) {
    bgm->SetAudioPath("Packages/Assets/Audio/title_bgm.mp3");
    bgm->SetLoop(true);
    bgm->Play(); // 再生リクエストを送信
}
```

---

## 3. SEPlayer の使い方

### インスペクター（Editor）での設定項目
- **Audio Path**: 再生する音声ファイル（`.mp3` / `.wav` / `.ogg`）のアセットパス。
- **Volume**: 音量（`0.0` 〜 `1.0`）。
- **Pitch**: ピッチ・再生速度（`0.0` 〜 `3.0`）。

---

### C# スクリプトからの制御

#### API リファレンス (`SEPlayer.cs`)
```csharp
public class SEPlayer : Component {
    public float volume = 1f;
    public float pitch = 1f;
    public string path = "";

    // 登録されているAudio PathのSEを再生する（重ねて再生可能）
    public void Play();
    
    // このプレイヤーから再生したすべてのSEを強制停止する
    public void Stop();

    // 指定した別ファイルのSEを単発で再生する（パス、音量、ピッチを個別指定）
    public void PlayOneShot(string audioPath, float customVolume = 1.0f, float customPitch = 1.0f);
}
```

#### C# 実装サンプル
```csharp
using System;

public class PlayerSoundEffects : Component {
    private SEPlayer sePlayer;

    public override void Start() {
        sePlayer = entity.GetComponent<SEPlayer>();
    }

    // 足音の再生 (設定されているデフォルトのSEを重ねて再生)
    public void PlayFootstep() {
        sePlayer?.Play();
    }

    // ダメージ音の再生 (別ファイルを指定してワンショット再生)
    public void PlayDamageSound() {
        sePlayer?.PlayOneShot("Packages/Assets/Audio/damage.wav", 0.8f, 1.0f);
    }

    // 攻撃音の再生
    public void PlayAttackSound() {
        sePlayer?.PlayOneShot("Packages/Assets/Audio/slash.wav", 0.7f, 1.2f);
    }
}
```

---

### C++ 側での制御（エンジン実装向け）
```cpp
// SEPlayerの取得と再生
if (auto* se = ecsGroup->GetComponent<SEPlayer>(entityId)) {
    se->Play(); // 重ねて再生
    
    // または直接OneShotリクエストを送る
    se->PlayOneShot("Packages/Assets/Audio/explosion.wav", 1.0f, 1.0f);
}
```

---

## 4. エディタ内での追加方法

1. シーン上の任意の Entity を選択します。
2. インスペクター（Inspector）ウィンドウ下部にある **「Add Component」** ボタンをクリックします。
3. カテゴリー一覧に新設された **`Audio`** を選択します。
4. **`BGMPlayer`** または **`SEPlayer`** を選択して追加します。

> [!NOTE]
> `Audio` カテゴリーから追加されたオーディオコンポーネントは、ヘッダー背景が**専用のカラー（パープル系）**で表示されるため、インスペクター上で他の計算系コンポーネント（Compute）と直感的に区別できます。

---

## 5. 安全な運用と自動制御

システム内部で以下の自動クリーンアップが行われるため、メモリリークや強制終了時のクラッシュを意識せず安全に使用できます。

> [!IMPORTANT]
> - **シーン遷移時の自動停止**: シーンが切り替わると、古いシーンの `BGMPlayer` / `SEPlayer` に紐づく音声は自動でフェードアウト・停止処理が走り、XAudio2のソースボイスが安全に破棄されます（音が残り続けるバグは発生しません）。
> - **エンジン停止時の安全設計**: 音声が再生されている状態でエディタの再生を止めたり、ウィンドウの×ボタンでアプリを終了した場合も、内部の破棄順序の整合性を保証しているため、アクセス違反によるクラッシュを引き起こさずに正常終了します。
