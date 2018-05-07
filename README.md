Glypha III fork
===============

A fork of Glypha III (a 1990's Mac shareware game written by John Calhoun) maintained by Josh Juran.

Rationale
---------

Why a fork?

The upstream [softdorothy/glypha_III][glypha_III] repository is unmaintained.  All the development is happening here, so it makes sense for this repository to be the core, not a satellite.

[glypha_III]:  <https://github.com/softdorothy/glypha_III>

Building
--------

This fork of Glypha III uses a custom build tool called A-line.  The CodeWarrior project files from the original import have been preserved in history but since removed.  The file [`A-line.conf`][A-line.conf] tells A-line everything it needs to know about how to build Glypha III in just [five terse lines][A-line.conf].

[A-line.conf]:  <https://github.com/jjuran/glypha3-fork/blob/public/A-line.conf>

Well, everything *specific to Glypha III*, that is.  Knowledge of how to build a Mac application generally is distributed between additional project config files, A-line itself, and other programs that A-line calls to do the heavy lifting.

To build Glypha III as a Mach-O application, you'll need:

  * Mac OS X with developer tools (the 10.4 and 10.5 SDKs are known to work)
  * Git (which you can build from source)
  * the [metamage_1][] repository (in order to build A-line and cpres)

First, get the repositories.  (Normally you should use `https:` instead of `git:`, but older Mac OS X systems can't verify the TLS certificates.)

	git clone git://github.com/jjuran/glypha3-fork.git
	git clone git://github.com/jjuran/metamage_1.git

Next, build the tools you'll need:

	cd metamage_1
	./build.pl A-line cpres

Install them in a directory listed in `$PATH` (assumed to be `~/bin` here):

	cp var/build/*-mach-carb-dbg/bin/*/{A-line,cpres} ~/bin/

Build A-line's project catalog:

	cd ..
	ALINE_SRC_ROOT=$PWD A-line -t

Finally, build Glypha III:

	A-line glypha3

And run it!

	open ~/var/build/*-mach-carb-dbg/bin/glypha3/Glypha\ III.app

To build Glypha III as a 68K or CFM PPC application, you'll need:

  * a PowerPC classic Mac OS environment (SheepShaver and OS X's Classic are okay)
  * Metrowerks CodeWarrior Pro 6 (earlier versions *might* work but haven't been tested)
  * Apple's MPW (Macintosh Programmer's Workshop) environment -- not the compilers, just ToolServer (included with CodeWarrior)
  * Universal Interfaces (3.4.2 has been tested, but CodeWarrior's should also work)
  * [MacRelix][], a POSIX-like environment for classic Mac OS (in which to run A-line and its related tools)
  * the [metamage_1][] repository (where lives A-line's store of knowledge)
  * my help, in all likelihood
  * lots of patience

I'm sorry it's this difficult.  This is what happens when an OS doesn't come with free developer tools.

[MacRelix]:    <https://www.macrelix.org/>
[metamage_1]:  <https://github.com/jjuran/metamage_1>

Running
-------

Glypha III's system requirements are unchanged, except that it no longer switches the screen depth in Mac OS X (even when running in Classic) -- so technically, 256 colors is no longer a requirement.

While Glypha III can run in Mac OS X's Classic environment, the performance suffers.  For best results on Mac OS X, build Glypha III as a Carbon application and run it natively.
