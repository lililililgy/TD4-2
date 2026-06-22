# ONEngine UI System 導入・開発マニュアル

本ドキュメントは、ONEngineで実装された新しいUIシステムについて、**エディタ（UI Editor）の使い方**、**スクリプトの書き方**、および**ゲーム本編（ランタイム）への組み込み方法**をまとめたガイドです。

---

## 1. UIシステムの全体設計

このシステムは、キーボードやゲームパッドによるゲームライクなUI操作（フォーカス移動や決定）を、ECS (Entity Component System) アーキテクチャに則って効率的に処理するために設計されています。

### 主要コンポーネント
- **`UIGroupComponent`**: UI要素のグループ（メニュー全体やダイアログなど）を統括します。グループ内の選択状態（`currentSelected`）や、フォーカス有無（`isFocused`）、表示状態（`isVisible`）を管理します。
- **`UIElementComponent`**: ボタンやスライダーなど、個別の選択可能なUI要素を表します。文字列ID（`elementId`）やインデックス（`elementIndex`）を保持します。
- **`UILinkNavigationComponent`**: 特定のキー入力があった際、どのUI要素にフォーカスを遷移させるかのマッピング（ナビゲーションリンク）を保持します。

---

## 2. エディタ（UI Editor）の使い方

新しく追加された **`UI` タブ** を使用して、視覚的にUIナビゲーションの設計とプレビュー同期を行います。

### 画面構成
1. **UITab (UI Editor)**:
   - **Inspector##UI**: 選択したUI要素やグループのプロパティを編集します。
   - **UI Node Editor**: ノードを配置し、接続（リンク）をワイヤリングするビジュアルエディタです。
   - **Hierarchy##UI**: アクティブシーン上のエンティティ構造をツリー表示します。

### 基本操作手順
1. **ノードの配置**:
   - ノードエディタのキャンバス上で右クリックし、「**Add Group Node**」または「**Add Element Node**」を選択してノードを生成します。
   - **Groupノード**: メニュー全体のフォーカス制御や可視性を担当します。
   - **Elementノード**: 各種ボタンなどの要素を担当します。
2. **プロパティ設定**:
   - 各ノード内の入力ボックスで `elementId` や `elementIndex` を設定します。
   - グループノード側で、初期選択される要素（`currentSelected`）や、初期フォーカス状態（`Is Focused`）を指定します。
3. **ナビゲーションリンクの配線 (Link)**:
   - Elementノードの右側には、各種遷移キーに対応する**出力ピン (Out)** が用意されています（`UpArrow`, `DownArrow`, `LeftArrow`, `RightArrow` など）。
   - キー出力をドラッグし、遷移先となる別のElementノードの左側にある**入力ピン (In)** に接続します。
   - これにより、「下キー（DownArrow）が押されたら、ボタンBに遷移する」といった動作が作成されます。
4. **保存と同期**:
   - エディタ上部のツールバーにある「**Save**」ボタンをクリックすると、設定データがプレハブ形式（デフォルト: `./Assets/UI/TitleMenu.prefab`）で保存されます。
   - 「**Sync Preview**」ボタンをクリックすると、現在のエディタ上の設計をゲーム画面のECSGroup（アクティブなゲームシーン）に直ちに反映（動的生成）し、実際の挙動をプレビューできます。

---

## 3. スクリプトの書き方 (C# Scripting)

UIのインタラクション（フォーカス時の強調表示、決定時のイベント）は、C#スクリプトをUI要素やグループのエンティティにアタッチし、特定の**コールバックメソッド**を実装することで行います。

### コールバックイベント一覧

| メソッド名 | 呼び出し対象 | 説明 | 引数 |
| :--- | :--- | :--- | :--- |
| **`OnSelect()`** | 個別の `UIElement` 側スクリプト | 自身がフォーカス（選択）されたときに呼ばれます。 | なし |
| **`OnDeselect()`** | 個別の `UIElement` 側スクリプト | フォーカスが外れたときに呼ばれます。 | なし |
| **`OnSubmit()`** | 個別の `UIElement` 側スクリプト | フォーカス中に決定キー（Enter, Space, パッドAボタン）が押された時に呼ばれます。 | なし |
| **`OnUISelect(string elementId)`** | 親 `UIGroup` 側スクリプト | グループ内でフォーカス対象が切り替わったときに呼ばれます。 | `elementId`: 選択された要素のID |
| **`OnUISubmit(string elementId)`** | 親 `UIGroup` 側スクリプト | グループ内のいずれかの要素が決定されたときに呼ばれます。 | `elementId`: 決定された要素のID |

### 実装サンプルコード (C#)

#### ① 個別のボタン要素用スクリプト (例: `MenuButton.cs`)
```csharp
using System;

public class MenuButton : MonoScript {
    private SpriteRenderer spriteRenderer;

    void Start() {
        // ハイライト表現用のスプライトレンダラーを取得しておく
        spriteRenderer = entity.GetComponent<SpriteRenderer>();
    }

    // 選択された時の処理 (例: スプライトを黄色にする)
    public void OnSelect() {
        if (spriteRenderer) {
            spriteRenderer.color = new Color(1f, 1f, 0f, 1f); // Yellow
        }
        Debug.LogInfo(entity.name + " がフォーカスされました");
    }

    // 選択が外れた時の処理 (例: スプライトを白に戻す)
    public void OnDeselect() {
        if (spriteRenderer) {
            spriteRenderer.color = new Color(1f, 1f, 1f, 1f); // White
        }
    }

    // 決定（決定キー押下）時の処理
    public void OnSubmit() {
        Debug.LogInfo(entity.name + " が決定されました！");
        // ボタン固有の個別処理をここに記述
    }
}
```

#### ② メニュー全体を管理するスクリプト (例: `TitleMenuController.cs`)
```csharp
using System;

public class TitleMenuController : MonoScript {
    
    // グループ全体で共通の決定処理を行う場合に便利
    public void OnUISubmit(string elementId) {
        Debug.LogInfo("Group Submit: " + elementId);
        
        switch (elementId) {
            case "btn_start":
                StartGame();
                break;
            case "btn_options":
                OpenOptions();
                break;
            case "btn_exit":
                ExitGame();
                break;
        }
    }

    private void StartGame() {
        Debug.LogInfo("ゲームを開始します...");
        // シーン遷移ロジックなど
        // SceneManager.LoadScene("GameStage1");
    }

    private void OpenOptions() {
        Debug.LogInfo("オプションを開きます...");
    }

    private void ExitGame() {
        Debug.LogInfo("ゲームを終了します...");
    }
}
```

---

## 4. ゲーム本編（ランタイム）への組み込み

ゲーム実行時にUIプレハブをロードし、実際に動作させるための流れです。

### プレハブの生成
UIエディタで作成・保存した `.prefab` ファイルは、シーン初期化時に `SceneIO` や `ECSGroup::GenerateEntityFromPrefab` を使用してエンティティ群として動的生成します。

*C++側での生成コード例:*
```cpp
// ゲーム用のECSGroupを取得
ONEngine::ECSGroup* gameGroup = pEcs_->GetECSGroup("GameScene");

// プレハブからUIエンティティ群を読み込み・生成
// （※UIHierarchySystemが自動的に親子関係やisVisible状態の同期を行います）
gameGroup->GenerateEntityFromPrefab("./Assets/UI/TitleMenu.prefab");
```

### システムの自動制御
シーン内でゲームが実行されている間、登録されたシステムが以下の制御を自動的にバックグラウンドで実行します。
1. **表示制御 (`UIHierarchySystem`)**:
   - `UIGroupComponent` の `isVisible` を `false` に切り替えると、自動的にそのグループ配下にあるすべてのUIエンティティの `active` フラグが `false` に設定され、**描画・入力処理・スクリプト更新（Start/Update等）が停止**します。
   - 再び `true` にすることで、一括で表示・動作が再開されます。
2. **入力処理 (`UIInputNavigationSystem`)**:
   - グループの `isFocused` が `true` の時のみキーボード/ゲームパッドの入力をスキャンします。
   - 設定されたキーコード（DirectInput準拠）と一致するキーが押されると、対応する遷移先に `currentSelected` を変更し、アタッチされたMonoBehaviourの `OnSelect`, `OnDeselect` 等を自動的に呼び出します。
