# AudioSource コンポーネント説明書

`AudioSource` は、ゲーム内で効果音やBGMなどの音声アセットを再生・管理するためのコンポーネントです。

---

## 主な機能

1. **持続音声の再生・停止 (`Play` / `Stop`)**
   - 設定されたオーディオファイルを再生または停止します。
   - すでに再生中に `Play()` を呼び出した場合、以前の音声は自動的に停止し、即座に最初から再生し直されます。
2. **ワンショット再生 (`OneShotPlay`)**
   - 既存の再生状態に影響を与えることなく、指定したパスの音声を重ねて1回だけ再生します。
3. **音量とピッチの動的変更 (`SetParams` / `volume` / `pitch`)**
   - 再生中であっても、動的に音量やピッチ（再生速度）を変更することができます。

---

## C# スクリプトでの使い方

`AudioSource` は `Component` を継承しているため、エンティティから取得して使用します。

### 基本的な再生と停止

```csharp
// エンティティから AudioSource コンポーネントを取得
AudioSource audio = entity.GetComponent<AudioSource>();

if (audio != null) {
    // 1. 再生するアセットのパスを設定
    audio.path = "Assets/Audio/bgm.mp3";
    
    // 2. 音量とピッチの設定 (デフォルト: volume = 1.0f, pitch = 1.0f)
    audio.volume = 0.8f;
    audio.pitch = 1.0f;
    
    // 3. 再生を開始 (再生中の場合は最初から再生し直されます)
    audio.Play();
}
```

```csharp
// 再生中の音声を停止する
audio.Stop();
```

### パラメータの動的変更

```csharp
// 再生中に音量とピッチを変更する
float newVolume = 0.5f;
float newPitch = 1.2f; // 少し高音・早回しにする
audio.SetParams(newVolume, newPitch);
```

### 効果音（SE）などの重ね合わせ再生 (ワンショット)

BGMとは異なり、攻撃音や決定音など、前回の音を消さずに重ねて再生したい場合は `OneShotPlay` を使用します。

```csharp
// 指定したパスの音声を指定の音量・ピッチでワンショット再生
audio.OneShotPlay(1.0f, 1.0f, "Assets/Audio/se_laser.wav");
```

---

## メンバー変数・メソッド仕様

### プロパティ / 変数

| 変数名 | 型 | 説明 | デフォルト値 |
| :--- | :--- | :--- | :--- |
| `path` | `string` | 再生する音声ファイルのパス（相対パス）。`.mp3`, `.wav`, `.ogg` に対応。 | `""` |
| `volume` | `float` | 音声の音量。 `0.0` (無音) から `1.0` (最大) の範囲で指定します。 | `1.0f` |
| `pitch` | `float` | 再生速度・ピッチ。 `1.0` が標準で、値を大きくすると高音・高速化します。 | `1.0f` |

### メソッド

#### `void Play()`
設定された `path` の音声ファイルを再生します。すでに再生中の場合は、現在の音声を停止し、最初から再生を開始します。

#### `void Stop()`
現在再生中の音声を停止します。

#### `void SetParams(float volume, float pitch)`
音量とピッチを即座に変更して適用します。

#### `void OneShotPlay(float volume, float pitch, string path)`
指定した `path` の音声ファイルを重ねて一度だけ再生します。

---

## エディタ (ImGui デバッグ) での操作

C++エンジンのエディタ上（ImGui）から、エンティティの `AudioSource` を選択して以下の操作が可能です。

- **Audio Path のドラッグ＆ドロップ**: アセットビューなどから音声ファイルをドラッグ＆ドロップして再生ファイルを割り当てられます。
- **Volume / Pitch スライダー**: リアルタイムに音量とピッチを調整し、テストできます。
- **Play ボタン**: エディタ上で即座に音声をテスト再生できます。
