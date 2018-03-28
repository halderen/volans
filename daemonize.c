#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include <stdarg.h>
#include <syslog.h>
#include <pwd.h>
#include <grp.h>

extern void *argv0;

int
daemonize(char *directory)
{
    int status = 0;
    int fd, i;
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        return -1;
    }
    if (pid > 0) {
        exit(0);
    }
    if(setsid() < 0) {
        /* log error */
        status = -1;
    }
    if(directory) {
        if(chdir(directory)) {
            /* log error */
            status = -1;
        }
    }
    for (i = getdtablesize(); i >= 0; --i)
        close(i);
    fd = open("/dev/null", O_RDWR, 0);
    if (fd != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > 2) {
            close(fd);
        }
    }
    umask(027);
    return status;
}

int
drop(char *username, char* groupname)
{
    int status = 0;
    struct passwd* userinfo;
    struct group* groupinfo;
    if (username != NULL) {
        userinfo = getpwnam(username);
        if (userinfo) {
            if (setuid(userinfo->pw_uid)) {
                status = -1;
                /* log error */
            }
            if(chdir(userinfo->pw_dir)) {
                /* log warning */
                if(chdir("/")) {
                status = -1;
                 /* log error */
                }
            }
        } else {
                status = -1;
            /* log error */
        }
    }
    if (groupname != NULL) {
        groupinfo = getgrnam(groupname);
        if (groupinfo) {
            if (setgid(groupinfo->gr_gid)) {
                status = -1;
                /* log error */
            }
        } else {
                status = -1;
            /* log error */
        }
    }
    return status;
}
