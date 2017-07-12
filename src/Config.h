#pragma once

#include "util\trim.h"
#include "util\log.h"

#include <string>
#include <map>
#include <fstream>

// Simple config parser
class CExtConfig
{
	// Config
	static std::string m_sSaveFolder;

	// Private ctor
	CExtConfig();

	// internal use
	static std::string m_sPath;
	static std::string m_sLastError;

public:
	// TODO: inline
	static bool TryLoad(const std::string &pathToConfig)
	{
		// Only run once
		static bool bRunOnceDone = false;
		if (bRunOnceDone)
			return true;
		bRunOnceDone = true;

		// Defaults
		m_sSaveFolder = "serialized_data";

		// Load config
		auto config = std::ifstream(pathToConfig);

		// File check
		if (!config.is_open())
		{
			util::LogMessage("ERROR! (Config). Couldn't open config file " + pathToConfig + ".");
			return false;
		}

		// Save our path
		m_sPath = pathToConfig;

		// Get config variables
		auto varMap = ParseConfigFile(config);

		// Not elegant (Check if func had an error)
		if (!m_sLastError.empty())
		{
			if (config.is_open()) config.close();
			return false;
		}

		// Check we have some variables
		if (varMap.size() == 0)
		{
			util::LogMessage("WARNING! (Config). Couldn't find any variables in " + pathToConfig + ".");
			return false;
		}

		// Set any config variables that we use
		if (varMap.count("save_folder") == 1)
		{
			m_sSaveFolder = varMap.at("save_folder");
		}

		// close safely
		if (config.is_open()) config.close();

		return true;
	}

	static std::string GetSaveFolder()
	{
		return m_sSaveFolder;
	}

private:
	static std::map<std::string, std::string> ParseConfigFile(std::ifstream &stream)
	{
		if (!stream.is_open())
		{
			util::LogMessage("ERROR! (Config). Internal - Error in ParseConfigFile() because stream wasn't open.");
			return{}; // empty map
		}

		// Prepare our result map of config variables
		std::map<std::string, std::string> configMap;

		// Loop each line in config (\r\n|\r|\n)
		std::string line = "";
		while (std::getline(stream, line))
		{
			// Get it all in one run
			std::string left = "";
			std::string right = "";
			bool haveAssignmentOp = false;

			// lets go through each line character by character
			for (auto &c : line)
			{
				// We only care about lines like something = something else
				if (c == '=')
					haveAssignmentOp = true;
				else
					if (!haveAssignmentOp)
						// Variable name
						left.push_back(c);
					else
						// Variable value
						right.push_back(c);
			}

			// Trim spaces around assignment op and edges (std::isspace)
			util::trim(left);
			util::trim(right);

			// Skip empty lines
			if (line.length() == 0)
				continue;

			// Pointless to error, should be obvious mistake (i hope)
			if (!haveAssignmentOp)
				continue;

			// check we have a variable name at least
			if (left.length() == 0)
			{
				util::LogMessage("WARNING! (Config). Syntax error - expected variable name but line started with assignment operator. Line ignored.");
				continue;
			}

			// skip comments
			if (left.front() == '#')
				continue;

			// TODO: left to lowercase

			// Check if variable name already used;
			if (configMap.count(left) == 1)
			{
				if (configMap.at(left) != right)
				{
					util::LogMessage("WARNING! (Config). Variable overrides existing value! (Variable: " + left + "; From: " + configMap.at(left) + "; To: " + right + ").");
					configMap.at(left) = right;
				}
				else
				{
					util::LogMessage("WARNING! (Config). Matching duplicate found, ignoring. (" + left + ").");
				}
			}
			else
			{
				// Success, store this variable and its value in the map
				configMap[left] = right;
			}
		}

		return configMap;
	}

};