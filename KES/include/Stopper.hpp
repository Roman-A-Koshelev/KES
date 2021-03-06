#pragma once

#ifndef STOPPER_HPP
#define STOPPER_HPP


class Stopper {
public:
	Stopper(const TCHAR* pStopEventName = nullptr);
	Stopper(const Stopper&) = delete;
	Stopper(const Stopper&&) = delete;
	Stopper& operator= (const Stopper&) = delete;
	Stopper& operator= (const Stopper&&) = delete;
	~Stopper();

	HANDLE hStopEvent;
};


#endif STOPPER_HPP
