using System;
using System.Collections.Generic;

public class OpenOption : MonoScript {

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
		Entity parentGroup = entity.parent;
		if (parentGroup != null) {
			UIGroupComponent uiGroupComp = parentGroup.GetComponent<UIGroupComponent>();
			if (uiGroupComp != null) {
				uiGroupComp.isVisible = false;
				uiGroupComp.isFocused = false;
			}
		}


		Entity optionGroup = ecsGroup.FindEntity("OptionMenu");
		if (optionGroup != null) {
			UIGroupComponent uiGroupComp = optionGroup.GetComponent<UIGroupComponent>();
			if (uiGroupComp != null) {
				uiGroupComp.isVisible = true;
				uiGroupComp.isFocused = true;
			}
		}
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
