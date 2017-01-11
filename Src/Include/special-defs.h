/*-
 * Copyright (c) 1993 Josh Osborne
 * Copyright (c) 1993 Kurt Lidl
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
