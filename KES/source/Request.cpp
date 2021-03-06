#include "Request.hpp"

static const int SLEEP_TIME = 1000;
static const int DONT_WAIT = 0;


// Возвращает NULL, если объект stopSignal указывает на необходимость остановки,
// либо указатель на память, которую в дальнейшем требуется удалить
Request* GetRequest(Stopper& stopSignal) {
	if (WAIT_OBJECT_0 == ::WaitForSingleObject(stopSignal.hStopEvent, DONT_WAIT)) {
		return nullptr;
	}

	std::unique_ptr < Request, decltype(&DeleteRequest) > pNewRequest(new Request(), &DeleteRequest);

	if (WAIT_OBJECT_0 == ::WaitForSingleObject(stopSignal.hStopEvent, SLEEP_TIME)) {// 4. Вызов GetRequest может не сразу возвращать задания
		pNewRequest.reset();
		return nullptr;
	}

	return pNewRequest.release();
}

// Обрабатывает запрос, но память не удаляет, завершает обработку досрочно, если
// объект stopSignal указывает на необходимость остановки
void ProcessRequest(Request* pRequest, Stopper& stopSignal) {
	if (!pRequest) {
		return;
	}

	if (WAIT_OBJECT_0 == ::WaitForSingleObject(stopSignal.hStopEvent, DONT_WAIT)) {
		return;
	}

	// Обработка запроса
	// ...

	::WaitForSingleObject(stopSignal.hStopEvent, SLEEP_TIME);                   // 5. Вызов ProcessRequest может не мгновенно обрабатывать задание
	return;
}

void DeleteRequest(Request* pRequest) {
	if (!pRequest) {
		return;
	}

	delete pRequest;
	return;
}
