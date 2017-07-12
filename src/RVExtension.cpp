#// WINDOWS HEADERS
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

// Exported symbols
extern "C"
{
	//--- Engine called on extension load 
	__declspec (dllexport) void __stdcall RVExtensionVersion(char *output, int outputSize);
	//--- STRING callExtension STRING
	__declspec (dllexport) void __stdcall RVExtension(char *output, int outputSize, const char *function);
	//--- STRING callExtension ARRAY
	__declspec (dllexport) int  __stdcall RVExtensionArgs(char *output, int outputSize, const char *function, const char **args, int argsCnt);
}

// DLL MAIN
BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID)
{
	return TRUE;
}
#pragma warning(disable:4996)

#include <iostream>
#include <string>
#include "Config.h"
#include "TempFileStore.h"
#include "util\log.h"
#include "Common.h"

// Statics
std::string CExtConfig::m_sSaveFolder = "";
std::string CExtConfig::m_sPath = "";
std::string CExtConfig::m_sLastError = "";

bool CTempFileStore::m_bWriteAccess = false;
std::string CTempFileStore::m_savePath = "";
size_t CTempFileStore::m_outSizeMax = 0;
std::map<std::string, std::string> CTempFileStore::m_sBufMap = {};

//#include "Route.h"

// In curernt working directory
#define EXTS_CONFIG_PATH "extSerialize.extcfg"

#define EXTS_STR_VERSION "0.0.0.1"
#define EXTS_STR_ERROR "ERROR"
#define EXTS_STR_SUCESS "SUCCESS"

// TODO: inline
bool OneTimeInit(int outputSize)
{
	// One-Time inits
	if (CExtConfig::TryLoad(EXTS_CONFIG_PATH))
		return CTempFileStore::InitDataStore(CExtConfig::GetSaveFolder(), outputSize);

	return false;
}

extern "C"
{
	//--- Extension version information shown in .rpt file
	void __stdcall RVExtensionVersion(char *szOutput, int outputSize)
	{
		if (!OneTimeInit(outputSize))
		{
			std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
			util::LogMessage("ERROR! (RVExtension) OneTimeInit() Failed!");
			return;
		}

		/*
		//--- max outputSize is 32 bytes
		strncpy_s(output, outputSize, SEXT_VERSION, _TRUNCATE);
		*/

		//CRoute::HandleRequest(output, outputSize);

		std::strncpy(szOutput, EXTS_STR_VERSION, std::strlen(EXTS_STR_VERSION) + 1);
	}

	//--- name callExtension function
	void __stdcall RVExtension(char *szOutput, int outputSize, const char *szFunction)
	{
		///CRoute::HandleRequest(output, outputSize, function);
		if (!OneTimeInit(outputSize))
		{
			std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
			util::LogMessage("ERROR! (RVExtension) OneTimeInit() Failed!");
			return;
		}

		// Take the raw string and copy it
		std::string function(szFunction);

		// ==== SANITY ====

		// No function - give version
		if (function.length() == 0)
		{
			std::strncpy(szOutput, EXTS_STR_VERSION, std::strlen(EXTS_STR_VERSION) + 1);
			util::LogMessage("WARNING! (RVExtension) Called with empty data.");
			return;
		}

		// Error - too short
		if (function.length() < 3)
		{
			std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
			util::LogMessage("ERROR! (RVExtension) Incorrect call, missing method id - too short. (" + function + ").");
			return;
		}

		// Error - Illegal characters in method id
		const std::string method(function.substr(0, 3));
		if (method.find_first_not_of("0123456789", 0) != std::string::npos)
		{
			std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
			util::LogMessage("ERROR! (RVExtension) Incorrect call, method id must be numbers only! (" + function + ").");
			return;
		}

		// Remove the method id
		function.erase(0, 3);

		// Handle the method ID
		const auto methodID = static_cast<uint16>(std::atoi(method.c_str())); // str to int - sanitized above
		switch (methodID)
		{
		default:
		{
			std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
			util::LogMessage("ERROR! (RVExtension) Incorrect call, method id not implemented! (" + function + ").");
			return;

		} break;

		case ERequestType_Read:
		{
			// Error - Expected the separator for the method
			if (function.length() == 0 || function.front() != ':')
			{
				std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
				util::LogMessage("ERROR! (RVExtension) (Read) Incorrect call, expected method separator! (" + function + ").");
				return;
			}

			// Error - No key specified
			if (function.length() < 2)
			{
				std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
				util::LogMessage("ERROR! (RVExtension) (Read) Incorrect call, no key specified after method separator! (" + function + ").");
				return;
			}

			// remove method separator character
			function.erase(0, 1);

			// Error - Illegal characters in key
			if (method.find_first_not_of(EXTS_ALLOWED_KEY_CHARACTERS, 0) != std::string::npos)
			{
				std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
				util::LogMessage("ERROR! (RVExtension) (Read) Key contains illegal characters (" + function + "), Allowed characters: " EXTS_ALLOWED_KEY_CHARACTERS ".");
				return;
			}

			// Attempt to return a chunk of data
			std::string chunk = CTempFileStore::LoadData(function);

			// Our chunk is too big? hmm, only cause is max changed between calls?
			if (chunk.length() >= outputSize)
			{
				std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
				util::LogMessage("ERROR! (RVExtension) (Read) Unexpected Error! Chunk bigger than max output size! (" + function + "). Chunk: (" + chunk + ").");
				return;
			}

			// copy chunk to out buffer
			std::strncpy(szOutput, chunk.c_str(), chunk.length() + 1);
			return;

		} break;

		case ERequestType_Write:
		{
			// Error - Expected the separator for the method
			if (function.length() == 0 || function.front() != ':')
			{
				std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
				util::LogMessage("ERROR! (RVExtension) (Write) Incorrect call, expected method separator! (" + function + ").");
				return;
			}

			// Error - No key specified
			if (function.length() < 2)
			{
				std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
				util::LogMessage("ERROR! (RVExtension) (Write) Incorrect call, no key specified after method separator! (" + function + ").");
				return;
			}

			// remove method separator character
			function.erase(0, 1);

			// Find key/data separator
			auto sepPos = function.find_first_of(':', 0);

			// Error - No separator
			if (sepPos == function.npos)
			{
				std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
				util::LogMessage("ERROR! (RVExtension) (Write) Incorrect call, no key/data separator! No Data?  (" + function + ").");
				return;
			}

			const std::string key = function.substr(0, sepPos);

			// Error - Illegal characters in key
			if (key.find_first_not_of(EXTS_ALLOWED_KEY_CHARACTERS, 0) != std::string::npos)
			{
				std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
				util::LogMessage("ERROR! (RVExtension) (Write) Key contains illegal characters (" + key + "), Allowed characters: " EXTS_ALLOWED_KEY_CHARACTERS ".");
				return;
			}

			const std::string data = function.substr(++sepPos);

			// Save failed for some reason
			if (!CTempFileStore::SaveData(key, data))
			{
				std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
				util::LogMessage("ERROR! (RVExtension) (Write) Save Failed! Key: '" + key + "'. Data: (" + data + ").");
				return;
			}

			// copy success to out buffer
			std::strncpy(szOutput, EXTS_STR_SUCESS, std::strlen(EXTS_STR_SUCESS) + 1);
			return;

		} break;
		}
	}

	//--- name callExtension [function, args]
	int __stdcall RVExtensionArgs(char *szOutput, int outputSize, const char *function, const char **args, int argsCnt)
	{
		if (!OneTimeInit(outputSize))
		{
			std::strncpy(szOutput, EXTS_STR_ERROR, std::strlen(EXTS_STR_ERROR) + 1);
			util::LogMessage("ERROR! (RVExtension) OneTimeInit() Failed!");
			return 0;
		}

		/*
		if (strcmp(function, "fnc1") == 0)
		{
			//--- Manually assemble output array
			int i = 0;
			std::string str = "[";

			//--- Each argument can be accessed via args[n]
			if (argsCnt > 0)
				str += args[i++];

			while (i < argsCnt)
			{
				str += ",";
				str += args[i++];
			}

			str += "]";

			//--- Extension result
			strncpy_s(output, outputSize, str.c_str(), _TRUNCATE);

			//--- Extension return code
			return 100;
		}

		else if (strcmp(function, "fnc2") == 0)
		{
			//--- Parse args into vector
			std::vector<std::string> vec(args, std::next(args, argsCnt));

			std::ostringstream oss;
			if (!vec.empty())
			{
				//--- Assemble output array
				std::copy(vec.begin(), vec.end() - 1, std::ostream_iterator<std::string>(oss, ","));
				oss << vec.back();
			}

			//--- Extension result
			strncpy_s(output, outputSize, ("[" + oss.str() + "]").c_str(), _TRUNCATE);

			//--- Extension return code
			return 200;
		}

		else
		{
			strncpy_s(output, outputSize, "Avaliable Functions: fnc1, fnc2", outputSize - 1);
			return -1;
		}
		*/

		//return CRoute::HandleRequest(output, outputSize, function, args, argsCnt);

		std::string testOutput = "RVExtensionArgs()\nFunction: '''" + std::string(function) + "'''\nNum Args: " + std::to_string(argsCnt);
		std::strncpy(szOutput, testOutput.c_str(), testOutput.length() + 1);

		return 100;
	}
}