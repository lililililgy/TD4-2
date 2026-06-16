#pragma once

/// std
#include <vector>

/// //////////////////////////////////////////////////
/// ECSのSystemのinterfaceクラス
/// //////////////////////////////////////////////////
namespace ONEngine {

class ECSISystem {
public:

	virtual ~ECSISystem() {}

	virtual void OutsideOfRuntimeUpdate(class ECSGroup* /*ecs*/) {}
	virtual void RuntimeUpdate(class ECSGroup* ecs) = 0;

};



} /// ONEngine
