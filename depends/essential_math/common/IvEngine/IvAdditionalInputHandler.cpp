#include "IvAdditionalInputHandler.h"

AdditionalInputHandler* gAddionalInputHandler = nullptr;

AdditionalInputHandler * GetAdditionalInputHandler()
{
	return gAddionalInputHandler;
}

void SetAdditionalInputHandler(AdditionalInputHandler * handler)
{
	gAddionalInputHandler = handler;
}
