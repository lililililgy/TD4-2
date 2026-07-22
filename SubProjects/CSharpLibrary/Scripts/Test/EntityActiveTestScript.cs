using System;
using ONEngine;

public class EntityActiveTestScript : MonoScript
{
    private int frameCount = 0;

    public override void Update()
    {
        frameCount++;
        if (frameCount == 1)
        {
            // 1フレーム目のUpdateでEntity全体を非アクティブにする
            entity.enable = false;
        }
    }
}
