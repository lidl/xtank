# Xtank

## What is Xtank?

Xtank is a multi-player vehicle combat game played in mazes of
various sizes and types.  You control a vehicle (usually a tank)
equipped with the latest in armor and weaponry.  Your objective
depends on the game you play.

## License

This release of Xtank has been re-licensed under the BSD 2-clause
copyright, sometimes referred to as the "FreeBSD Copyright".  Check
individual files for the name of the contributing Author(s).

## Source Code Layout

This is the top-level Xtank directory.  The subdirectories are as follows:

* Doc           Documentation on how to play Xtank
* Bin           Programs and shell scripts
* Help          Files used by the graphical help system in Xtank
* Todo          Ideas for the future, lists from the past
* Mazes         Files describing Mazes used in Xtank
* Programs      Files for robot programs to load into Xtank
* Src           Source code for Xtank
* Util          Random utilities used in writing Xtank (largely untested)
* Vehicles      Files describing vehicles used in Xtank
* README.md     This file

## How To Build

* Get a copy of the source code repository, and unpack it.
* Make sure that you have X11 libraries installed for your
  machine.  Xtank only requires libX11.
* Build the software:
  * On FreeBSD: Run 'make'
  * On Mac OS X: Run 'bmake'

### FreeBSD notes

The code is expected to work fairly well on FreeBSD 10, FreeBSD 11,
and FreeBSD-current (which is, as of this writing, what identifies
itself as FreeBSD-12).

### Mac OS X notes

The code is expected to work fairly well on Mac OS X.  As of this
writing, the code is known to work on OS X 10.11.6 (El Capitan).

The build system uses the Berkeley make command, typically called
'bmake' under Mac OS X.  The version that has been tested was
installed from MacPorts.

To display the graphics from Xtank, you will need an X11 server.
The code has been tested using Xquartz 2.7.11, and it works OK.

### Other systems

No work has been put into making it easy to build this software on
other platforms, as of yet.  This will be done, but getting something
that works released was deemed the most important first step.

## Mailing List

A mailing list has been created for discussion relating to the
code of Xtank, and for help with building/porting issues.

* Subscribe at: xtank-admin@xtank.org
* Post at: xtank@xtank.org

Posting are only allowed from email addresses that are subscribed
to the mailing list, to prevent abuse from spammers.  Sorry, but
the Internet is not the friendly, open place that it used to be.
