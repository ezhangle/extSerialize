#pragma once

#include <string>

// typedefs
typedef unsigned	__int8		uint8;
typedef unsigned	__int16		uint16;
typedef unsigned	__int32		uint32;
typedef unsigned	__int64		uint64;
typedef				__int8		int8;
typedef				__int16		int16;
typedef				__int32		int32;
typedef				__int64		int64;

// Type of request (Global - add more here first)
enum ERequestType : uint16
{
	ERequestType_Version	= 0,
	ERequestType_Debug		= 1,

	ERequestType_Read		= 2,
	ERequestType_Write		= 3,

	ERequestType_Unknown	= 255 // <- Always last in list
};

// Current state of the request
enum ERequestStatus : uint16
{
	ERequestStatus_Begin,
	ERequestStatus_Waiting,
	ERequestStatus_Processing,
	ERequestStatus_Done
};

// Base request struct for all types of requests
struct SRequest
{
	const ERequestType requestType;
	const char identifier[33];
	const char * data;

	ERequestStatus requestStatus;
};

// Contains raw buffer access
class CRawBuffer
{
	char * m_szBuf;
	const size_t m_maxLength;
	size_t m_curLength;

	CRawBuffer();
public:
	CRawBuffer(char * szBuf, size_t maxLen)
		: m_szBuf(szBuf), m_maxLength(maxLen), m_curLength(std::strlen(szBuf))
	{};
	~CRawBuffer() {};

	
};

// Contains raw argument access
class CRawArgs
{
	const char ** m_args;
	const size_t m_numArgs;

	CRawArgs();
public:
	CRawArgs(const char ** args, const size_t numArgs) :
		m_args(m_args),
		m_numArgs(numArgs)
	{};
	~CRawArgs() {};


};