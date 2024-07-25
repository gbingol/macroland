"""
This file is called when there is a selection is being made on a worksheet.
Therefore, it is highly recommended to put light-weight computation here
"""
import numbers as _numbers
from _sci import statbar_write, Workbook

#Status bar field where statistics are written.
STBAR_FIELD = 1



def _printstats(StBarField):
	ws = Workbook().activeworksheet()

	rng = ws.selection()
	nrows, ncols = rng.nrows(), rng.ncols()

	Text = str(nrows) + " x " + str(ncols)

	lst = rng.tolist()
	lst = [i for i in lst if isinstance(i, _numbers.Real)]
	N = len(lst)
	if N>0:
		Sum, Aver, Min, Max  = 0, 0, lst[0], lst[0]
		for i in lst:
			Sum += i
			Min = min(Min, i)
			Max = max(Max, i)
		
		Sum = round(Sum, 4)
		Aver = round(Sum / N, 4)
		Min = round(Min, 4)
		Max = round(Max, 4)
		Text += ",   sum:" + str(Sum) + ",  min:" + str(Min) + ",  mean:"+str(Aver) + ",  max:" + str(Max)

	statbar_write(Text, StBarField)


"""
Currently there are 3 fields (0, 1, 2) in status bar. 
Therefore, we are writing to the mid one.
"""
_printstats(STBAR_FIELD)
	