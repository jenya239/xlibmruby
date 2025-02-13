#pragma once

class IWindowService {
public:
	virtual ~IWindowService() = default;
	virtual void run() = 0;
};