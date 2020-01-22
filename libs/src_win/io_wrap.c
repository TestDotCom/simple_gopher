#include "io_wrap.h"
#include "stdgopher.h"
#include "stdlog.h"

static CHAR find_format(CHAR* selector)
{
    CHAR type;
    CHAR const* fmt = PathFindExtension(selector);
    
    if (strstr(fmt, ".exe") != NULL ||
            strstr(fmt, ".mp4") != NULL) {
        type = '9';
    } else if (strstr(fmt, ".jpg") != NULL) {
        type = 'I';
    } else if (strstr(fmt, ".mp3") != NULL) {
        type = 's';
    } else {
        type = '0';     // generic file
    }

    return type;
}

static INT list_dir(info_t* info)
{
    CHAR type;
    str_t menu = { 0 };

    WIN32_FIND_DATA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    TCHAR szDir[MAX_PATH];
    size_t path_len;
    
    HRESULT hres = StringCchLength(info->filename, MAX_PATH, &path_len);
	if (FAILED(hres)) {
        log_error("io_wrap::find_format - StringCchLength(). Error-code: %ld", GetLastError());
        return -1;
    }

    if (path_len > MAX_PATH - 3) {
        fprintf(stderr, "path lenght too long\n");
        return -1;
    }

    hres = StringCchCopy(szDir, MAX_PATH, info->filename);
    if (FAILED(hres)) {
        log_error("io_wrap::find_format - StringCchCopy(). Error-code: %ld", GetLastError());
        return -1;
    }

    // append \* to search everything inside path
    hres = StringCchCat(szDir, MAX_PATH, TEXT("\\*"));
    if (FAILED(hres)) {
        log_error("io_wrap::find_format - StringCchCat(). Error-code: %ld", GetLastError());
        return -1;
    }

    hFind = FindFirstFile(szDir, &ffd);
    if (hFind == INVALID_HANDLE_VALUE) {
        log_error("io_wrap::find_format - FindFirstFile(). Error-code: %ld", GetLastError());
        return -1;
    }

    do {
        if (!strcmp(ffd.cFileName, ".") || !strcmp(ffd.cFileName, "..")) {
            continue;   // ignore parent and current dir
        }

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			type = '1';
        } else {
            type = find_format(ffd.cFileName);
        }

        if (fill_menu(type, ffd.cFileName, info, &menu) == -1) {
            FindClose(hFind);
            
            return -1;
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    if (GetLastError() != ERROR_NO_MORE_FILES) {
        log_error("io_wrap::find_format - FindFirstFile(). Error-code: %ld", GetLastError());
        FindClose(hFind);
        
        return -1;
    }

    if (!FindClose(hFind)) {
        log_error("io_wrap::find_format - FindClose(). Error-code: %ld", GetLastError());
    }

    info->sendbuf = menu.buf;
    info->sendlen = menu.len;

    return ISDIR;
}

static VOID close_file(HANDLE hFile)
{
    if (!CloseHandle(hFile)) {
        log_error("io_wrap::close_file - CloseHandle(). Error-code: %ld", GetLastError());
    }
}

static INT map_file(info_t* info)
{
    LARGE_INTEGER fsize;
    OVERLAPPED overlapped = { 0 };

    HANDLE hFile = CreateFile(
        info->filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE) {
        log_error("io_wrap::map_file - CreateFile(). Error-code: %ld", GetLastError());
        return -1;
    }

    if (!GetFileSizeEx(hFile, &fsize)) {
        log_error("io_wrap::map_file - GetFileSizeEx(). Error-code: %ld", GetLastError());
        close_file(hFile);
        
        return -1;
    }

    if (!fsize.QuadPart) {
        log_error("0 file size not supported");
        close_file(hFile);
        
        return -1;
    }

    // RW lock whole file
    if (!LockFileEx(hFile, 0, 0, fsize.LowPart, fsize.HighPart, &overlapped)) {
        log_error("io_wrap::map_file - LockFile(). Error-code: %ld", GetLastError());
        close_file(hFile);

        return -1;
    }

    HANDLE hMapFile = CreateFileMapping(
        hFile,
        NULL,
        PAGE_READONLY,
        fsize.HighPart,
        fsize.LowPart,
        info->filename
    );

    if (!UnlockFileEx(hFile, 0, fsize.LowPart, fsize.HighPart, &overlapped)) {
        log_error("io_wrap::map_file - UnLockFile(). Error-code: %ld", GetLastError());
    }

    close_file(hFile);

    if (!hMapFile) {
        log_error("io_wrap::map_file - CreateFileMapping(). Error-code: %ld", GetLastError());
        return -1;
    }

    if (!(info->sendbuf = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0))) {
        log_error("io_wrap::map_file - MapViewOfFile(). Error-code: %ld", GetLastError());
        return -1;
    }

    info->sendlen = fsize.QuadPart;

    return ISFILE;
}

INT find_file(info_t* info)
{
    DWORD fileAttr;
    if (INVALID_FILE_ATTRIBUTES == (fileAttr = GetFileAttributes(info->filename))) {
        log_error("io_wrap::find_file - GetFileAttributs(). Error-code: %ld", GetLastError());
        return -1;
    }

    if (fileAttr == FILE_ATTRIBUTE_DIRECTORY) {
        return list_dir(info);
    } else {
        return map_file(info);
    }
}
