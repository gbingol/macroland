"""
This file is run at the 'end of selection'.

Definition of 'end of selection':
1) A range is being selected and it is the point when the left-button of the mouse is released.
2) A range is being selected with Shift button + Arrow Keys pressed
"""

from numpy import median, var
from _sci import Framework, Workbook, prettify, temporary

#Status bar field where statistics are written.
STBAR_FIELD = 1
ROUND = 4


class Goto(Exception):
	pass



if __name__=='__main__':
	try:
		ws = Workbook().activeworksheet()

		if ws == None:
			raise Goto()

		rng = ws.selection()
		if rng == None:
			raise Goto()

		lst = rng.tolist()
		lst = [i for i in lst if isinstance(i, int|float)]
		if len(lst)<3:
				raise Goto()

		#sample variance
		variance = prettify(var(lst, ddof=1))
		medianval = prettify(median(lst))

		"""
		selecting event will write to status bar before selected event.
		Therefore in order not to confuse the user, append selected event's
		output to selecting event's.
		"""
		Text = Framework().StatusBar().readtext(STBAR_FIELD) 
		if isinstance(Text, str):
			if temporary.EVENTSFOLDER["FIRED_SELECTING"]:
				Text += f"; var:{variance}; median:{medianval}"
			else:
				Text = f"var:{variance}; median:{medianval}"

		Framework().StatusBar().writetext(Text, STBAR_FIELD)
	
	except Goto:
		pass
