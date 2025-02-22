#include "ttkdumper.h"
#include "ttkversion.h"
#include "musicobject.h"
#include "musicotherdefine.h"
#include "miniprocess.h"

#ifdef Q_OS_WIN
#  include <qt_windows.h>
#  include "dbghelp.h"
#elif defined Q_OS_UNIX
#  include <signal.h>
#  include <execinfo.h>
#  include <unistd.h>
#  include <fcntl.h>
#endif

static void cleanAppicationCache()
{
    QFile::remove(MUSIC_COLOR_FILE);
    QFile::remove(MUSIC_IMAGE_FILE);
    QFile::remove(MUSIC_RECORD_FILE);
    QFile::remove(MUSIC_RECORD_DATA_FILE);

    ///clean thirdparty process
    QStringList process;
    process << MAKE_TRANSFORM_PREFIX
           << MAKE_KRC2LRC_PREFIX
           << MAKE_PLAYER_PREFIX
           << MAKE_GAIN_PREFIX;
    killProcessByName(process);
}

class TTKDumperPrivate : public TTKPrivate<TTKDumper>
{
public:
    TTKDumperPrivate();
    ~TTKDumperPrivate();

#ifdef Q_OS_WIN
    typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
                                        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                        CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

    #define MAX_WARNING_MESSAGE_PATH 1024
    #define MAX_DUMP_FILE_NUMBER 9999

    static LONG WINAPI errorHandler(EXCEPTION_POINTERS *info);
#elif defined Q_OS_UNIX
    static void errorHandler(int id);
#endif

    void initialize();

    static QString m_name;
    static QString m_version;
};

QString TTKDumperPrivate::m_name;
QString TTKDumperPrivate::m_version;

TTKDumperPrivate::TTKDumperPrivate()
{

}

#ifdef Q_OS_WIN
void TTKDumperPrivate::initialize()
{
    ::SetUnhandledExceptionFilter(errorHandler);
}

LONG TTKDumperPrivate::errorHandler(EXCEPTION_POINTERS *info)
{
    LONG retval = EXCEPTION_CONTINUE_SEARCH;

    // firstly see if dbghelp.dll is around and has the function we need
    // look next to the EXE first, as the one in System32 might be old
    // (e.g. Windows 2000)
    HMODULE instance = nullptr;
    WCHAR dbgHelpPath[_MAX_PATH];

    if(GetModuleFileNameW(nullptr, dbgHelpPath, _MAX_PATH))
    {
        WCHAR *slash = wcsrchr(dbgHelpPath, L'\\');
        if(slash)
        {
            wcscpy(slash + 1, L"DBGHELP.DLL");
            instance = LoadLibraryW(dbgHelpPath);
        }
    }

    if(instance == nullptr)
    {
        // load any version we can
        instance = LoadLibraryW(L"DBGHELP.DLL");
    }

    if(instance)
    {
        MINIDUMPWRITEDUMP dump = TTKVoid_cast(MINIDUMPWRITEDUMP)GetProcAddress(instance, "MiniDumpWriteDump");
        if(dump)
        {
            WCHAR dumpPath[_MAX_PATH];
            WCHAR dumpRootPath[_MAX_PATH];
            WCHAR scratch[_MAX_PATH];

            // work out a good place for the dump file
            if(GetModuleFileNameW(nullptr, dbgHelpPath, _MAX_PATH))
            {
                WCHAR *slash = wcsrchr(dbgHelpPath, L'\\');
                if(slash)
                {
                    wcscpy(slash + 1, L"");
                    wcscpy(dumpPath, dbgHelpPath);
                }
            }
            else if(!GetTempPathW(_MAX_PATH, dumpPath))
            {
                wcscpy(dumpPath, L"c:\\temp\\");
            }
            wcscpy(dumpRootPath, dumpPath);

            // ask the user if they want to save a dump file
            //if(::MessageBox(nullptr, _T("Something bad happened in your program, would you like to save a diagnostic file?"), m_szAppName, MB_YESNO)==IDYES)
            {
                HANDLE file = INVALID_HANDLE_VALUE;
                int i = 1;
                WCHAR fileNumber[_MAX_PATH];
                while(file == INVALID_HANDLE_VALUE)
                {
                    swprintf(fileNumber, sizeof(fileNumber), L"_%04d",i);
                    wcscpy(dumpPath, dumpRootPath);
                    wcscat(dumpPath, m_name.toStdWString().c_str());
                    wcscat(dumpPath, L"_");
                    wcscat(dumpPath, m_version.toStdWString().c_str());
                    wcscat(dumpPath, fileNumber);
                    wcscat(dumpPath, L".dmp");

                    file = ::CreateFileW(dumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
                    i++;
                    if(i > MAX_DUMP_FILE_NUMBER)
                    {
                        file = ::CreateFileW(dumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                        break;
                    }
                }
                // create the file
                if(file != INVALID_HANDLE_VALUE)
                {
                    MINIDUMP_EXCEPTION_INFORMATION exInfo;
                    exInfo.ThreadId = ::GetCurrentThreadId();
                    exInfo.ExceptionPointers = info;
                    exInfo.ClientPointers = 0;

                    // write the dump
                    BOOL ok = dump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, &exInfo, nullptr, nullptr);
                    if(ok)
                    {
                        swprintf(scratch, sizeof(scratch), L"Saved dump file to '%s'", dumpPath);
                        retval = EXCEPTION_EXECUTE_HANDLER;
                    }
                    else
                    {
                        swprintf(scratch, sizeof(scratch), L"Failed to save dump file to '%s' (error %d)", dumpPath, GetLastError());
                    }
                    ::CloseHandle(file);

                    WCHAR outMessage[MAX_WARNING_MESSAGE_PATH];
                    swprintf(outMessage, sizeof(outMessage), outMessage, m_name.toStdWString().c_str(), dumpPath);
                }
                else
                {
                    swprintf(scratch, sizeof(scratch), L"Failed to create dump file '%s' (error %d)", dumpPath, GetLastError());
                }
            }
        }
    }
    return retval;
}
#elif defined Q_OS_UNIX
void TTKDumperPrivate::errorHandler(int id)
{
    TTK_LOGGER_INFO("App error occurred, error code " << id);
    cleanAppicationCache();

    char stamp[50];
    sprintf(stamp, "%ld", time(nullptr));
    const TTKString& file_name = (m_name + '_' + m_version).toStdString() + "." + stamp + ".dmp";

    constexpr int size = 512;
    void* array[size];
    const int fd = open(file_name.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    backtrace_symbols_fd(array, backtrace(array, size), fd);
    close(fd);

    raise(id);
    exit(0);
}

void TTKDumperPrivate::initialize()
{
    signal(SIGPIPE, errorHandler);
    signal(SIGSEGV, errorHandler);
    signal(SIGFPE, errorHandler);
    signal(SIGABRT, errorHandler);
}
#endif

TTKDumperPrivate::~TTKDumperPrivate()
{
    cleanAppicationCache();
}



TTKDumper::TTKDumper()
{
    TTK_INIT_PRIVATE(TTKDumper);
    TTK_D(TTKDumper);
    d->m_name = "TTK";
    d->m_version = TTK_VERSION_STR;
}

void TTKDumper::run()
{
    TTK_D(TTKDumper);
    d->initialize();
}
