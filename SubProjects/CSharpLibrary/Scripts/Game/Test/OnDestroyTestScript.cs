using System;

// 破棄されるターゲットEntityにアタッチするスクリプト
public class OnDestroyTargetScript : MonoScript {
    public static bool IsDestroyedCalled = false;

    public override void OnDestroy() {
        IsDestroyedCalled = true;
        Debug.Log("OnDestroyTargetScript.OnDestroy called successfully!");
    }
}

// テスト実行とアサーションを担当するスクリプト
public class OnDestroyTestScript : MonoScript {
    private Entity targetEntity;
    private int frameCount = 0;
    private bool destroyedTriggered = false;

    public override void Initialize() {
        OnDestroyTargetScript.IsDestroyedCalled = false;
        
        // テスト用のEntityを生成
        targetEntity = ecsGroup.CreateEntity("Test");
        if (targetEntity == null) {
            throw new Exception("OnDestroyTest Failed: Could not create targetEntity");
        }
        
        // テストターゲットスクリプトをアタッチ
        targetEntity.AddScript<OnDestroyTargetScript>();
        Debug.Log("OnDestroyTestScript: targetEntity created and OnDestroyTargetScript added.");
    }

    public override void Update() {
        frameCount++;

        // 10フレーム目にEntityを破棄
        if (frameCount == 10 && !destroyedTriggered) {
            Debug.Log("OnDestroyTestScript: Destroying targetEntity...");
            targetEntity.Destroy();
            destroyedTriggered = true;
        }

        // 20フレーム目にOnDestroyが呼ばれたか確認
        if (frameCount == 20) {
            if (OnDestroyTargetScript.IsDestroyedCalled) {
                Debug.Log("OnDestroyTest Passed: OnDestroy was successfully called on the destroyed Entity's script!");
            } else {
                Debug.LogError("OnDestroyTest Failed: OnDestroy was NOT called after Entity.Destroy()");
                throw new Exception("OnDestroyTest Failed: OnDestroy was NOT called after Entity.Destroy()");
            }
        }
    }
}
