/*****************************************************************************\
* special-defs.h - part of XTank					      *
* 									      *
* This file contains specials definitions.				      *
* 									      *
* The definitions are presented in the form of a macro invokation with	      *
* several arguments belonging to various contexts.  This file is #included in *
* different places under differnt definitions of the macro, each of which     *
* selects the argument(s) relevant at that point.  Kind of obscure, but it    *
* keeps all the data in the same place.					      *
* 									      *
* The macro should have the general form:				      *
* 									      *
* #define QQ(sym,type,cost) ...						      *
* 									      *
* The args are: internal symbol used in the code, text name, cost	      *
\*****************************************************************************/

/* sym          type          cost */
QQ(CONSOLE,    "Console",      250)
QQ(MAPPER,     "Mapper",       500)
QQ(RADAR,      "Radar",       1000)
QQ(REPAIR,     "Repair",     50000)
QQ(RAMPLATE,   "Ram Plate",   2000)
QQ(DEEPRADAR,  "Deep Radar",  8000)
QQ(STEALTH,    "Stealth",    20000)
QQ(NAVIGATION, "Navigation",    20)
