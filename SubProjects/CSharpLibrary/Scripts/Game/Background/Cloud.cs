using System;
using System.Collections.Generic;

public class Cloud : MonoScript {

	[SerializeField] public float speed = 1.0f;

	public override void Initialize() {

	}

	public override void Update() {
		transform.position.x += speed * Time.deltaTime;

		if (transform.position.x < -5000) {
			entity.Destroy();
		}

	}
}
