#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

static struct option command_options[] = {
    { "help", no_argument, NULL, 'h'},
    { "config", no_argument, NULL, 'c'},
    { 0, 0, 0, 0}
};

static struct option command_zoneadd_options[] = {
    { "policy", no_argument, NULL, 'p'},
    { 0, 0, 0, 0}
};
static struct option daemon_options[] = {
    { "help", no_argument, NULL, 'h'},
    { "config", no_argument, NULL, 'c'},
    { "no-daemon", no_argument, NULL, 'D'},
    { 0, 0, 0, 0}
};

extern char* argv0;

char* argv0;

static void
usage() {
    fprintf(stderr, "usage: %s [-h]\n", argv0);
}

struct command_struct {
    enum {
        STATUS, ZONEADD, ZONEDEL, MODELREAD, MODELCREATE
    } cmd;
    union {
        struct {
            char *name;
            char *policy;
        } zoneadd;
        struct {
            char *name;
        } zonedel;
    } arg;
};

extern int commandStatus(void);
extern int commandZoneAdd(char *zone, char* policy);
extern int commandZoneDel(char *zone);
extern int commandModelCreate(void);
extern int commandModelRead(void);

int
main(int argc, char** argv)
{
    int status;
    int ch;
    int i;
    struct command_struct command;

    /* Get the name of the program */
    if ((argv0 = strrchr(argv[0], '/')) == NULL)
        argv0 = argv[0];
    else
        ++argv0;

    if (!strcmp(argv0, "opendnssec")) {
        while ((ch = getopt_long(argc, argv, "+hc", command_options, NULL)) >= 0) {
            switch (ch) {
                case 'h':
                    usage(argv[0]);
                    return EXIT_SUCCESS;
                case '?':
                    return EXIT_FAILURE;
                default:
                    usage(argv[0]);
                    return EXIT_FAILURE;
            };
        }
        if (argc - optind >= 1 && !strcmp(argv[optind + 0], "status")) {
            command.cmd = STATUS;
            optind += 2;
        } else if (argc - optind >= 2 && !strcmp(argv[optind + 0], "model") && !strcmp(argv[optind + 1], "create")) {
            command.cmd = MODELCREATE;
            optind += 2;
        } else if (argc - optind >= 2 && !strcmp(argv[optind + 0], "model") && !strcmp(argv[optind + 1], "read")) {
            command.cmd = MODELREAD;
            optind += 2;
        } else if (argc - optind >= 2 && !strcmp(argv[optind + 0], "zone") && !strcmp(argv[optind + 1], "add")) {
            command.cmd = ZONEADD;
            optind += 2;
        } else if (argc - optind >= 2 && !strcmp(argv[optind + 0], "add") && !strcmp(argv[optind + 1], "zone")) {
            command.cmd = ZONEADD;
            optind += 2;
        } else if (argc - optind >= 2 && !strcmp(argv[optind + 0], "zone") && !strcmp(argv[optind + 1], "delete")) {
            command.cmd = ZONEDEL;
            optind += 2;
        } else if (argc - optind >= 2 && !strcmp(argv[optind + 0], "delete") && !strcmp(argv[optind + 1], "zone")) {
            command.cmd = ZONEDEL;
            optind += 2;
        } else if (argc - optind >= 2 && !strcmp(argv[optind + 0], "zone") && !strcmp(argv[optind + 1], "del")) {
            command.cmd = ZONEDEL;
            optind += 2;
        } else if (argc - optind >= 2 && !strcmp(argv[optind + 0], "del") && !strcmp(argv[optind + 1], "zone")) {
            command.cmd = ZONEDEL;
            optind += 2;
        } else if (argc - optind == 0) {
            fprintf(stderr, "%s: no command given.\n", argv0);
	    return EXIT_FAILURE;
        } else {
            fprintf(stderr, "%s: unrecognized command line arguments:\n", argv0);
            for (i = optind; i < argc; i++) {
                fprintf(stderr, " %s\n", argv[i]);
            }
            return EXIT_FAILURE;
        }
        switch(command.cmd) {
            case STATUS:
                if (argc - optind != 1) {
                    fprintf(stderr, "%s: status command expects no arguments\n",argv0);
                    return EXIT_FAILURE;
                }
                break;
            case MODELCREATE:
                if (argc - optind != 1) {
                    fprintf(stderr, "%s: model create command expects no arguments\n",argv0);
                    return EXIT_FAILURE;
                }
                break;
            case MODELREAD:
                if (argc - optind != 1) {
                    fprintf(stderr, "%s: model read command expects no arguments\n",argv0);
                    return EXIT_FAILURE;
                }
                break;
            case ZONEADD:
                command.arg.zoneadd.policy = "default";
                while ((ch = getopt_long(argc, argv, "p", command_zoneadd_options, NULL)) >= 0) {
                    switch (ch) {
                        case 'p':
                            command.arg.zoneadd.policy = optarg;
                            return EXIT_SUCCESS;
                        case '?':
                            return EXIT_FAILURE;
                        default:
                            usage();
                            return EXIT_FAILURE;
                    }
                }
                if (argc - optind != 1) {
                    fprintf(stderr, "%s: zone add command expects one argument specifying zone name.\n",argv0);
                    return EXIT_FAILURE;
                }
                command.arg.zoneadd.name = argv[optind];
                break;
            case ZONEDEL:
                if (argc - optind != 1) {
                    fprintf(stderr, "%s: zone delete command expects one argument specifying zone name.\n",argv0);
                    return EXIT_FAILURE;
                }
                command.arg.zonedel.name = argv[optind];
                break;
        }
        switch(command.cmd) {
            case STATUS:
                status = commandStatus();
                break;
            case MODELCREATE:
                status = commandStatus();
                break;
            case MODELREAD:
                status = commandStatus();
                break;
            case ZONEADD:
                status = commandZoneAdd(command.arg.zoneadd.name, command.arg.zoneadd.policy);
                break;
            case ZONEDEL:
                status = commandZoneDel(command.arg.zonedel.name);
                break;
        }
        if(status) {
            return EXIT_FAILURE;
        }
    } else if (!strcmp(argv0, "opendnssecd")) {
        while ((ch = getopt_long(argc, argv, "hcD", daemon_options, NULL)) >= 0) {
            switch (ch) {
                case 'h':
                    usage(argv[0]);
                    return EXIT_SUCCESS;
                case '?':
                    return EXIT_FAILURE;
                default:
                    usage(argv[0]);
                    return EXIT_FAILURE;
            };
        }
        if (optind < argc) {
            fprintf(stderr, "%s: unrecognized command line arguments.\n", argv0);
        }
    } else {
        fprintf(stderr, "%s: bad command line invocation; executable name %s not recognized.\n", argv[0], argv0);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int commandStatus(void)
{
	return 0;
}
int commandZoneAdd(char *zone, char* policy)
{
	return 0;
}
int commandZoneDel(char *zone)
{
	return 0;
}
