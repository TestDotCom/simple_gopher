#include "io_wrap.h"
#include "stdgopher.h"
#include "stdlog.h"

static char find_format(char* selector)
{
    char type;

    char const* s = "file -b --mime-type ";
    const size_t len = strlen(selector) + strlen(s) + 1;
    char cmd[len];
    
    FILE* f;
    char* mimetype = NULL;
    size_t mimelen = 0;

    if (snprintf(cmd, len, "%s%s", s, selector) == -1) {
        log_errno_msg("io_wrap::find_format - snprintf()");
        return '3';
    }

    if (!(f = popen(cmd, "r"))) {
        log_errno_msg("io_wrap::find_format - popen()");
        return '3';
    }

    if (getline(&mimetype, &mimelen, f) == -1) {
        log_errno_msg("io_wrap::find_format - getline()");
        return '3';
    }

    if (strstr(mimetype, "application") != NULL) {
        type = '9';
    } else if (strstr(mimetype, "image") != NULL) {
        type = 'I';
    } else if (strstr(mimetype, "audio") != NULL) {
        type = 's';
    } else {
        type = '0';     // generic file
    }

    free(mimetype);

    if (pclose(f) == -1) {
        log_errno_msg("io_wrap::find_format - pclose()");
    }

    return type;
}

static int list_dir(info_t* info)
{
    char type;
    str_t menu = { 0 };

    size_t pathlen;
    struct stat stbuf;

    struct dirent* dp;
    DIR* dirp = opendir(info->filename);

    if (!dirp) {
        log_errno_msg("io_wrap::find_format - opendir()");
        return -1;
    }

    while ((dp = readdir(dirp))) {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) {
            // ignore parent and current dir
            continue;
        }

        pathlen = strlen(info->filename) + strlen(dp->d_name) + 2;
        char newpath[pathlen];

        if (snprintf(newpath, pathlen, "%s/%s", info->filename, dp->d_name) == -1) {
            log_errno_msg("io_wrap::list_dir - snprintf()\n");
            closedir(dirp);

            return -1;
        }

        if (stat(newpath, &stbuf) == -1) {
            log_errno_msg("io_wrap::list_dir - stat()");
            closedir(dirp);

            return -1;
        }

        if (S_ISDIR(stbuf.st_mode)) {
            type = '1';
        } else {
            type = find_format(newpath);
        }

        if (fill_menu(type, dp->d_name, info, &menu) == -1) {
            closedir(dirp);
            return -1;
        }
    }
    
    closedir(dirp);

    info->sendbuf = menu.buf;
    info->sendlen = menu.len;

    return ISDIR;
}

static void close_unlock_file(int fd)
{
    if (lockf(fd, F_ULOCK, 0) == -1) {
        log_errno_msg("io_wrap::close_unlock_file - lockf(F_UNLOCK)");
    }

    if (close(fd) == -1) {
        log_errno_msg("io_wrap::close_unlock_file - close()");
    }
}

static int map_file(info_t* info)
{
    int fd;
    struct stat statbuf;

    if (-1 == (fd = open(info->filename, O_RDWR))) {
        log_errno_msg("io_wrap::map_file - open()");
        return -1;
    }

    if (lockf(fd, F_LOCK, 0) == -1) {
        log_errno_msg("io_wrap::map_file - lockf(F_LOCK)");

        if (close(fd) == -1) {
            log_errno_msg("io_wrap::map_file - close()");
        }

        return -1;
    }

    if (fstat(fd, &statbuf) == -1) {
        log_errno_msg("io_wrap::map_file - fstat()");
        close_unlock_file(fd);

        return -1;
    }

    if (!statbuf.st_size) {
        log_error("0 file size not allowed");
        close_unlock_file(fd);

        return -1;
    }

    if (MAP_FAILED == (info->sendbuf = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0))) {
        log_errno_msg("io_wrap::map_file - mmap()");
        close_unlock_file(fd);

        return -1;
    }

    close_unlock_file(fd);

    info->sendlen = statbuf.st_size;

    return ISFILE;
}

int find_file(info_t* info)
{
    struct stat stbuf;
    if (stat(info->filename, &stbuf) == -1) {
        log_errno_msg("io_wrap::find_file - stat()");
        return -1;
    }

    if (S_ISDIR(stbuf.st_mode)) {
        return list_dir(info);
    }

    return map_file(info);
}
