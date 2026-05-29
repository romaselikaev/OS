#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#pragma comment(lib, "winmm.lib")  

const char* MAPPING_NAME = "MySharedMemory";
const char* MUTEX_NAME = "RW_Mutex";

int main() {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    DWORD pageSize = si.dwPageSize;
    const int numPages = 10;
    SIZE_T bufferSize = numPages + (SIZE_T)pageSize * numPages;
    bool wait = true;

    HANDLE hMap = CreateFileMappingA(
        INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE, 0, (DWORD)bufferSize, MAPPING_NAME
    );
    if (!hMap) { std::cerr << "Error: CreateFileMapping failed\n"; return 1; }

    BYTE* pBuf = (BYTE*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, bufferSize);
    if (!pBuf) { CloseHandle(hMap); std::cerr << "Error: MapViewOfFile failed\n"; return 1; }
    VirtualLock(pBuf, bufferSize);

    HANDLE hMutex = CreateMutexA(NULL, FALSE, MUTEX_NAME);
    if (!hMutex) { UnmapViewOfFile(pBuf); CloseHandle(hMap); std::cerr << "Error: CreateMutex failed\n"; return 1; }

    DWORD pid = GetCurrentProcessId();
    std::ofstream log("Reader_" + std::to_string(pid) + ".log");
    log << std::unitbuf;
    srand(timeGetTime() ^ pid);

    int lastPage = rand() % numPages;

    while (true) {
        int page = -1;
        if (wait) {
            wait = false;
            log << "[" << timeGetTime() << "] PID=" << pid << " waiting\n";
        }
        WaitForSingleObject(hMutex, INFINITE);
        for (int i = 0; i < numPages; ++i) {
            int idx = (lastPage + i) % numPages;
            if (pBuf[idx] == 2) {
                pBuf[idx] = 3;
                page = idx;
                break;
            }
        }
        ReleaseMutex(hMutex);

        if (page != -1) {
            log << "[" << timeGetTime() << "] PID=" << pid << " read page " << page << "\n";
            volatile BYTE data = pBuf[numPages + page * pageSize];

            Sleep(500 + rand() % 1001);

            WaitForSingleObject(hMutex, INFINITE);
            pBuf[page] = 0;
            ReleaseMutex(hMutex);
            log << "[" << timeGetTime() << "] PID=" << pid << " release\n";
            lastPage = (page + 1) % numPages;
            wait = true;
        }
    }

    UnmapViewOfFile(pBuf); CloseHandle(hMap); CloseHandle(hMutex);
    return 0;
}