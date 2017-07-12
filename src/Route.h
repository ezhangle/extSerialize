#pragma once
#include "Common.h"

class CRoute
{
public:

	inline static void HandleRequest(char *output, int outputSize)
	{
		HandleRequest(output, outputSize, nullptr);
	};
	inline static void HandleRequest(char *output, int outputSize, const char *function)
	{
		HandleRequest(output, outputSize, function, nullptr, 0);
	}
	static int  HandleRequest(char *output, int outputSize, const char *function, const char **args, int argsCnt);
private:
	static void HandleRequestInternal(ERequestType eType, CRawBuffer * pRawBuf, CRawArgs * pRawArgs);
	inline static ERequestType GetRequestTypeEnum(uint8 id);
};