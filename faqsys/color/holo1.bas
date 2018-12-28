
'     ******** Program for reviewing hologram subjects ********

SCREEN 12: COLOR 4: pi = 3.1416: a = 125

 FOR t = 0 TO 1 * pi STEP .02
    r = a * COS(t * 3)   '********-PLACE ANY FUNCTION HERE-********
     x = 320 + r * COS(t): y = 240 + r * SIN(t)
    PSET (x, y)
 NEXT t

DO: LOOP WHILE INKEY$ = ""

