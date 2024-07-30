"""
This file is called when there is a selection is being made on a worksheet.
Therefore, it is highly recommended to put light-weight computation here
"""
import numbers as _numbers
from _sci import Framework, Workbook

#Status bar field where statistics are written.
STBAR_FIELD = 1



def _printstats(StBarField):
	ws = Workbook().activeworksheet()

	if ws == None:
		return

	rng = ws.selection()
	if rng == None:
		return
	
	nrows, ncols = rng.nrows(), rng.ncols()

	Text = f"{nrows}x{ncols}  "

	lst = rng.tolist()
	lst = [i for i in lst if isinstance(i, _numbers.Real)]
	NRealNums = len(lst)
	if NRealNums == 0:
		Framework().statbar_write(Text, StBarField)
		return
	
	Sum, Aver, Min, Max  = 0, 0, lst[0], lst[0]
	for i in lst:
		Sum += i
		Min = min(Min, i)
		Max = max(Max, i)
	
	Sum = round(Sum, 4)
	Aver = round(Sum / NRealNums, 4)
	Min = round(Min, 4)
	Max = round(Max, 4)
	Text += f"sum:{Sum} ; min:{Min} ; mean:{Aver} ; max:{Max}"

	Framework().statbar_write(Text, StBarField)


if __name__=='__main__':
	"""
	Currently there are 3 fields (0, 1, 2) in status bar. 
	Therefore, we are writing to the mid one.
	"""
	_printstats(STBAR_FIELD)
	