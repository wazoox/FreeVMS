#include "lib.h"
#include "l4io.h"
#include "kip.h"
#include "sigma0.h"
#include "thread.h"
#include "bootinfo.h"

#include "information.h"
#include "system.h"
#include "levels.h"

#define NULL ((void *) 0)

#define FREEVMS_VERSION "0.0.1"

#define notice(...) printf(__VA_ARGS__)

char *strstr(const char *, const char *);

#define PANIC	printf("Panic !\n"); while(1);
