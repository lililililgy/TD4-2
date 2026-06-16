using System;
using System.Collections.Generic;

static public class ObjectPool {
	
	// オブジェクトの定義
	static public Pool<Vector2>    vector2    = new Pool<Vector2>(() => new Vector2(0f, 0f),         resetAction: v => { v.x = 0; v.y = 0; } ,                 500, 1000);
	static public Pool<Vector2Int> vector2Int = new Pool<Vector2Int>(() => new Vector2Int(0, 0),     resetAction: v => { v.x = 0; v.y = 0; },                  500, 1000);
	static public Pool<Vector3>    vector3    = new Pool<Vector3>(() => new Vector3(0f, 0f, 0f),     resetAction: v => { v.x = 0; v.y = 0; v.z = 0;},          500, 1000);
	static public Pool<Vector4>    vector4    = new Pool<Vector4>(() => new Vector4(0f, 0f, 0f, 0f), resetAction: v => { v.x = 0; v.y = 0; v.z = 0; v.w = 0;}, 500, 1000);


	public class Pool<T> {
		private readonly Stack<T> pool = new Stack<T>();
		private readonly Func<T> factory;
		private readonly Action<T> resetAction;
		private readonly int maxSize;

		public Pool(Func<T> factory, Action<T> resetAction, int initialCapacity = 10, int maxSize = 100) {
			this.factory = factory;
			this.resetAction = resetAction;
			this.maxSize = maxSize;
			for (int i = 0; i < initialCapacity; i++) {
				pool.Push(factory());
			}
		}

		public T Get() => pool.Count > 0 ? pool.Pop() : factory();

		public void Release(T obj) {
			if (pool.Count < maxSize) pool.Push(obj);
		}

		public int Count => pool.Count;
	}
}