#include "EventParser.hpp"
#include <fstream>
#include <cassert>
#include <windows.h>

//const for invalid state when parsing a log entries
const int invalidEventState = -1;

CEventParser::~CEventParser()
{}

CEventParser::CEventParser()
{}

void CEventParser::ParseEvents(std::string deviceID, const std::string& logName)
{
	//allow only access to one thread at a time to this method
	std::lock_guard<std::mutex> lock(m_parseEventsMutex);

    //Uncomment to test threading and large log file
    //Sleep(2000);

	const int sequenceDalay = 5 * 60; // 5 minutes in seconds
    std::string line;
    FaultState currentState = Stateless;
    std::tm state3Time = {};
    std::ifstream logfile(logName);

    if (!logfile.is_open()) 
    {
        fprintf(stderr, "Error: Could not open the specified file: '%s'.\n", logName.c_str());
        return ;
    }
    
    //Read the log file, line at a time, till EOF
    while (std::getline(logfile, line)) 
    {
        int lineState = GetEventStateFromLine(line);
        
        //Handle if this method returned invalidEventState
        if (invalidEventState == lineState)
        {
            //break out of here, we have an invalid log, should log this error somewhere...
            break;
        }

        //
        //We are interested in states 3, 2 and 0 only as per spec.
		//Enter an appropriate state (in FaultState enum) based on the log's line state and current state:
		// 1. If we get a state 3 while in Stateless, we will enter InState3 and record the time of that state 3 transition.
		//      NOTE: if we get additional state 3(s) while InState3, those events will be ignored and the first state 3 will be treated as start of the sequence (NOT in spec).
		// 2. If we get a state 2 while in InState3, we will check if the time difference between state 3 and state 2, if it is greater than or equal to 5 minutes, 
        //      then, we will enter InState2, else, we will reset to Stateless as this is not a valid sequence for a fault.
		// 3. If we are InState2, all additional state 2 and state 3 will be ignored (as per spec).
   		// 4. If we get a state 0 while in InState2, we will count that as a fault for the device, increment the failure count for that device in the m_deviceFaults map
        //    and reset state to Stateless to look for new fault sequence.
		// 5. If we get a state 0 while in any other state except InState2, we will reset to Stateless and look for new sequence, as in above 1 - 4 steps.
		// 6. If any other state is encountered (other than 3, 2, 0) we will reset to Stateless and look for new sequence (Default in switch below).
        //
        switch (lineState)
        {
            case 3:
				//Note: if we get a state 3 while in state 3 state, igone that and treat the first state3 as the start of the sequence.
                if (Stateless == currentState)
                {
                    std::tm tmp = GetEventTimeFromLine(line);
					assert(0 != tmp.tm_mday); //we should be getting valid time from log line, if not, we have an issue with log format or GetEventTimeFromLine method.
					if (0 != tmp.tm_mday)//check if we got a valid time from log line
                    {
                        state3Time = tmp;
                        currentState = InState3;
                    }
                }
				break;
            case 2:
                if (InState3 == currentState)
                {
                    std::tm state2Time = GetEventTimeFromLine(line);
                    if (0 != state2Time.tm_mday)//check if we got a valid time from log line
                    {
                        // Calculate the time difference between state3Time and state2Time
                        std::time_t t1 = std::mktime(&state3Time);
                        std::time_t t2 = std::mktime(&state2Time);
                        double timeDiffSeconds = std::difftime(t2, t1);
                        //Check for time diff between recorded state 3 and this state 2, if it is greater than or equal to 15 min, then we will enter InState2
                        if (timeDiffSeconds >= sequenceDalay)
                        {
                            //Transsition to state2
                            currentState = InState2;
                        }
                        else
                        {
                            //we have got a state 3 and then a state 2 before the 15 min interval for a fault sequence, reset.
                            currentState = Stateless;
                            state3Time = {};
                        }
                    }
                    else
                    {
                        //couldnt get valid time from log, reset
						assert(0);
                        currentState = Stateless;
                        state3Time = {};
                    }
                }
                break;
            case 0:
                if (InState2 == currentState)
                {
                    m_deviceFaults[deviceID]++; // Increment fault count for this device
                }
                //break omitted to go to default;
            default:
                currentState = Stateless; // Reset state for any other line state
                state3Time = {};
				break;
        }
    }

    logfile.close();
}

// Helper methods to get state and time of an event from a single log line.
int CEventParser::GetEventStateFromLine(const std::string& line) const
{
    int eventState = invalidEventState;
	size_t lastTabPos = line.find_last_of('\t');
    if (lastTabPos != std::string::npos)
    {
        std::string numPart = line.substr(lastTabPos + 1);
        try 
        {
            eventState = std::stoi(numPart);
        }
        catch (...)
        {
            eventState = invalidEventState; // Invalid state value
        }
    }

    return eventState;
}

std::tm CEventParser::GetEventTimeFromLine(const std::string& line) const
{
    std::tm returnTime = {};
	
    // Find the tab character that separates the timestamp from the state to get the time
    size_t tabPos = line.find('\t');

    if (tabPos != std::string::npos) {
		// Extract "2023-03-07 07:20:32" part and convert to tm struct
        std::string dateTimeStr = line.substr(0, tabPos);
        std::istringstream ss(dateTimeStr);
        ss >> std::get_time(&returnTime, "%Y-%m-%d %H:%M:%S");

        if (ss.fail()) {
            returnTime = {};
        }
    }

    return returnTime;
}


int CEventParser::GetEventCount(std::string deviceId) const
{
    if (m_deviceFaults.contains(deviceId))
        return m_deviceFaults.at(deviceId);
    else
        return 0;
}

