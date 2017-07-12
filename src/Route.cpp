#include "Route.h"

// All requests are parsed and initially handled here
int CRoute::HandleRequest(char *output, int outputSize, const char *function, const char **args, int argsCnt)
{
	CRawBuffer pBuf(output, outputSize);

	ERequestType eType = (function == nullptr || std::strlen(function) == 0)? 
		ERequestType_Version : GetRequestTypeEnum(function[0]);



	return -1;
}

// After parsing the request data, this method acts on the request itself.
void CRoute::HandleRequestInternal(ERequestType eType, CRawBuffer * pRawBuf, CRawArgs * pRawArgs)
{



}

ERequestType CRoute::GetRequestTypeEnum(uint8 id)
{
	switch (id)
	{
	case ERequestType_Debug:
		{ return ERequestType_Debug; }

	case ERequestType_Read:
		{ return ERequestType_Read; }

	case ERequestType_Version:
		{ return ERequestType_Version; }

	case ERequestType_Write:
		{ return ERequestType_Write; }

	default:
		{ return ERequestType_Unknown; }
	}
}