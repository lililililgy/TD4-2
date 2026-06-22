#pragma once

#include "Engine/Editor/Views/EditorViewCollection.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <imgui-node-editor/imgui_node_editor.h>
#include "Engine/Asset/Guid/Guid.h"

namespace ONEngine {
class EntityComponentSystem;
namespace Asset {
class AssetCollection;
}
}

namespace Editor {

namespace ed = ax::NodeEditor;

class UIEditorWindow : public IEditorWindow {
public:
	UIEditorWindow(
		const std::string& title,
		ONEngine::EntityComponentSystem* ecs,
		ONEngine::Asset::AssetCollection* assetCollection,
		class InspectorWindow* inspector
	);
	~UIEditorWindow() override;

	void ShowImGui() override;

private:
	enum class PinKind { Input, Output };
	enum class NodeType { Group, Element };

	struct Node;

	struct Pin {
		ed::PinId id;
		std::string name;
		PinKind kind;
		Node* node;
		std::vector<int32_t> keyCodes; // 出力ピンが表すキーコード群

		Pin(int id, const std::string& name, PinKind kind, const std::vector<int32_t>& keys = {})
			: id(id), name(name), kind(kind), node(nullptr), keyCodes(keys) {}
	};

	struct Node {
		ed::NodeId id;
		NodeType type;
		std::string name;
		std::string elementId;
		int32_t elementIndex = 0;
		bool isFocused = false;
		bool isVisible = true;
		ONEngine::Guid guid; // GameEntity の GUID と一致させる

		std::vector<Pin> inputs;
		std::vector<Pin> outputs;

		Node(int id, NodeType t, const std::string& n, const ONEngine::Guid& g)
			: id(id), type(t), name(n), guid(g) {}
	};

	struct Link {
		ed::LinkId id;
		ed::PinId startPinId;
		ed::PinId endPinId;

		Link(ed::LinkId id, ed::PinId start, ed::PinId end)
			: id(id), startPinId(start), endPinId(end) {}
	};

private:
	void DrawNodeEditor();
	void DrawToolbar();
	void DrawFileDialogs();
	void LoadPrefab(const std::string& filepath);
	void SavePrefab(const std::string& filepath);
	void SyncWithEngineScene();

private:
	ONEngine::EntityComponentSystem* pEcs_ = nullptr;
	ONEngine::Asset::AssetCollection* pAssetCollection_ = nullptr;
	InspectorWindow* pInspector_ = nullptr;

	ed::EditorContext* m_Editor = nullptr;

	std::vector<std::unique_ptr<Node>> m_Nodes;
	std::vector<Link> m_Links;
	int m_NextId = 1;

	std::string m_CurrentPrefabPath = "";
	std::string windowName_;
	std::unordered_map<unsigned long long, std::vector<bool>> m_SelectedKeysMap;

	enum class KeyFilter {
		All,
		Keyboard,
		Controller,
		Mouse
	};
	KeyFilter m_CurrentKeyFilter = KeyFilter::All;
};

} /// namespace Editor
