using System;
using System.Collections.Generic;

/// <summary>
/// 弾倉クラス。弾の最大数、残弾数、リロード処理などを管理する。
/// </summary>
public class Magazine : MonoScript {
    public override void Update() {
        PlayerInputComponent input = entity.GetScript<PlayerInputComponent>();
        if (input == null) {
            return;
        }

        // リロード処理
        if (input.IsReloadButtonPressed && CanReload) {
            SetCurrentAmmo(maxAmmo_);
        }

    }

    public bool CanReload {
        get { return currentAmmo_ < maxAmmo_; }
    }
    public bool IsEmpty {
        get { return currentAmmo_ <= 0; }
    }

    [SerializeField] private int maxAmmo_ = 10;
    int currentAmmo_ = 0;


    public int GetCurrentAmmo {
        get { return currentAmmo_; }
    }
    public void SetCurrentAmmo(int ammo) {
        currentAmmo_ = Mathf.Clamp(ammo, 0, maxAmmo_);
    }

    public int GetMaxAmmo {
        get { return maxAmmo_; }
    }

}
