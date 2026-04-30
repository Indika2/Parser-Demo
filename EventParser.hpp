#pragma once
#include "IEventCounter.hpp"
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <chrono>
#include <mutex>
#include <ctime>

//Keep track of current state when running ParseEvents.
enum FaultState { Stateless, InState3, InState2 };

class CEventParser : public IEventCounter
{
public:
	~CEventParser();
	CEventParser();

private:
	// Mutex to protect ParseEvents method from concurrent access.
	std::mutex m_parseEventsMutex;
		
	// Map of device ID to running count of faults in all parsed logs.
	std::map<std::string, int> m_deviceFaults; 
		
public:
	//IEventCounter implementations

	/// <summary>
	/// Parse and accumulate event information (fault sequences) from the given log data.
	/// </summary>
	/// <param name="deviceID">ID of the device that the log is associated with (ex: "HV1")</param>
	/// <param name="eventLog">A stream of lines representing time/value recordings.</param>
	void ParseEvents(std::string deviceID, const std::string& logName) override;

	/// <summary>
	/// Gets the accumulated fault count for the given device ID, if the device ID is not found in the map, return 0.
	/// </summary>
	/// <returns>An integer representing the number of detected events</returns>
	[[nodiscard]] int GetEventCount(std::string deviceId) const override;
	
	/// <summary>
	/// Returns parsed event state  from the given line.
	/// </summary>
	/// <returns>An integer parsed from the line, or invalidEventState (-1) in case of error</returns>
	int GetEventStateFromLine(const std::string& line) const;

	/// <summary>
	/// Returns parsed event time from the given line.
	/// </summary>
	/// <returns>An std::tm struct that represent the parsed time, or empty std::tm in case of error</returns>
	std::tm GetEventTimeFromLine(const std::string& line) const;
	
	/// <summary>
	/// Rests the internal variables.
	/// </summary>	
	void ResetParser() { m_deviceFaults.clear(); }
};


