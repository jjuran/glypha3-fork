File menu's Quit item persists in Aqua
======================================

In Aqua, an application shouldn't include a Quit item in its File menu.

(Technically, Glypha III's leftmost non-Apple menu is named "Game", not "File" -- but it's still morally a File menu:  It begins with a New command and ends with Quit.)

The actual conditional removal of the two menu items (Quit itself and the separator above it) is trivial.  The problem is that the OS-provided Quit menu item relies on Apple events, which Glypha III [doesn't implement][apple-events].

[apple-events]:  <apple-events.md>

Remedy
------

Implement the [Quit Apple event][quit-event] to unblock work on this bug.

[quit-event]:  <FIXED/quit-apple-event.md>

Status
------

The Quit Apple event has been implemented, so this is now unblocked.

History
-------

This only became a bug when Glypha III was ported to Carbon in April 2018.
