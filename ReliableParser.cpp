// Main.cpp : This file contains the 'main' function with sample of running the parser and its unit tests.
//

#include "EventParser.hpp"
#include "EventParserUT.hpp"
#include <filesystem>

int main()
{
    std::string baseLogPath = std::filesystem::current_path().string();
    CEventParser* p = new CEventParser();
    if (p != nullptr)
    {
        p->ParseEvents("H1", baseLogPath + "/logs/testHV.txt");
        p->ParseEvents("H1", baseLogPath + "/logs/testHV.txt");
        p->ParseEvents("H2", baseLogPath + "/logs/testHV 2 faults.txt");
        p->ParseEvents("H3", baseLogPath + "/logs/sequence no time lapse - no fault.txt");
        p->ParseEvents("H4", baseLogPath + "/logs/Multiple 3 - 2 one fault.txt");


        int H1count = p->GetEventCount("H1"); //should be 4 faults (ran the 2 fault file twice)
        int H2count = p->GetEventCount("H2"); //should be 2 faults
        int H3count = p->GetEventCount("H3"); //should be 0 faults
        int H4count = p->GetEventCount("H4"); //should be 1 fault

        delete p;
    }

    CEventParserUT* ut = new CEventParserUT();
    if (ut != nullptr)
    {
        std::string testResults1;
        std::string testResults2;
        std::string testResults3;

        ut->TestParserUtilityMethods();
        testResults1 = ut->GetAllUTErrors();

        ut->Reset();
        ut->TestParserMultiThreaded(baseLogPath);
        testResults2 = ut->GetAllUTErrors();

        ut->Reset();
        ut->TestParser(baseLogPath);
        testResults3 = ut->GetAllUTErrors();

        delete ut;
    }
}