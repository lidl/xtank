This is the main Xtank directory.  The subdirectories are as follows:

Doc           Documentation on how to play Xtank
Bin           Programs and shell scripts
Bin/xtank     Shell script to run Xtank
Bin/dbgxtank  Shell script to run Xtank under dbx
Help          Files used by the on-line help system in Xtank
Todo          Ideas for the future, lists from the past
Mazes         Files describing Mazes used in Xtank (stickybitted)
Programs      Files for robot programs to load into Xtank (stickybitted)
Src           Source code for Xtank
Util          Random utilities used in writing Xtank (largely untested)
Vehicles      Files describing vehicles used in Xtank (stickybitted)

README        This file

Code is currently being maintained by stripes@pix.com and lidl@pix.com

There are two USENET news groups that are used to support this game,
rec.games.xtank.programmer, and rec.games.xtank.play.  The old
mailing list that we used to run has been superceded by the
rec.games.xtank.programmer newsgroup.  However, if you can't get
USENET news, mail to xtank-request@eng.umd.edu, and ask to
be added to the newsgroup gateway.  We gateway the .programmer
newsgroup into the xtank@eng.umd.edu mailing list.

Note: requests to be added to the mailing list that are directed
to the main mailing list are most likely to be ignored, with
extreme prejudice.  Additionally, if you are on the mailing list,
and mail to your site bounces, you will be removed from the mailing
list.  If you don't have reliable E-Mail, you have no business
installing a game like Xtank.

To build this UMDENG Xtank distribution:

         0) Make sure that you system has "imake" installed on it.
            If your computer vendor was stupid enough to ship X11R?,
            and not ship "imake" with it, we *really* don't want to
            know about it.  Tell your vendor to fix this brain damage,
            quickly.
         1) "chmod +w Imakefile.Config"
         2) Edit Imakefile.Config for your site
            If you have a supported system, *all* you should have to
            change is the XTANK_DIR variable in this file.
         3) "xmkmf"
         4) "make Makefiles"
         5) "make"
         6) Edit the xtank shell script in Bin/xtank so it knows
            where you put the xtank binaries.

Note: If you do not have a recent version of "flex" installed on your
system, you will need to copy some files around before you can build
things.  Namely, you should copy .../Src/vload.flexed to
.../Src/vload.c and copy .../Src/setups.flexed to .../Src/setups.c

Keep the original .flexed files around, in case something bad happens...

If it didn't work correctly on your system, keep reading!

Make sure you read the FAQ file, located in Help/xtank.FAQ!
That might help you out, it has lots of useful information in it!

We need you to tell us:
  * What your setup is:
       - Hardware platform (eg Sun Sparcstation 1+)
       - OS version (eg SunOS 4.1.2)
       - Windowing System (eg OpenWin 3, or MIT X11R5, DecWindows, etc)
       - Compiler used (eg gcc 1.37, vendor cc, etc)
       - any non-default Imakefile.Config -D flags
       - what version of Xtank that you are trying to run
  * If it works.
  * If it doesn't work.
  * If it doesn't work, and you know why it doesn't
  * If it almost works.
  * If you like it.
  * If you don't like it.
  * If it improves your sex life.

Our set-ups (where we test the release before shipping it):
    Sun3's running SunOS 4.1 and 4.1.1, using SUN_LWP, sun's cc
    Sun4's, running SunOS 4.1.2, using SUN_LWP, sun's cc
    (sometimes with GCC V2.2.2 also)
    DECstation 5000's, running Ultrix 4.2, using THREAD_MP, dec's cc
    VAXstation 3100's, running Ultrix 4.2, using THREAD_MP, gcc 2.2.2

By the way, previous versions (before 1.2e) used "machtype" or it's
ilk to figure out if it was a VAX or a Sun, or whatever.  Since that
wasn't very portable, for a while we used "arch", since that was
"standard" on SunOS and BSD4.4 we used to use that.  But no
longer!

Now that we use a real Imakefile, we use the symbol that is defined
in the X11R4 or X11R5 source tree from MIT.  This is large and ugly,
and it shows in the Imakefile.  However, it is extremely portable
across *lots* of architectures.  After all, if you have X, you ought
to have xtank.  (The only thing that still requires some machine
intelligence is the Bin/xtank shell script.  You're on your own
there, sorry.)

						- stripes & lidl
