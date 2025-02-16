#include "services/selection_service.hpp"
#include <cstddef>
#include <string>

void SelectionService::startSelection(const std::string& componentId, size_t position) {
    activeComponentId_ = componentId;
    selections_[componentId] = {position, position, ""};
    isSelecting_ = true;
}

void SelectionService::updateSelection(size_t position) {
	if (!isSelecting_ || activeComponentId_.empty()) return;
	
	auto& selection = selections_[activeComponentId_];
	selection.end = position;
}

void SelectionService::endSelection() {
	isSelecting_ = false;
}

void SelectionService::setComponentText(const std::string& componentId, const std::string& text) {
	if (selections_.count(componentId)) {
		 auto& selection = selections_[componentId];
		 selection.text = text;
	}
}

std::string SelectionService::getSelectedText() const {
	if (!activeComponentId_.empty() && selections_.count(activeComponentId_)) {
		 const auto& selection = selections_.at(activeComponentId_);
		 size_t start = std::min(selection.start, selection.end);
		 size_t end = std::max(selection.start, selection.end);
		 return selection.text.substr(start, end - start);
	}
	return "";
}

bool SelectionService::hasSelection(const std::string& componentId) const {
	return selections_.count(componentId) && 
			 selections_.at(componentId).start != selections_.at(componentId).end;
}

std::pair<size_t, size_t> SelectionService::getSelectionRange() const {
	if (!activeComponentId_.empty() && selections_.count(activeComponentId_)) {
		 const auto& selection = selections_.at(activeComponentId_);
		 return {std::min(selection.start, selection.end), 
				  std::max(selection.start, selection.end)};
	}
	return {0, 0};
}

void SelectionService::clearSelections() {
	selections_.clear();
}