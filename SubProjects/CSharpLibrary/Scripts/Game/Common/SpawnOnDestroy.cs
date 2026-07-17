using System;
using System.Collections.Generic;

public class SpawnOnDestroy : MonoScript
{
    [SerializeField] private string spawnPrefabName;
    [SerializeField] private Vector3 spawnPos;

    public override void OnDestroy()
    {
        if (spawnPrefabName == null)
        {
            return;
        }

        // 敵の生成処理
        Entity spawnedEnemy = ecsGroup.CreateEntity(spawnPrefabName);
        Transform enemyT = spawnedEnemy.GetComponent<Transform>();
        enemyT.position = spawnPos;
    }

}