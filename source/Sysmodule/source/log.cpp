#include "switch.h"
#include "log.h"
#include <sys/stat.h>
#include <cstdarg>
#include "SwitchUtils.h"
#include <stdio.h>

static Mutex printMutex = 0;

void DiscardOldLogs()
{
    SwitchUtils::ScopedLock printLock(printMutex);

    FsFileSystem* fs = fsdevGetDeviceFileSystem("sdmc");
    FsFile file;
    s64 fileSize;

    Result rc = fsFsOpenFile(fs, LOG_PATH, FsOpenMode_Read, &file);
    if (R_FAILED(rc))
        return;

    rc = fsFileGetSize(&file, &fileSize);
    fsFileClose(&file);
    if (R_FAILED(rc))
        return;

    if (fileSize >= 0x20'000)
    {
        fsFsDeleteFile(fs, LOG_PATH);
        WriteToLog("Deleted previous log file");
    }
}

void WriteToLog(const char* fmt, ...)
{
    SwitchUtils::ScopedLock printLock(printMutex);

    // ams::TimeSpan ts = ams::os::ConvertToTimeSpan(ams::os::GetSystemTick());

    time_t unixTime = time(NULL);
    struct tm tStruct;
    localtime_r(&unixTime, &tStruct);

    mkdir(CONFIG_PATH, 777);

    FILE* fp = fopen(LOG_PATH, "a");

    // Print time
    fprintf(fp, "%02i %02i:%02i:%02i: ", tStruct.tm_mday, tStruct.tm_hour, tStruct.tm_min, tStruct.tm_sec);
    // fprintf(fp, "%02lid %02li:%02li:%02li: ", ts.GetDays(), ts.GetHours() % 24, ts.GetMinutes() % 60, ts.GetSeconds() % 60);

    // Print the actual text
    va_list va;
    va_start(va, fmt);
    vfprintf(fp, fmt, va);
    va_end(va);

    fprintf(fp, "\n");
    fclose(fp);
}

void LockedUpdateConsole()
{
    SwitchUtils::ScopedLock printLock(printMutex);
    consoleUpdate(NULL);
}