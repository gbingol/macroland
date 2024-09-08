import numbers as _numbers

from _sci import (Framework, StatusBar, PageChangedEvent, SelectedEvent, SelectingEvent, RightClickEvent,
                  Workbook, colnum2label, prettify, vars)

#Status bar field where statistics are written.
STBAR_FIELD = 1
ROUND = 4



def _printgridcursor(StBarField):
	ws = Workbook().activeworksheet()
	if ws == None:
		return
	
	row, col = ws.cursor()
	Text = f"{colnum2label(col + 1)}{row + 1}" #E16, A1..
	StatusBar().writetext(Text, StBarField)



def _selected(StBarField):
	#reports sample variance
	from statistics import variance, median

	ws = Workbook().activeworksheet()
	rng = ws.selection()
	if rng == None:
		return

	DataX = rng.tolist()
	DataX = [i for i in DataX if isinstance(i, int|float)]
	if len(DataX)<3:
		return

	#sample variance
	var = prettify(variance(DataX))
	medianval = prettify(median(DataX))

	"""
	selecting event will write to status bar before selected event.
	Therefore in order not to confuse the user, append selected event's
	output to selecting event's.
	"""
	Text = StatusBar().readtext(StBarField) 
	if isinstance(Text, str):
		Text += f"; var:{var}; median:{medianval}"

	StatusBar().writetext(Text, StBarField)



def _selecting(StBarField):
	ws = Workbook().activeworksheet()

	if ws == None:
		return

	rng = ws.selection()
	if rng == None:
		return
	
	nrows, ncols = rng.nrows(), rng.ncols()

	Text = f"{nrows}x{ncols} "

	lst = rng.tolist()
	lst = [i for i in lst if isinstance(i, _numbers.Real)]
	NRealNums = len(lst)
	if NRealNums == 0:
		StatusBar().writetext(Text, StBarField)
		return
	
	Sum, Aver, Min, Max  = 0, 0, lst[0], lst[0]
	for i in lst:
		Sum += i
		Min = min(Min, i)
		Max = max(Max, i)
	
	Sum = round(Sum, ROUND)
	Aver = round(Sum / NRealNums, ROUND)
	Min = round(Min, ROUND)
	Max = round(Max, ROUND)
	Text += f"sum:{Sum} ; min:{Min} ; mean:{Aver} ; max:{Max}"

	StatusBar().writetext(Text, StBarField)






if __name__=='__main__':
	Workbook().bind(SelectingEvent(), _selecting, STBAR_FIELD)
	Workbook().bind(SelectedEvent(), _selected, STBAR_FIELD)
	Workbook().bind(PageChangedEvent(), _printgridcursor, STBAR_FIELD)
	