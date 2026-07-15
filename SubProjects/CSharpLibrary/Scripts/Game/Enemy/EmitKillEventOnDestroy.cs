using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


// OnDestroy()で KillEvent を 発行するスクリプト
public class EmitKillEventOnDestroy : MonoScript {
    public override void OnDestroy() {
        MessageBus.Publish(new EnemyKilledEvent(entity.name));
    }
}