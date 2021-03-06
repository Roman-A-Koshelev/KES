#pragma once

#ifndef THREAD_PROC_HPP
#define THREAD_PROC_HPP

#include "Stopper.hpp"


struct TThreadProcParams {
	TThreadProcParams(const TCHAR* pMutexName = nullptr, const TCHAR* pStopEventName = nullptr, const TCHAR* pNotEmptyQueueEventName = nullptr);
	TThreadProcParams(const TThreadProcParams&) = delete;
	TThreadProcParams(const TThreadProcParams&&) = delete;
	TThreadProcParams& operator= (const TThreadProcParams&) = delete;
	TThreadProcParams& operator= (const TThreadProcParams&&) = delete;
	~TThreadProcParams();

	HANDLE hMutex;
	HANDLE hNotEmptyQueueEvent;
	Stopper stopSignal;
};

unsigned __stdcall ThreadGetRequestProc(void* lpParam);

unsigned __stdcall ThreadProcessRequestProc(void* lpParam);


#endif THREAD_PROC_HPP
