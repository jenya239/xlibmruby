#pragma once
#include <string>
#include <utility>  // Для std::pair

class ISelectionService {
public:
	virtual ~ISelectionService() = default;
	
	virtual void startSelection(const std::string& componentId, size_t position) = 0;
	virtual void updateSelection(size_t position) = 0;
	virtual void endSelection() = 0;
	virtual std::string getSelectedText() const = 0;
	virtual bool hasSelection(const std::string& componentId) const = 0;
	virtual std::pair<size_t, size_t> getSelectionRange() const = 0;
};