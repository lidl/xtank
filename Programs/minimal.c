/* minimal.c */

#include <xtanklib.h>

static void main()
{
    while (1) {
	done();
    }
}

Prog_desc minimal_prog = {
    "minimal",
    "Vanguard",
    "Does nothing.",
    "Robert Potter",
    0,
    0,
    main
};
