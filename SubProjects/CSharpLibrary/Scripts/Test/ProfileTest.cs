using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public class ProfileTest : MonoScript {

	public override void Update() {
		//MeshRenderer renderer = entity.GetComponent<MeshRenderer>();
		//renderer.color = new Vector4(1.0f, 0.0f, 0.0f, 1.0f);

		if (Input.TriggerKey(KeyCode.Space)) {


			Entity create = ecsGroup.CreateEntity("Test");
			create.transform.position = new Vector3(10, 10, 10);

		}
	}



}
