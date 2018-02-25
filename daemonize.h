#ifndef DAEMONIZE_H
#define DAEMONIZE_H

extern char *argv0;

int daemonize(char *directory);

int drop(char *username, char* groupname);

#endif
