/*
** Xtank
**
** Copyright 1993 by Pix Technologies Corp.
**
** special-defs.h
*/

/*
$Author: lidl $
$Id: special-defs.h,v 1.1.1.1 1995/02/01 00:25:43 lidl Exp $
*/

/****************************************************************/
/* This file contains specials definitions.                     */
/*                                                              */
/* The definitions are presented in the form of a macro         */
/* invocation with several arguments belonging to various       */
/* contexts.  This file is included in different places under   */
/* different definitions of the macro, each of which selects    */
/* the argument(s) relevant at that point.  Kind of obscure,    */
/* but it keeps all the data in the same place.                 */
/*                                                              */
/****************************************************************/
/* The arguments are:
   internal symbol used in the code, text name, cost
*/

/* sym          type          cost */
QQ(CONSOLE, "Console", 250)
QQ(MAPPER, "Mapper", 500)
QQ(RADAR, "Radar", 1000)
QQ(REPAIR, "Repair", 30000)
QQ(RAMPLATE, "Ram Plate", 2000)
#ifndef NO_HUD
QQ(HUD, "HU Display", 1)
#else /* !NO_HUD */
QQ(DEEPRADAR, "Deep Radar", 8000)
#endif /* !NO_HUD */
QQ(STEALTH, "Stealth", 20000)
QQ(NAVIGATION, "Navigation", 20)
QQ(NEW_RADAR, "New Radar", 3000)
QQ(TACLINK, "Tac Link", 1000)
#ifndef NO_CAMO
QQ(CAMO, "Camo", 2000)
QQ(RDF, "Rdf", 1000)
#endif /* !NO_CAMO */
