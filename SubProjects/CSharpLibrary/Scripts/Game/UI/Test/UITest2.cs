using System;
using System.Collections.Generic;

public class UITest2 : MonoScript {
	public void OnSelect() {
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer) {
			renderer.color = Vector4.blue;
		}
	}

	public void OnDeselect() {
		SpriteRenderer renderer = entity.GetComponent<SpriteRenderer>();
		if (renderer) {
			renderer.color = Vector4.one;
		}
	}
}
