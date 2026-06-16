#include "SystemCollection.h"

using namespace ONEngine;

#include "Engine/ECS/EntityComponentSystem/ECSGroup.h"

void SystemCollection::AddSystem(std::unique_ptr<ECSISystem> system) {
	if (system) {
		systems_.emplace_back(std::move(system));
	}
}

void SystemCollection::OutsideOfRuntimeUpdate(ECSGroup* ecs) {
	for (auto& system : systems_) {
		if (system) {
			system->OutsideOfRuntimeUpdate(ecs);
		}
	}
}

void SystemCollection::RuntimeUpdate(ECSGroup* ecs) {
	for (auto& system : systems_) {
		if (system) {
			system->RuntimeUpdate(ecs);
		}
	}
}
