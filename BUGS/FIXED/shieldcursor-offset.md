[FIXED] ShieldCursor() offset is off
====================================

The call to ShieldCursor() passes an offset which is only correct if the screen is 640x480 and the menu bar is 20px high.

Remedy
------

Just use the main window's local coordinate system to compute the offset by translating 0,0.

Confusingly, the operation required is GlobalToLocal(), not LocalToGlobal().

Status
------

This is now fixed.

History
-------

This bug dates back to the 1.0.3 source release and is confirmed present in Glypha III 1.0.1.

Fixed in May 2018.
