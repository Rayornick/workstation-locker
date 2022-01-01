//============================================================================
// Name        : main.cpp
// Author      : Ivan Smolyakov
//============================================================================

#include <iostream>
#include <thread>
#include <windows.h>

using namespace std::chrono;

void makeJob()
{
    LockWorkStation();

    std::this_thread::sleep_for(seconds(1));

    SendMessage(
            (HWND)0xffff,   // HWND_BROADCAST
            0x0112,         // WM_SYSCOMMAND
            (WPARAM)0xf170, // SC_MONITORPOWER
            (LPARAM)0x0002  // POWER_OFF
        );
}

bool abortJob(steady_clock::time_point untill_time)
{
    HANDLE in_handle;
    INPUT_RECORD input_record;
    DWORD num_read;

    in_handle = GetStdHandle(STD_INPUT_HANDLE);

	std::cout << "     > Press any key to abort operation! <" << std::endl << std::endl;
    std::cout << "Monitor will turned off and workstation will locked" << std::endl;

	auto calcTime = [](steady_clock::time_point untill_time)
	{
		auto cur_time = steady_clock::now();
		return (cur_time >= untill_time)
			? 0.0
			: double((untill_time - cur_time).count()) * steady_clock::period::num / steady_clock::period::den;
	};

	uint32_t seconds_left = static_cast<uint32_t>(calcTime(untill_time));
	uint32_t last_seconds_left = seconds_left + 1;

    while(seconds_left)
    {
        GetNumberOfConsoleInputEvents(in_handle, &num_read);

        if(seconds_left != last_seconds_left)
        {
        	std::cout << "in " << seconds_left << " seconds...\r" << std::flush;
        	last_seconds_left = seconds_left;
        }

        seconds_left = static_cast<uint32_t>(calcTime(untill_time));

        if(num_read == 0)
            continue;

        ReadConsoleInput(in_handle, &input_record, 1, &num_read);

        if(input_record.EventType == KEY_EVENT)
        {
            if (input_record.Event.KeyEvent.bKeyDown)
            {
                std::cout << "Operation aborted\r" << std::flush;
				return true;
            }
        }
    }

	return false;
}

int main(int argc, char** argv)
{
    uint32_t seconds_till_job = 20u;

    if(argc > 1)
    {
        try {
            seconds_till_job = std::clamp(std::stoul(argv[1]), 0ul, 60ul);
        } catch (std::exception const &e) {
            seconds_till_job = 20u;
        }
    }

	if(!abortJob(steady_clock::now() + seconds(seconds_till_job)))
		makeJob();

    std::this_thread::sleep_for(seconds(1));

    return 0;
}
