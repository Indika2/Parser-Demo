#pragma once
#include <string>

/// <summary>
/// Interface for counting events within a log file.
/// </summary>
struct IEventCounter
{
    /// <summary>
    /// Parse and accumulate event information from the given log data.
    /// </summary>
    /// <param name="deviceID">ID of the device that the log is associated with (ex: "HV1")</param>
    /// <param name="eventLog">A stream of lines representing time/value recordings.</param>
    virtual void ParseEvents(std::string deviceID, const std::string& logName) = 0;

    /// <summary>
    /// Gets the current count of events detected for the given device
    /// </summary>
    /// <returns>An integer representing the number of detected events</returns>
    [[nodiscard]] virtual int GetEventCount(std::string deviceId) const = 0;
};
