[FIXED] High score colors are too dark
======================================

The high scores are drawn in dark blue text on a background in Mac OS X, even in Classic.  They're supposed to be in multiple brighter colors.

The problem is that the colors are sourced from the color lookup table, but in Mac OS X (including Classic), Glypha III no longer switches screen depths, so the 256-color lookup table it expects isn't in effect.

Remedy
------

Remove the Index2Color() calls and encode the RGB colors directly.

Optionally, specify the colors in a resource instead of in the source code.

Status
------

The five colors used are now specified explicitly in the code, and Index2Color() is no longer called, so this is now fixed.

History
-------

This is a regression introduced in April 2018 by disabling depth switching in Mac OS X.

Fixed in May 2018.
