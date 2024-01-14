"""
Here, page literally means worksheet. Thus this file is run when switched between worksheets.
Therefore, it is highly recommended to put light-weight computation here
"""


from _sci import statbar_write, activeworksheet, colnum2labels


#Status bar field where statistics are written.
STBAR_FIELD = 1


def _printgridcursor(StBarField):
	ws = activeworksheet()
	if ws != None:
		row, col = ws.cursor()
		Text = colnum2labels(col + 1) + str(row + 1) #E16, A1..
		statbar_write(Text, StBarField)


"""
Writing the text to the field where statistics on selection was/will be written.
"""
_printgridcursor(STBAR_FIELD)