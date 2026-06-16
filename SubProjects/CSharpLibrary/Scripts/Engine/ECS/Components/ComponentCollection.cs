using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


public interface IComponentArray {
	int Count {
		get;
	}
}

public class ComponentArray<T> : IComponentArray where T : Component {
	public List<T> components = new List<T>();

	public int Count => components.Count;

	public T Get(int index) {
		return components[index];
	}
}

public class ComponentCollection {
	Dictionary<Type, IComponentArray> arrays_ = new Dictionary<Type, IComponentArray>();


	public void AddComponent<T>(T component) where T : Component {
		if (!arrays_.ContainsKey(typeof(T))) {
			arrays_[typeof(T)] = new ComponentArray<T>();
		}

		ComponentArray<T> array = (ComponentArray<T>)arrays_[typeof(T)];
		array.components.Add(component);
	}

	public void RemoveComponent<T>(T component) where T : Component {
		if (!arrays_.ContainsKey(typeof(T))) {
			return;
		}

		ComponentArray<T> array = (ComponentArray<T>)arrays_[typeof(T)];
		array.components.Remove(component);
	}

	public bool TryGetArray(Type type, out IComponentArray array) {
		return arrays_.TryGetValue(type, out array);
	}

}