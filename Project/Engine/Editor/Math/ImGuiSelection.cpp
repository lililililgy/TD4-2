#include "ImGuiSelection.h"

/// engine
#include "Engine/Asset/Collection/AssetCollection.h"

using namespace Editor;

namespace {

	/// ----- ImGuiSelection ----- ///
	std::unordered_set<ONEngine::Guid> gSelectedObjects;
	ONEngine::Guid gLastSelectedObject = ONEngine::Guid::kInvalid;
	SelectionType gSelectionType = SelectionType::None;

	/// ----- ImGuiInfo ----- ///
	std::string gInfo;

} /// namespace

 
const std::unordered_set<ONEngine::Guid>& ImGuiSelection::GetSelectedObjects() {
	return gSelectedObjects;
}

const ONEngine::Guid& ImGuiSelection::GetLastSelectedObject() {
	return gLastSelectedObject;
}

void ImGuiSelection::SetSelectedObject(const ONEngine::Guid& guid, SelectionType type) {
	gSelectedObjects.clear();
	gSelectedObjects.insert(guid);
	gLastSelectedObject = guid;
	gSelectionType = type;
}

void ImGuiSelection::AddSelectedObject(const ONEngine::Guid& guid, SelectionType type) {
	gSelectedObjects.insert(guid);
	gLastSelectedObject = guid;
	gSelectionType = type;
}

void ImGuiSelection::RemoveSelectedObject(const ONEngine::Guid& guid) {
	gSelectedObjects.erase(guid);
	if (gLastSelectedObject == guid) {
		gLastSelectedObject = ONEngine::Guid::kInvalid;
	}
	if (gSelectedObjects.empty()) {
		gSelectionType = SelectionType::None;
	}
}

void ImGuiSelection::ClearSelection() {
	gSelectedObjects.clear();
	gLastSelectedObject = ONEngine::Guid::kInvalid;
	gSelectionType = SelectionType::None;
}

bool ImGuiSelection::IsSelected(const ONEngine::Guid& guid) {
	return gSelectedObjects.contains(guid);
}

SelectionType ImGuiSelection::GetSelectionType() {
	return gSelectionType;
}



const std::string& ImGuiInfo::GetInfo() {
	return gInfo;
}

void ImGuiInfo::SetInfo(const std::string& info) {
	gInfo = info;
}
