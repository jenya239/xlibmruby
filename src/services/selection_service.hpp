#pragma once
#include <cstddef>          // Для size_t
#include <string>           // Для std::string
#include <unordered_map>    // Для std::unordered_map
#include <utility>          // Для std::pair
#include "interfaces/iselection_service.hpp"  // Базовый интерфейс

class SelectionService : public ISelectionService {
private:
	struct Selection {
		size_t start;
		size_t end;
		std::string text;
	};

	std::unordered_map<std::string, Selection> selections_;
	std::string activeComponentId_;
	bool isSelecting_ = false;

public:
	void startSelection(const std::string& componentId, size_t position) override;
	void updateSelection(size_t position) override;
	void endSelection() override;
	void setComponentText(const std::string& componentId, const std::string& text);
	std::string getSelectedText() const override;
	bool hasSelection(const std::string& componentId) const override;
	std::pair<size_t, size_t> getSelectionRange() const override;

private:
	void clearSelections();
};