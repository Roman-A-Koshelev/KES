#pragma once

#ifndef REQUEST_HPP
#define REQUEST_HPP


#include "Stopper.hpp"

class Request {

};

// возвращает NULL, если объект stopSignal указывает на необходимость остановки,
// либо указатель на память, которую в дальнейшем требуется удалить
Request* GetRequest(Stopper& stopSignal);

// обрабатывает запрос, но память не удаляет, завершает обработку досрочно, если
// объект stopSignal указывает на необходимость остановки
void ProcessRequest(Request* request, Stopper& stopSignal);

void DeleteRequest(Request* request);


#endif REQUEST_HPP
