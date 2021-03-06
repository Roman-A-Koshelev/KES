#include "Request.hpp"
#include "ThreadProc.hpp"


static const int ITERATION_COUNT = 100;

// Очередь запросов
static std::queue<Request*> quRequests;


auto HandleCloser = [](HANDLE* pHandle) {
	::CloseHandle(*pHandle);
};

auto MutexReleaser = [](HANDLE* pHandle) {
	::ReleaseMutex(*pHandle);
};


TThreadProcParams::TThreadProcParams(const TCHAR* pMutexName, const TCHAR* pStopEventName, const TCHAR* pNotEmptyQueueEventName)
	: stopSignal(pStopEventName)
{
	hMutex = ::CreateMutex(NULL, FALSE, NULL);
	if (NULL == hMutex) {
		std::exception("Failed to create queueMutex.\n");
	}
	std::unique_ptr<HANDLE, decltype(HandleCloser)> pQueueMutex(&hMutex, HandleCloser);

	hNotEmptyQueueEvent = ::CreateEvent(nullptr, TRUE, FALSE, pNotEmptyQueueEventName);
	if (NULL == hNotEmptyQueueEvent) {
		std::exception("Failed to create notEmptyQueueEvent.\n");
	}

	pQueueMutex.release();
}

TThreadProcParams::~TThreadProcParams() {
	::CloseHandle(hMutex);
	::CloseHandle(hNotEmptyQueueEvent);
}



unsigned __stdcall ThreadGetRequestProc(void* lpParam) {
	TThreadProcParams* pParams = static_cast<TThreadProcParams*>(lpParam);
	HANDLE hSyncObjects[] = { pParams->stopSignal.hStopEvent, pParams->hMutex };
	const size_t hSyncObjectCount = sizeof(hSyncObjects) / sizeof(hSyncObjects[0]);

	DWORD waitRes;
	for (int i = 0; i < ITERATION_COUNT; ++i) {
		std::unique_ptr<Request, decltype(&DeleteRequest)> pRequest(GetRequest(pParams->stopSignal), &DeleteRequest);

		waitRes = ::WaitForMultipleObjects(hSyncObjectCount, hSyncObjects, FALSE, INFINITE);
		if (WAIT_OBJECT_0 == waitRes) {                                         // Остановка потока
			::_endthreadex(0);
			return 0;
		}

		std::unique_ptr<HANDLE, decltype(MutexReleaser)> pMutex(&pParams->hMutex, MutexReleaser);
		quRequests.push(pRequest.release());
		::SetEvent(pParams->hNotEmptyQueueEvent);
		pMutex.reset();
	}

	::_endthreadex(0);
	return 0;
}

unsigned __stdcall ThreadProcessRequestProc(void* lpParam) {
	TThreadProcParams* pParams = static_cast<TThreadProcParams*>(lpParam);
	std::vector<HANDLE> hNotEmptyQueueSyncObjects { pParams->stopSignal.hStopEvent, pParams->hNotEmptyQueueEvent };
	std::vector<HANDLE> hQueueSyncObjects { pParams->stopSignal.hStopEvent, pParams->hMutex };

	DWORD waitRes;
	for (int i = 0; i < ITERATION_COUNT; ++i) {
		waitRes = ::WaitForMultipleObjects((DWORD)hNotEmptyQueueSyncObjects.size(), &hNotEmptyQueueSyncObjects[0], FALSE, INFINITE);
		if (WAIT_OBJECT_0 == waitRes) {                                         // Остановка потока
			::_endthreadex(0);
			return 0;
		}

		waitRes = ::WaitForMultipleObjects((DWORD)hQueueSyncObjects.size(), &hQueueSyncObjects[0], FALSE, INFINITE);
		if (WAIT_OBJECT_0 == waitRes) {                                         // Остановка потока
			::_endthreadex(0);
			return 0;
		}
		std::unique_ptr<HANDLE, decltype(MutexReleaser)> pMutex(&pParams->hMutex, MutexReleaser);

		if (quRequests.empty()) {                                               // Пока мы ждали мьютекса очередь опустела
			::ResetEvent(pParams->hNotEmptyQueueEvent);
			pMutex.reset();
			continue;
		}

		std::unique_ptr<Request, decltype(&DeleteRequest)> pRequest(quRequests.front(), &DeleteRequest);
		quRequests.pop();
		
		if (quRequests.empty()) {                                               // Мы забрали последний элемент из очереди
			::ResetEvent(pParams->hNotEmptyQueueEvent);
		}
		pMutex.reset();

		ProcessRequest(pRequest.get(), pParams->stopSignal);
		pRequest.reset();
	}

	::_endthreadex(0);
	return 0;
}
