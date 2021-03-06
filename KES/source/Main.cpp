/**
Используя С++, STL, Win32 API и корректно реализовать следующую задачу:

Откуда-то дано:
class Request
{
};

// возвращает NULL, если объект stopSignal указывает на необходимость остановки,
// либо указатель на память, которую в дальнейшем требуется удалить
Request* GetRequest(Stopper stopSignal);

// обрабатывает запрос, но память не удаляет, завершает обработку досрочно, если
// объект stopSignal указывает на необходимость остановки
void ProcessRequest(Request* request, Stopper stopSignal);

void DeleteRequest(Request* request);

1.	С++ 11 использовать можно.
2.	Решение должно работать на Windows XP.
3.	Тип Stopper должен быть определён вами и должен представлять собой механизм досрочной остановки выполняемого действия(предполагается, что GetRequest и ProcessRequest будут его корректно использовать).
4.	Вызов GetRequest может не сразу возвращать задания.
5.	Вызов ProcessRequest может не мгновенно обрабатывать задание.
6.	Синхронизационная часть задачи должна использовать Win32 API.


Задача:

1)	Организовать в несколько потоков(переменное число, но не менее двух) приём запросов, для этого класть в одну очередь задания, возвращаемые функцией GetRequest.
2)	Запустить несколько обрабатывающих запросы потоков(переменное число, но не менее двух), которые должны обрабатывать поступающие из очереди задания с помощью ProcessRequest.
3)	Поработать в течение 30 секунд.
4)	Корректно остановить все потоки. Если остались необработанные задания, не обрабатывать их и корректно удалить.
5)	Завершить программу.
/**/

#include "Request.hpp"
#include "Stopper.hpp"
#include "ThreadProc.hpp"


static const int THREAD_GET_REQUEST_COUNT_DEF = 5;
static const int THREAD_PROCESS_REQUEST_COUNT_DEF = 5;
static const int WORK_TIME = 30000;


int main()
try {
	CONST HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	
	{
		TThreadProcParams threadProcParams;

		int threadGetRequestCount = std::thread::hardware_concurrency();
		if (0 == threadGetRequestCount) {
			threadGetRequestCount = THREAD_GET_REQUEST_COUNT_DEF;
		}

		int threadProcessRequestCount = std::thread::hardware_concurrency();
		if (0 == threadProcessRequestCount) {
			threadProcessRequestCount = THREAD_PROCESS_REQUEST_COUNT_DEF;
		}

		const int threadCount = threadGetRequestCount + threadProcessRequestCount;
		auto ThreadHandlesCloser = [&threadCount](HANDLE hThreads[]) {
			for (int threadNum = 0; threadNum < threadCount; ++threadNum) {
				::CloseHandle(hThreads[threadNum]);
			}
		};
		std::unique_ptr<HANDLE[], decltype(ThreadHandlesCloser)> pHThreads(new HANDLE[threadCount], ThreadHandlesCloser);

		// Создание GetRequest-потоков
		for (int threadNum = 0; threadNum < threadGetRequestCount; ++threadNum) {
			pHThreads.get()[threadNum] = (HANDLE) ::_beginthreadex(NULL, 0, &ThreadGetRequestProc, (void*)&threadProcParams, 0, NULL);
			if (NULL == pHThreads.get()[threadNum]) {
				std::exception("Failed to create thread.\n");
			}
		}

		// Создание ProcessRequest-потоков
		for (int threadNum = threadGetRequestCount; threadNum < threadCount; ++threadNum) {
			pHThreads.get()[threadNum] = (HANDLE) ::_beginthreadex(NULL, 0, &ThreadProcessRequestProc, (void*)&threadProcParams, 0, NULL);
			if (NULL == pHThreads.get()[threadNum]) {
				std::exception("Failed to create thread.\n");
			}
		}

		auto waitRes = ::WaitForMultipleObjects(threadCount, pHThreads.get(), TRUE, WORK_TIME);
		if (WAIT_TIMEOUT == waitRes) {
			::PulseEvent(threadProcParams.stopSignal.hStopEvent);
		}

		pHThreads.reset();
	}

	std::cout << "Successful!" << std::endl;

	::ExitProcess(0);
}
catch (const std::exception& ex) {
	std::cout << ex.what() << std::endl;
	::ExitProcess(0);
}
