#pragma once
#include <vector>
#include <string>
#include "EventParser.hpp"


struct ErrorInfo
{
	std::string logName;
	std::string deviceName;
	std::string errorMessage;
};

class CEventParserUT
{
private:
	CEventParser* m_pParser;
	std::vector<ErrorInfo> m_errors;

public:
	CEventParserUT() : m_pParser(new CEventParser()) {}
	~CEventParserUT() 
	{ 
		if (m_pParser != nullptr)
		{
			delete m_pParser;
			m_pParser = nullptr;
		}
	}

	/// <summary>
	/// Tests GetEventStateFromLine and GetEventTimeFromLine with a valid and invalid string, checks expected success or failure result.
	/// </summary>
	void TestParserUtilityMethods();

	/// <summary>
	/// Tests the parser in 3 threads in a succession, waits for all the threads to finish and checks if we have the expected number of faults for the device.
	///  Note: In ParseEvents method, there is a Sleep(2000) that is commented out, you can uncomment that to simulate a long running ParseEvents method and test the thread safety of the method.
	/// </summary>
	void TestParserMultiThreaded(std::string baseLogPath);

	/// <summary>
	/// Tests the parser with 3 different log files and verifies the known resuts from those log files.
	/// </summary>
	void TestParser(std::string baseLogPath);

	/// <summary>
	/// Rests internals of this object.
	/// </summary>
	void Reset() { m_errors.clear(); if (m_pParser) m_pParser->ResetParser(); };
	
	/// <summary>
	/// Gets the accumulated error count from running the unit tests.
	/// </summary>
	/// <returns>Integer of error count</returns>
	int  GetUTErrorCount() const { return static_cast<int>(m_errors.size()); }

	/// <summary>
	/// Gets all the recorded errors from the tests runs.
	/// </summary>
	/// <returns>String of errors, one error per line</returns>
	std::string GetAllUTErrors() const;
};

