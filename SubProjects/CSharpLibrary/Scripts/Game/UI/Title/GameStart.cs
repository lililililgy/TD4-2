using System;
using System.Collections.Generic;

public class GameStart : MonoScript {
	bool selected = false;
	bool deselected = false;
	float time = 0f;
	[SerializeField] float maxTime = 1f;
	[SerializeField] Vector3 minScale = Vector3.one;
	[SerializeField] Vector3 maxScale = Vector3.one;

	public void OnSelect() {
		selected = true;
		deselected = false;
		time = 0f;
	}

	public void OnDeselect() {
		selected = false;
		deselected = true;
		time = 0f;
	}

	public void OnSubmit() {
		/// ゲームを最初から開始する
		GameFlow.StartNewGame();
	}

	public override void Update() {
		if (selected) {
			time += Time.deltaTime;
			float lerpT = Mathf.Clamp01(time / maxTime);

			Vector3 scale = Vector3.Lerp(minScale, maxScale, lerpT);
			transform.scale = scale;

			if (time > maxTime) {
				selected = false;
			}
		}


		if (deselected) {
			time += Time.deltaTime;
			float lerpT = Mathf.Clamp01(time / maxTime);

			Vector3 scale = Vector3.Lerp(maxScale, minScale, lerpT);
			transform.scale = scale;

			if (time > maxTime) {
				selected = false;
			}
		}
	}
}
