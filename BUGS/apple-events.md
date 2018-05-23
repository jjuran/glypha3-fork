Required Apple events are unimplemented
=======================================

Glypha III doesn't install handlers for the required Apple events.

While in a sense it can get away with this on the pretext that it's designed for System 6, the PPC build stretches this theory rather thin.  In any case, the A-line build system currently has no means of specifying the flags in the 'SIZE' resource -- so whatever fallback mechanisms the OS has won't be used.

In particular, sending a [Quit event fails][quit-event].

[quit-event]:  <quit-apple-event.md>

Remedy
------

Implement the required Apple events.

History
-------

Since Glypha III dates from 1995 (years after System 7 shipped), this has arguably been a bug since the beginning.
