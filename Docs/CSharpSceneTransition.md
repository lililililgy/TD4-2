# C# シーン遷移ドキュメント

本ドキュメントでは、ONEngine (TD4-2) において C# スクリプトからシーン遷移を行う方法について説明します。

## 1. シーン遷移の基本

シーン遷移には `SceneManager` クラスの `LoadScene` メソッドを使用します。

### API
```csharp
static public void SceneManager.LoadScene(string sceneName)
```

- **sceneName**: 読み込みたいシーンの名前（ファイル名から拡張子を除いたもの）。

## 2. シーンファイルの配置

シーンファイル（`.scene`）は以下のディレクトリに配置されている必要があります。

- `Project/Assets/Scene/`

例として、`Project/Assets/Scene/Title.scene` を読み込みたい場合は、`"Title"` を指定します。

## 3. 実装例

以下は、特定の入力があった際にシーンを切り替える簡単な例です。

```csharp
using System;

public class SceneChanger : MonoScript {
    public override void Update() {
        // スペースキーが押されたら GameScene に遷移
        if (Input.IsTriggerKey(KeyCode.Space)) {
            SceneManager.LoadScene("GameScene");
        }
    }
}
```
※ `Input` や `KeyCode` の仕様はプロジェクトの `Input` クラスの定義に従ってください。

## 4. 遷移の挙動と注意点

### 4.1 遷移のタイミング
`LoadScene` を呼び出すと、内部的に「次のシーン」が予約されます。実際の遷移処理（現在のシーンの破棄と新しいシーンの読み込み）は、**現在のフレームの更新処理（Update）の最後**、または次フレームの開始時に行われます。

### 4.2 内部で行われる処理
シーン遷移時には以下の処理が自動的に行われます：
1. **GPU同期**: 描画命令の完了を待機し、リソースの安全な破棄を確保します。
2. **ECSのクリア**: 現在のシーンに属するすべてのエンティティとコンポーネントが破棄されます。
3. **新シーンの読み込み**: 指定された `.scene` ファイル（JSON形式）を解析し、エンティティとコンポーネントを再構築します。
4. **時間のリセット**: `Time.ResetTime()` が呼ばれ、シーン開始時からの経過時間がリセットされます。

### 4.3 拡張子の省略
`LoadScene` に渡す文字列には、拡張子の `.scene` を含めないでください。内部で自動的に付与されます。
