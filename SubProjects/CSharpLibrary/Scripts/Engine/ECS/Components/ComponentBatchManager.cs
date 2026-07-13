using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;

// 変更点: Allocatorも配列全体(IComponentArray)を受け取るように変更
delegate Array ComponentBatchConverter(IComponentArray array);
delegate Array ComponentBatchAllocator(IComponentArray array);

static class ComponentBatchManager {
	private static Dictionary<Type, ComponentBatchConverter> converters = new Dictionary<Type, ComponentBatchConverter>();
	private static Dictionary<Type, ComponentBatchAllocator> allocators = new Dictionary<Type, ComponentBatchAllocator>();

	public static void Initialize() {

		// --- Transform の登録 ---

		// 送信用コンバータ
		RegisterConverter<Transform, Transform.BatchData>((ComponentArray<Transform> array) => {
			int count = array.Count;
			Transform.BatchData[] batch = new Transform.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
				batch[i].position = comp.position;
				batch[i].rotate = comp.rotate;
				batch[i].scale = comp.scale;
			}
			return batch;
		});

		// 受信用アロケータ (変更点: IDを事前に埋める)
		RegisterAllocator<Transform, Transform.BatchData>((ComponentArray<Transform> array) => {
			int count = array.Count;
			Transform.BatchData[] batch = new Transform.BatchData[count];

			// C++側がどのエンティティのデータを書き込むべきか判別できるように、
			// 予め compId を設定した状態で配列を作成する
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
			}
			return batch;
		});


		// --- MeshRenderer の登録 ---
		RegisterConverter<MeshRenderer, MeshRenderer.BatchData>((ComponentArray<MeshRenderer> array) => {
			int count = array.Count;
			MeshRenderer.BatchData[] batch = new MeshRenderer.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				var batchData = comp.GetBatchData();
				batch[i].compId = comp.compId;
				batch[i].color = batchData.color;
				batch[i].postEffectFlags = batchData.postEffectFlags;
				batch[i].uvTransform = batchData.uvTransform;
			}
			return batch;
		});

		RegisterAllocator<MeshRenderer, MeshRenderer.BatchData>((ComponentArray<MeshRenderer> array) => {
			int count = array.Count;
			MeshRenderer.BatchData[] batch = new MeshRenderer.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
			}
			return batch;
		});

		// --- SpriteRenderer の登録 ---
		RegisterConverter<SpriteRenderer, SpriteRenderer.BatchData>((ComponentArray<SpriteRenderer> array) => {
			int count = array.Count;
			SpriteRenderer.BatchData[] batch = new SpriteRenderer.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				var batchData = comp.GetBatchData();
				batch[i].compId = comp.compId;
				batch[i].color = batchData.color;
				batch[i].textureSize = batchData.textureSize;
				batch[i].uvTransform = batchData.uvTransform;
			}
			return batch;
		});

		RegisterAllocator<SpriteRenderer, SpriteRenderer.BatchData>((ComponentArray<SpriteRenderer> array) => {
			int count = array.Count;
			SpriteRenderer.BatchData[] batch = new SpriteRenderer.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
			}
			return batch;
		});

		// --- BoxCollider2D の登録 ---
		RegisterConverter<BoxCollider2D, BoxCollider2D.BatchData>((ComponentArray<BoxCollider2D> array) => {
			int count = array.Count;
			BoxCollider2D.BatchData[] batch = new BoxCollider2D.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				var batchData = comp.GetBatchData();
				batch[i].compId = comp.compId;
				batch[i].size = batchData.size;
				batch[i].isTrigger = batchData.isTrigger;
				batch[i].mass = batchData.mass;
				batch[i].useOwnerScale = batchData.useOwnerScale;
			}
			return batch;
		});

		RegisterAllocator<BoxCollider2D, BoxCollider2D.BatchData>((ComponentArray<BoxCollider2D> array) => {
			int count = array.Count;
			BoxCollider2D.BatchData[] batch = new BoxCollider2D.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
			}
			return batch;
		});

		// --- DissolveMeshRenderer の登録 ---
		RegisterConverter<DissolveMeshRenderer, DissolveMeshRenderer.BatchData>((ComponentArray<DissolveMeshRenderer> array) => {
			int count = array.Count;
			DissolveMeshRenderer.BatchData[] batch = new DissolveMeshRenderer.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
				batch[i].threshold = comp.threshold;
				batch[i].uvTransform = comp.uvTransform;
			}
			return batch;
		});

		RegisterAllocator<DissolveMeshRenderer, DissolveMeshRenderer.BatchData>((ComponentArray<DissolveMeshRenderer> array) => {
			int count = array.Count;
			DissolveMeshRenderer.BatchData[] batch = new DissolveMeshRenderer.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
			}
			return batch;
		});

		// --- CameraComponent の登録 ---
		RegisterConverter<CameraComponent, CameraComponent.BatchData>((ComponentArray<CameraComponent> array) => {
			int count = array.Count;
			CameraComponent.BatchData[] batch = new CameraComponent.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
				// 本来は各行列なども同期すべきだが、まずは構造体サイズの一致を優先
			}
			return batch;
		});

		RegisterAllocator<CameraComponent, CameraComponent.BatchData>((ComponentArray<CameraComponent> array) => {
			int count = array.Count;
			CameraComponent.BatchData[] batch = new CameraComponent.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
			}
			return batch;
		});

		// --- AgentIntentComponent の登録 ---
		RegisterConverter<AgentIntentComponent, AgentIntentComponent.BatchData>((ComponentArray<AgentIntentComponent> array) => {
			int count = array.Count;
			AgentIntentComponent.BatchData[] batch = new AgentIntentComponent.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
				batch[i].desiredMoveDirection = comp.desiredMoveDirection;
				batch[i].desiredRotation = comp.desiredRotation;
				batch[i].rotationSpeed = comp.rotationSpeed;
				batch[i].maxSpeed = comp.maxSpeed;
				batch[i].useDesiredRotation = (byte)(comp.useDesiredRotation ? 1 : 0);
				batch[i].isAttacking = (byte)(comp.isAttacking ? 1 : 0);
				batch[i].targetEntityId = comp.targetEntityId;
			}
			return batch;
		});

		RegisterAllocator<AgentIntentComponent, AgentIntentComponent.BatchData>((ComponentArray<AgentIntentComponent> array) => {
			int count = array.Count;
			AgentIntentComponent.BatchData[] batch = new AgentIntentComponent.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
			}
			return batch;
		});

		// --- UIGroupComponent の登録 ---
		RegisterConverter<UIGroupComponent, UIGroupComponent.BatchData>((ComponentArray<UIGroupComponent> array) => {
			int count = array.Count;
			UIGroupComponent.BatchData[] batch = new UIGroupComponent.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
				batch[i].currentSelectedId = comp.currentSelectedId;
				batch[i].isFocused = (byte)(comp.isFocused ? 1 : 0);
				batch[i].isVisible = (byte)(comp.isVisible ? 1 : 0);
				batch[i].parentGroupId = comp.parentGroupId;
			}
			return batch;
		});

		RegisterAllocator<UIGroupComponent, UIGroupComponent.BatchData>((ComponentArray<UIGroupComponent> array) => {
			int count = array.Count;
			UIGroupComponent.BatchData[] batch = new UIGroupComponent.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
			}
			return batch;
		});

		// --- UIElementComponent の登録 ---
		RegisterConverter<UIElementComponent, UIElementComponent.BatchData>((ComponentArray<UIElementComponent> array) => {
			int count = array.Count;
			UIElementComponent.BatchData[] batch = new UIElementComponent.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
				batch[i].groupIdId = comp.groupIdId;
				batch[i].elementIndex = comp.elementIndex;
			}
			return batch;
		});

		// --- TextRenderer の登録 ---
		RegisterConverter<TextRenderer, TextRenderer.BatchData>((ComponentArray<TextRenderer> array) => {
			int count = array.Count;
			TextRenderer.BatchData[] batch = new TextRenderer.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				var batchData = comp.GetBatchData();
				batch[i].compId = comp.compId;
				batch[i].color = batchData.color;
				batch[i].textureSize = batchData.textureSize;
				batch[i].uvTransform = batchData.uvTransform;
			}
			return batch;
		});

		RegisterAllocator<TextRenderer, TextRenderer.BatchData>((ComponentArray<TextRenderer> array) => {
			int count = array.Count;
			TextRenderer.BatchData[] batch = new TextRenderer.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
			}
			return batch;
		});

		RegisterAllocator<UIElementComponent, UIElementComponent.BatchData>((ComponentArray<UIElementComponent> array) => {
			int count = array.Count;
			UIElementComponent.BatchData[] batch = new UIElementComponent.BatchData[count];
			for (int i = 0; i < count; i++) {
				var comp = array.Get(i);
				batch[i].compId = comp.compId;
			}
			return batch;
		});
	}


	// Converter を登録
	public static void RegisterConverter<TComponent, TData>(Func<ComponentArray<TComponent>, TData[]> converter)
		where TComponent : Component {
		converters[typeof(TComponent)] = (IComponentArray array) => {
			return converter((ComponentArray<TComponent>)array);
		};
	}

	// Allocator を登録 (変更点: Func<int, ...> から Func<ComponentArray<...>, ...> へ変更)
	public static void RegisterAllocator<TComponent, TData>(Func<ComponentArray<TComponent>, TData[]> allocator)
		where TComponent : Component {
		allocators[typeof(TComponent)] = (IComponentArray array) => {
			return allocator((ComponentArray<TComponent>)array);
		};
	}


	// 一括送信
	public static void SendAllBatches(ComponentCollection collection, string ecsGroupName) {
		// Debug.LogInfo("ComponentBatchManager.SendAllBatches: Start sending all batches.");
		// Debug.LogInfo($"ComponentBatchManager.SendAllBatches: Total converters registered: {converters.Count}.");

		foreach (var kv in converters) {
			if (!collection.TryGetArray(kv.Key, out IComponentArray array)) {
				// Debug.LogWarning($"ComponentBatchManager.SendAllBatches: ComponentArray for {kv.Key} not found.");
				continue;
			}

			// Debug.LogInfo($"ComponentBatchManager.SendAllBatches: Sending batch for {kv.Key}.");
			Array batch = kv.Value(array);
			InternalSetBatch(kv.Key, batch, batch.Length, ecsGroupName);
		}
	}

	// 一括受信
	public static void ReceiveAllBatches(ComponentCollection collection, string ecsGroupName) {
		foreach (var kv in allocators) {
			if (!collection.TryGetArray(kv.Key, out IComponentArray array)) {
				// Debug.LogWarning($"ComponentBatchManager.ReceiveAllBatches: ComponentArray for {kv.Key} not found.");
				continue;
			}

			int count = array.Count;
			if (count == 0) {
				// Debug.LogWarning($"ComponentBatchManager.ReceiveAllBatches: No components to receive for {kv.Key}.");
				continue;
			}

			// 変更点: 配列そのもの(array)を渡して、ID設定済みのBatch配列を受け取る
			Array batch = kv.Value(array);

			// Debug.LogInfo($"ComponentBatchManager.ReceiveAllBatches: Receiving batch for {kv.Key} with count {count}.");

			// batch内には既に compId/nativeHandle が入っているので、C++側で正しく処理可能
			InternalGetBatch(kv.Key, batch, count, ecsGroupName);

			ApplyBatch(kv.Key, batch, array);
		}
	}


	// データの適用
	static public void ApplyBatch(Type componentType, Array batch, IComponentArray array) {
		if (componentType == typeof(Transform)) {
			var transformArray = (ComponentArray<Transform>)array;
			var transformBatch = (Transform.BatchData[])batch;

			for (int i = 0; i < transformBatch.Length; i++) {
				var comp = transformArray.Get(i);
				comp.position = transformBatch[i].position;
				comp.rotate = transformBatch[i].rotate;
				comp.scale = transformBatch[i].scale;
				comp.matrix = transformBatch[i].matrix; // ワールド行列も毎フレーム反映（m30/31/32 がワールド座標）
			}
		} else if (componentType == typeof(MeshRenderer)) {
			var meshArray = (ComponentArray<MeshRenderer>)array;
			var meshBatch = (MeshRenderer.BatchData[])batch;

			for (int i = 0; i < meshBatch.Length; i++) {
				var comp = meshArray.Get(i);
				// MeshRendererの個別のデータがあればここで適用
			}
		} else if (componentType == typeof(SpriteRenderer)) {
			var spriteArray = (ComponentArray<SpriteRenderer>)array;
			var spriteBatch = (SpriteRenderer.BatchData[])batch;

			for (int i = 0; i < spriteBatch.Length; i++) {
				var comp = spriteArray.Get(i);
				comp.color = spriteBatch[i].color;
				comp.uvTransform = spriteBatch[i].uvTransform;
			}
		} else if (componentType == typeof(BoxCollider2D)) {
			var colliderArray = (ComponentArray<BoxCollider2D>)array;
			var colliderBatch = (BoxCollider2D.BatchData[])batch;
			int limit = Math.Min(colliderArray.Count, colliderBatch.Length);

			for (int i = 0; i < limit; i++) {
				var comp = colliderArray.Get(i);
				comp.ApplyBatchData(colliderBatch[i]);
			}
		} else if (componentType == typeof(AgentIntentComponent)) {
			var agentArray = (ComponentArray<AgentIntentComponent>)array;
			var agentBatch = (AgentIntentComponent.BatchData[])batch;
			int limit = Math.Min(agentArray.Count, agentBatch.Length);

			for (int i = 0; i < limit; i++) {
				var comp = agentArray.Get(i);
				comp.desiredMoveDirection = agentBatch[i].desiredMoveDirection;
				comp.desiredRotation = agentBatch[i].desiredRotation;
				comp.rotationSpeed = agentBatch[i].rotationSpeed;
				comp.maxSpeed = agentBatch[i].maxSpeed;
				comp.useDesiredRotation = agentBatch[i].useDesiredRotation != 0;
				comp.isAttacking = agentBatch[i].isAttacking != 0;
				comp.targetEntityId = agentBatch[i].targetEntityId;
			}
		} else if (componentType == typeof(UIGroupComponent)) {
			var uiGroupArray = (ComponentArray<UIGroupComponent>)array;
			var uiGroupBatch = (UIGroupComponent.BatchData[])batch;
			int limit = Math.Min(uiGroupArray.Count, uiGroupBatch.Length);

			for (int i = 0; i < limit; i++) {
				var comp = uiGroupArray.Get(i);
				comp.currentSelectedId = uiGroupBatch[i].currentSelectedId;
				comp.isFocused = uiGroupBatch[i].isFocused != 0;
				comp.isVisible = uiGroupBatch[i].isVisible != 0;
				comp.parentGroupId = uiGroupBatch[i].parentGroupId;
			}
		} else if (componentType == typeof(UIElementComponent)) {
			var uiElementArray = (ComponentArray<UIElementComponent>)array;
			var uiElementBatch = (UIElementComponent.BatchData[])batch;
			int limit = Math.Min(uiElementArray.Count, uiElementBatch.Length);

			for (int i = 0; i < limit; i++) {
				var comp = uiElementArray.Get(i);
				comp.groupIdId = uiElementBatch[i].groupIdId;
				comp.elementIndex = uiElementBatch[i].elementIndex;
			}
		} else if (componentType == typeof(TextRenderer)) {
			var textArray = (ComponentArray<TextRenderer>)array;
			var textBatch = (TextRenderer.BatchData[])batch;
			int limit = Math.Min(textArray.Count, textBatch.Length);

			for (int i = 0; i < limit; i++) {
				var comp = textArray.Get(i);
				comp.color = textBatch[i].color;
				comp.uvTransform = textBatch[i].uvTransform;
			}
		}
	}


	[MethodImpl(MethodImplOptions.InternalCall)]
	public static extern void InternalSetBatch(Type componentType, Array batch, int count, string ecsGroupName);

	[MethodImpl(MethodImplOptions.InternalCall)]
	public static extern void InternalGetBatch(Type componentType, Array batch_, int count, string ecsGroupName);
}