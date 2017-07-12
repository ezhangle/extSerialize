#pragma once
#include <fstream>
#include <string>

#define EXTS_LOG_PATH "extSerialize.log"

namespace util
{

	void LogMessage(const std::string &message)
	{
		static bool bFatalError = false;
		if (bFatalError)
			return;

		static const char filename[] = EXTS_LOG_PATH;
		static std::fstream stream;

		// Check if the log file has already been opened
		if (!stream || !stream.is_open())
		{
			// Try to open (read if exists)
			stream.open(filename, std::fstream::in | std::fstream::out | std::fstream::app);
			if (!stream)
			{
				// Try to create
				stream.open(filename, std::fstream::in | std::fstream::out | std::fstream::trunc);
				if (!stream)
				{
					bFatalError = true;
					return; // TODO: very bad situation (lost error messages), should somehow send event in output back to sqf
				}
			}
		}

		// Log message to file
		stream << message << std::endl;
	}

}