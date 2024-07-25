"""
Here, page literally means worksheet. Thus this file is run when switched between worksheets.
Therefore, it is highly recommended to put very light-weight computation here
"""

from _sci import statbar_write, Workbook, colnum2label


#Status bar field where statistics are written.
STBAR_FIELD = 1


def _printgridcursor(StBarField):
	ws = Workbook().activeworksheet()
	if ws != None:
		row, col = ws.cursor()
		Text = colnum2label(col + 1) + str(row + 1) #E16, A1..
		statbar_write(Text, StBarField)


if __name__ == '__main__':
	"""
	Writing the text to the field where statistics on selection was/will be written.
	"""
	_printgridcursor(STBAR_FIELD)