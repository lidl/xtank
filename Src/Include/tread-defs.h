/*****************************************************************************\
* tread-defs.h - part of XTank						      *
* 									      *
* This file contains tread definitions.					      *
* 									      *
* The definitions are presented in the form of a macro invokation with	      *
* several arguments belonging to various contexts.  This file is #included in *
* different places under different definitions of the macro, each of which    *
* selects the argument(s) relevant at that point.  Kind of obscure, but it    *
* keeps all the data in the same place.					      *
* 									      *
* The macro should have the general form:				      *
* 									      *
* #define QQ(sym,type,fric,cost) ...					      *
* 									      *
* The args are: 							      *
* 	internal symbol used in the code				      *
* 	text name							      *
* 	friction							      *
* 	cost								      *
\*****************************************************************************/

/* sym            type   friction  cost */
QQ(SMOOTH_TREAD, "Smooth",  0.70,  100)
QQ(NORMAL_TREAD, "Normal",  0.80,  200)
QQ(CHAIN_TREAD,  "Chained", 0.90,  400)
QQ(SPIKED_TREAD, "Spiked",  1.00, 1000)
QQ(HOVER_TREAD,  "Hover",   0.20,  500)
