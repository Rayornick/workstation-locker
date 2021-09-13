#include <windows.h>
#include <iostream>
#include <atomic>

#include <thread>

std::atomic<bool> abort_job(false);

void makeJob()
{
    LockWorkStation();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    SendMessage(
            (HWND)0xffff,   // HWND_BROADCAST
            0x0112,         // WM_SYSCOMMAND
            (WPARAM)0xf170, // SC_MONITORPOWER
            (LPARAM)0x0002  // POWER_OFF
        );
}

void timerTillJob(uint32_t seconds_left)
{
    std::cout << "     > Press any key to abort operation! <" << std::endl << std::endl;
    std::cout << "Monitor will turned off and workstation will locked" << std::endl;
    while(seconds_left > 0)
    {
        std::cout << "in " << seconds_left << " seconds...\r" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        seconds_left--;
        
        if(abort_job)
            break;
    }
    
    if(!abort_job)
    {
        abort_job = true;
        makeJob();
    }
}

void abortJob()
{
    HANDLE in_handle;
    INPUT_RECORD input_record;
    DWORD num_read;
    
    in_handle = GetStdHandle(STD_INPUT_HANDLE);
    while(!abort_job)
    {
        GetNumberOfConsoleInputEvents(in_handle, &num_read);
        if(num_read == 0)
            continue;
        
        ReadConsoleInput(in_handle, &input_record, 1, &num_read);
        
        if(input_record.EventType == KEY_EVENT)
        {
            if (input_record.Event.KeyEvent.bKeyDown)
            {
                abort_job = true;
                std::cout << "Operation aborted\r" << std::flush;
            }
        }
    }
}

int main(int argc, char**argv)
{
    uint32_t seconds_till_jod = 20u;
    
    if(argc > 1)
    {
        try {
            seconds_till_jod = std::clamp(std::stoul(argv[1]), 0ul, 60ul);
        } catch (std::exception const &e) {
            seconds_till_jod = 20u;
        }
    }
    
    std::thread job_thread(timerTillJob, seconds_till_jod);
    std::thread abort_job_thread(abortJob);
    
    job_thread.join();
    abort_job_thread.join();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    return 0;
}
