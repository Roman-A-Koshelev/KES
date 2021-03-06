#include "Stopper.hpp"


Stopper::Stopper(const TCHAR* pStopEventName) {
	hStopEvent = ::CreateEvent(nullptr, TRUE, FALSE, pStopEventName);
	if (NULL == hStopEvent) {
		std::exception("Failed to create event.\n");
	}
}

Stopper::~Stopper() {
	if (0 != hStopEvent) {
		::CloseHandle(hStopEvent);
	}
}
