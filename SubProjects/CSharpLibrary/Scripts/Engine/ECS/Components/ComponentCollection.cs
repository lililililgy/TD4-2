using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


public interface IComponentArray
{
	int Count
	{
		get;
	}
	void Clear();
}

public class ComponentArray<T> : IComponentArray where T : Component
{
	public List<T> components = new List<T>();

	public int Count => components.Count;

	public T Get(int _index)
	{
		return components[_index];
	}

	public void Clear()
	{
		components.Clear();
	}
}

public class ComponentCollection
{
	Dictionary<Type, IComponentArray> arrays_ = new Dictionary<Type, IComponentArray>();


	public void AddComponent<T>(T _component) where T : Component
	{
		if (!arrays_.ContainsKey(typeof(T)))
		{
			arrays_[typeof(T)] = new ComponentArray<T>();
		}

		ComponentArray<T> array = (ComponentArray<T>)arrays_[typeof(T)];
		array.components.Add(_component);
	}

	public void RemoveComponent<T>(T _component) where T : Component
	{
		if (!arrays_.ContainsKey(typeof(T)))
		{
			return;
		}

		ComponentArray<T> array = (ComponentArray<T>)arrays_[typeof(T)];
		array.components.Remove(_component);
	}

	public void Clear()
	{
		foreach (var array in arrays_.Values)
		{
			array.Clear();
		}
	}

	public bool TryGetArray(Type _type, out IComponentArray _array)
	{
		return arrays_.TryGetValue(_type, out _array);
	}

	public ComponentArray<T> GetArray<T>() where T : Component
	{
		if (!arrays_.ContainsKey(typeof(T)))
		{
			arrays_[typeof(T)] = new ComponentArray<T>();
		}
		return (ComponentArray<T>)arrays_[typeof(T)];
	}

}
