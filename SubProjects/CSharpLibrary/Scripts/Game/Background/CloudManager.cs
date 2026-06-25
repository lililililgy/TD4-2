using System;
using System.Collections.Generic;

public class CloudManager : MonoScript {


	float spawnCT = 0.0f;
	[SerializeField] float spawnTime = 3.0f;


	public override void Initialize() {
		spawnCT = spawnTime;
	}

	public override void Update() {

		spawnCT -= Time.deltaTime;
		if (spawnCT < 0.0f) {
			spawnCT = spawnTime;

			Entity cloud = ecsGroup.CreateEntity("Cloud");
			cloud.transform.position = new Vector3(-5000.0f, RandomUtil.RandomRange(-1, 1), 0.0f);
		}
	}
}
