using System;
using System.Collections.Generic;

public class GesoWeakPoint : MonoScript {

	public override void Initialize() {
		
	}

	public override void Update() {
		
	}

	public void Damage(int damage)
    {
        var hp = entity.GetScript<HP>();
        if (hp != null)
        {
            hp.Damage(damage);
        }
    }
}
