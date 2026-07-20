using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


// OnDestroy()で KillEvent を 発行するスクリプト
public class EmitKillEventOnDestroy : MonoScript
{
    public override void OnDestroy()
    {
        HP hp = entity.GetScript<HP>();
        if (hp != null && hp.IsDead)
        {
            // HP が 0 になったときに撃破イベントを発行する
            MessageBus.Publish(new EnemyKilledEvent(entity.name));
        }
    }
}