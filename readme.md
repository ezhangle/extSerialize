Serialize Extension for Arma 3
===============================
copyright uniflare at gmail.com 2017


- - - - - - - - - - - - - - - - - - - - - - -
Brief:
	Serialize extension for Arma 3 is a very simple extension that reads and writes raw data strings.
	Strings are serialized inside the extension.

- - - - - - - - - - - - - - - - - - - - - - -
Concept:
	Client-side (Either):
		0. No direct access to extension.
		
	Server-side (read):
		1. Server checks if extension is ready/available (version check via callExtension)
		2. Server sends read request with unique identifier (max 64 chars)
		3. Extension will either reply with either:
			a. The full data
			b. A full buffer of partial data (extension limitation)
			c. ENDOFDATA string identifier signalling end of data
		Note: Server read must loop the read request until ENDOFDATA is returned for that unique data identifier.
		
	Server-side (write):
		1. Server checks if extension is ready/available
		2. Server sends write request with unique identifier and either:
			a. The full Data
			b. A full buffer of partial data (extension limitation)
			c. ENDOFDATA string to tell the extension there is no more data
		3. Server can check last return for SUCCESS or ERROR string
		

	Extension-side (read):
		1. Checks if an existing request is being served (mid-read or mid-write)
			if the unique identifier does not match, exit with "ERROR"
		2. Loads the data if needed via the unique identifier
		3. Returns either:
			a. The full data
			b. A full buffer of partial data
			c. ENDOFDATA string
		Note: Extension will always end with ENDOFDATA in a read request.

	Extension-side (write):
		1. Checks if an existing request is being served (mid-read or mid-write)
			if the unique identifier does not match, return ERROR
		2. Creates a capture string to add to if not created yet for this identifier
		3. Checks if the request is "ENDOFDATA"
			Found: Writes file with name as hash of unique identifier
			Not Found: Adds data to current captured data string
		4. If data is empty and file glob exists then it will be deleted
		4. Returns SUCCESS or ERROR
		
	At any call an ERROR string may be returned from the extension.
	Use the request "get_last_error" with the unique identifier to get the error details.

- - - - - - - - - - - - - - - - - - - - - - -
Usage:
	Pop the dll into your arma 3 server root directory (next to arma3server.exe/arma3server_x64.exe)
	Note:All the following functions can only be used server-side ofc
	
	When you want to write some data to retrieve later:
		_null = callExtension "serializeExt" format ["write;%1;%2", _data_identifier, _raw_string];
		
	When you want to read some data already saved:
		_raw_string = callExtension "serializeExt" format ["read;%1", _data_identifier];
		
	Note: Writing empty data to a unique identifier will effectively delete the data (server file will be deleted).

- - - - - - - - - - - - - - - - - - - - - - -
Request signatures:
	
	[READ]
		syntax:		read;<data_identifier>
		return:		string <Full Data|Partial Data|"ERROR"|"ENDOFDATA">
		Example:
			
			
	[WRITE]
		syntax:		write;<data_identifier>;<raw_string>
		return:		string <"ERROR"|"SUCCESS">
		Example:
			
			
	[GET LAST ERROR]
		syntax:		get_last_error;<data_identifier>
		return:		string <"ERROR"|Last Error Details>
		Example:
			
			
	[VERSION]
		syntax:		version
		return:		string <"ERROR"|"SUCCESS">
		Example:
			