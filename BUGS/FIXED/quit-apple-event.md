[FIXED] Quit Apple event fails
==============================

Glypha III doesn't quit in response to a Quit event.  This is primarily because it [doesn't implement Apple events][apple-events] at all.

[apple-events]:  <../apple-events.md>

Secondarily, nonetheless setting the isHighLevelEventAware flag inhibits Mac OS' MultiFinder-style fallback mechanism, but even if the flag is cleared, it doesn't work anyway.

In classic Mac OS, sending Glypha III a Quit event blocks the sending application's process (and prevents switching away from it) until it times out.  If the isHighLevelEventAware flag is cleared, sending a Quit event fails immediately with paramErr (perhaps because the Quit item is in a menu called "Game" instead of "File").

Remedy
------

Clearing the flag avoids one failure mode in favor of another that's less disruptive, but still a failure.  The solution is to implement the required Apple events (or at least the Quit event).

Status
------

The Quit Apple event has been implemented, so this is now fixed.

History
-------

Unless earlier systems seek the Quit menu item more aggressively than System 7.6.1 does, the paramErr failure mode has been a bug since the game's inception in 1995.  The timeout failure mode replaced the paramErr bug when Glypha III was ported to the A-line build system in April 2018.

Fixed in May 2018.
