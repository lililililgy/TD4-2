#include "IComponent.h"

using namespace ONEngine;

void IComponent::Reset() {}

void IComponent::SetOwner(GameEntity* owner) {
	owner_ = owner;
}

GameEntity* IComponent::GetOwner() const {
	return owner_;
}

