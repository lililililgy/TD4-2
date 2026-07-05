#pragma once

#include "IEditCommand.h"
#include <functional>

namespace Editor {

class LambdaCommand : public IEditCommand {
public:
	LambdaCommand(std::function<void()> executeAction, std::function<void()> undoAction)
		: executeAction_(executeAction), undoAction_(undoAction) {}

	~LambdaCommand() override = default;

	EDITOR_STATE Execute() override {
		if (executeAction_) {
			executeAction_();
			return EDITOR_STATE_FINISH;
		}
		return EDITOR_STATE_FAILED;
	}

	EDITOR_STATE Undo() override {
		if (undoAction_) {
			undoAction_();
			return EDITOR_STATE_FINISH;
		}
		return EDITOR_STATE_FAILED;
	}

private:
	std::function<void()> executeAction_;
	std::function<void()> undoAction_;
};

} // namespace Editor
