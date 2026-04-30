#include "EventParserUT.hpp"
#include <thread>

using namespace std;

void CEventParserUT::TestParserUtilityMethods()
{
	string validLine = "2023-03-07 07:20:32	3";
	string invalidStateLine = "2023-03-07 07:20:32 3"; //space instead of a tab between time and state
	string invalidTimeLine = "202303-07	07:20:32	3"; //time part is not in expected format

	int validState = 3;
	int invalidState = -1;

	tm validTimeTemp = {};
	
	//setup the valid time from above string for comparison
	std::istringstream ss("2023-03-07 07:20:32");
	ss >> get_time(&validTimeTemp, "%Y-%m-%d %H:%M:%S");
	time_t validTime = std::mktime(&validTimeTemp);


	if (m_pParser)
	{
		//Test valid state in log line
		int temp = m_pParser->GetEventStateFromLine(validLine);
		if (temp != validState)
		{
			m_errors.push_back({ "TestParserUtilityMethods", "UT", "GetEventStateFromLine did not return expected state for valid line - " + validLine });
		}

		//Test invalid state in log line
		temp = m_pParser->GetEventStateFromLine(invalidStateLine);
		if (temp != invalidState)
		{
			m_errors.push_back({ "TestParserUtilityMethods", "UT", "GetEventStateFromLine did not return expected state for invalid state line - " + invalidStateLine });
		}

		//Test valid date time in log line
		tm tempTm = m_pParser->GetEventTimeFromLine(validLine);
		//Convert tempTm to time_t for comparison
		time_t t1 = std::mktime(&tempTm);
		if (t1 != validTime)
		{
			m_errors.push_back({ "TestParserUtilityMethods", "UT", "GetEventTimeFromLine did not return expected date time for valid line - " + validLine });
		}

		//Test invalid date time in log line, should return a tm struct with mday set to 0.
		tempTm = m_pParser->GetEventTimeFromLine(invalidTimeLine);
		if (tempTm.tm_mday != 0)
		{
			m_errors.push_back({ "TestParserUtilityMethods", "UT", "GetEventTimeFromLine did not return expected date time for invalid line - " + invalidTimeLine });
		}
	}
}

/// <summary>
/// Method to run in a thread.
/// </summary>
void WorkerThread(CEventParser* parser, string deviceID, string logName)
{
	parser->ParseEvents(deviceID, logName);
}

void CEventParserUT::TestParserMultiThreaded(std::string baseLogPath)
{
	string deviceID = "Hvac1";
	string logName = baseLogPath + "/logs/testHV.txt";

	thread t1(WorkerThread, m_pParser, deviceID, logName);
	thread t2(WorkerThread, m_pParser, deviceID, logName);
	thread t3(WorkerThread, m_pParser, deviceID, logName);
	
	//Wait for all threads to finish
	if(t1.joinable())
		t1.join();

	if(t2.joinable())
		t2.join();

	if(t3.joinable())
		t3.join();
	
	//validate outcome
	int count = m_pParser->GetEventCount(deviceID);
	if (count != 6) //we are parsing the same log 3 times, that has two fault. We should have 6 faults total.
	{
		m_errors.push_back({ "TestParserMultiThreaded", "UT", "Did not return expected fault count. Expected Faults: 3. Actual Faults: " + to_string(count) });
	}
}

void CEventParserUT::TestParser(std::string baseLogPath)
{
	string logFile0 = baseLogPath + "/logs/sequence no time lapse - no fault.txt";
	string logFile1 = baseLogPath + "/logs/testHV.txt";
	string logFile2 = baseLogPath + "/logs/testHV 2 faults.txt";

	m_pParser->ParseEvents("HV0", logFile0);
	m_pParser->ParseEvents("HV1", logFile1);
	m_pParser->ParseEvents("HV2", logFile2);

	int file0result = m_pParser->GetEventCount("HV0");
	int file1result = m_pParser->GetEventCount("HV1");
	int file2result = m_pParser->GetEventCount("HV2");

	if (file0result != 0 || file1result != 2 || file2result != 2)
	{
		m_errors.push_back({ "TestParser", "UT", "Did not return expected fault counts in TestParser method" });
	}
}

std::string CEventParserUT::GetAllUTErrors() const
{
	std::string allErrors;
	for (const ErrorInfo& errorEntry : m_errors)
	{
		allErrors += errorEntry.logName + " - Device: " + errorEntry.deviceName + ", Error Message: " + errorEntry.errorMessage + "\n";
	}
	return allErrors;
}

