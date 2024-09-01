"""
This file is run at the 'end of selection'.

Definition of 'end of selection':
1) A range is being selected and it is the point when the left-button of the mouse is released.
2) A range is being selected with Shift button + Arrow Keys pressed
"""

import numpy as np
import numbers as _numbers
from _sci import Framework, Workbook

#Status bar field where statistics are written.
STBAR_FIELD = 1
ROUND = 4


def _printstats(StBarField):
    ws = Workbook().activeworksheet()

    if ws == None:
        return

    rng = ws.selection()
    if rng == None:
        return

    lst = rng.tolist()
    lst = [i for i in lst if isinstance(i, _numbers.Real)]
    if len(lst)<3:
         return

    #sample variance
    variance = round(np.var(lst, ddof=1), ROUND)
    median = round(np.median(lst), ROUND)

    """
    selecting event will write to status bar before selected event.
    Therefore in order not to confuse the user, append selected event's
    output to selecting event's.
    """
    Text = Framework().StatusBar().readtext(STBAR_FIELD) 
    if isinstance(Text, str):
        Text += f"; var:{variance}; median:{median}"

    Framework().StatusBar().writetext(Text, StBarField)


if __name__=='__main__':
	"""
	Currently there are 3 fields (0, 1, 2) in status bar. 
	Therefore, we are writing to the mid one.
	"""
	_printstats(STBAR_FIELD)