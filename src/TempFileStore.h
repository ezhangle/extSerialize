#pragma once
#include "util/log.h"
#include "util/files.h"

#include <string>
#include <map>
#include <cstdio>
#include <fstream>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>

#define EXTS_ALLOWED_KEY_CHARACTERS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_- ."

//! Temporary quick solution to store strings in files
class CTempFileStore
{
	// In case we imlpement read-only soemtime
	static bool m_bWriteAccess;

	// Path to data store
	static std::string m_savePath;

	// Output size given via RVExtension call
	static size_t m_outSizeMax;

	// Map of key buffers
	static std::map<std::string, std::string> m_sBufMap;

	// No default ctor
	CTempFileStore();

public:

	//! InitDataStore( pathname )
	// pathname		-	Path to storage folder (read/write save location).
	//
	// Initializes the data store, checks permissions etc. Always use this before using any data store functions!
	//
	// TODO: inline
	static bool InitDataStore(const std::string &pathname, const size_t outSizeMax)
	{
		// Only run once
		static bool bRunOnceDone = false;
		static bool bResult = false;
		if (bRunOnceDone)
			return bResult;
		bRunOnceDone = true;

		m_outSizeMax = outSizeMax;

		static std::string cwd(util::GetCWD());

		// Sanity checks
		struct stat info;
		if (stat(pathname.c_str(), &info) != 0)
		{
			util::LogMessage("ERROR! (FileStore) Cannot access storage path '" + util::JoinPaths(cwd, pathname) + "'.");
		}
		else if (!(info.st_mode & S_IFDIR))
		{
			util::LogMessage("ERROR! (FileStore) Storage directory doesn't exist! (" + util::JoinPaths(cwd, pathname) + ").");
		}
		else
		{
			// store pathname
			m_savePath = pathname;

			// Check read/write permissions
			const bool bCanWrite = static_cast<bool>(std::ofstream("writetest.txt").put('x'));
			if (!bCanWrite)
			{
				util::LogMessage("ERROR! (FileStore) Cannot create/write files to '" + util::JoinPaths(cwd, pathname) + "'.");
			}
			else
			{
				if (std::remove("writetest.txt") != 0)
				{
					util::LogMessage("ERROR! (FileStore) Cannot create/write files to '" + util::JoinPaths(cwd, pathname) + "'.");
				}
				else
				{
					m_bWriteAccess = true;
				}
			}

			// Just in case we cant write, might as well read at minimum.
			bResult = true;
			return bResult;
		}
		bResult = false;
		return bResult;
	}

	//! LoadData ( key )
	// key	-	Unique name for the file to store the data
	//
	// Loads a file and creates a buffer with UID in case we need to. (output limit).
	// 
	static std::string LoadData(const std::string &key)
	{
		// Sanitize key characters
		if (key.find_first_not_of(EXTS_ALLOWED_KEY_CHARACTERS) != key.npos)
		{
			util::LogMessage("ERROR! (FileStore) LoadData(" + key + "). Key contains illegal characters, Allowed characters: " EXTS_ALLOWED_KEY_CHARACTERS);
			m_sBufMap[key] = "ERROR";
		}

		// create buffer if we need
		if (!HaveChunk(key))
		{
			// Create a new buffer
			m_sBufMap[key] = "";

			// Try to find data
			std::ifstream file(m_savePath + "/" + key);
			if (file.good())
			{
				std::stringstream buffer;
				buffer << file.rdbuf();
				m_sBufMap.at(key) = buffer.str();
			}
			else
			{
				util::LogMessage("ERROR! (FileStore) Could not find file '" + util::JoinPaths(util::JoinPaths(util::GetCWD(), m_savePath), key) + "'.");
				m_sBufMap[key] = "ERROR";
			}
		}

		return GetNextChunk(key);
	}

	//! SaveData ( key, data )
	// key		-	Unique name for the file to store the data
	// data		-	The data content to store inside the file
	//
	// Saves data to file_key file (clears first/overwrite)
	// 
	static bool SaveData(const std::string &key, const std::string &data)
	{
		// Sanitize key characters
		if (key.find_first_not_of(EXTS_ALLOWED_KEY_CHARACTERS) != key.npos)
		{
			util::LogMessage("ERROR! (FileStore) SaveData(" + key + "). Key contains illegal characters, Allowed characters: " EXTS_ALLOWED_KEY_CHARACTERS);
			return false;
		}

		// Open and truncate (delete contents)
		std::ofstream stream(m_savePath + "/" + key, std::ios::out | std::ios::trunc);

		if (!stream.good())
		{
			util::LogMessage("ERROR! (FileStore) SaveData(" + key + "). Cannot open/create file: '" + util::JoinPaths(util::JoinPaths(util::GetCWD(), m_savePath), key) + "'.");
			return false;
		}

		stream << data;
		stream.close();

		return true;
	}

	static inline bool HaveChunk(const std::string &key)
	{
		return m_sBufMap.count(key) >= 1;
	}

	static inline bool HasWriteAccess()
	{
		return m_bWriteAccess;
	}

	static inline std::string GetStorePath()
	{
		return m_savePath;
	}

private:

	static std::string GetNextChunk(const std::string& key)
	{
		std::string chunk;

		// Sanity
		if (!HaveChunk(key))
		{
			util::LogMessage("ERROR! (FileStore) BufferCopySubtract Assert: '" + key + "' NOT FOUND.");
			chunk = "ERROR";
		}
		else
		{
			if (m_sBufMap.at(key).length() == 0)
			{
				m_sBufMap.erase(key);
				chunk = "ENDOFDATA";
			}
			else
			{
				// get next chunk
				chunk = m_sBufMap.at(key).substr(0, m_outSizeMax - 1);

				// remove chunk from internal buffer
				m_sBufMap.at(key).erase(0, m_outSizeMax - 1);
			}
		}

		return chunk;
	}
};