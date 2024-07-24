from wx import GetTextFromUser 
from _sci import (activeworksheet, CommandWindowDict, messagebox)
import numpy as _np



class GotoLabel(Exception): 
	"""Serves as a label"""
	pass




if __name__ == '__main__':
	try:
		ws = activeworksheet()

		msg = """
Enter a valid expression that returns ndarray|list|dict|str|int|float.

IMPORTANT: Any variables used in the expression must already be defined in the command window.

For 1 dimensional containers, selecting more rows than columns will write the result 
of the expression as row-wise, otherwise will be written as column-wise.
"""
		expression = GetTextFromUser(msg, "Enter an expression")
		if expression == "":
			raise GotoLabel("")
		
		result = eval(expression, CommandWindowDict)
		assert isinstance(result, _np.ndarray|list|dict|str|int|float), "expected ndarray|list|dict|str|int|float"

		rng = ws.selection()
		ncols, nrows = 0, 0
		if rng!=None:
			ncols, nrows = rng.ncols(), rng.nrows()
		
		CurRow, CurCol = ws.cursor()
		
		if isinstance(result, list|_np.ndarray):
			if isinstance(result, list):
				arr = _np.array(result)
			else:
				arr = result
				
			assert arr.ndim<=2, "Max dimension of the list can be 2."

			if arr.ndim == 2:
				ws.writelist2d(arr.tolist(), CurRow, CurCol)
			else:
				ws.writelist(arr.tolist(), CurRow, CurCol, rowmajor=(nrows>=ncols))
		
		elif isinstance(result, dict):
			ws.writedict(result, CurRow, CurCol, rowmajor=(nrows>=ncols))
		
		elif isinstance(result, int|float):
			ws[CurRow, CurCol] = str(result)
		
		elif isinstance(result, str):
			ws.writestr(result, CurRow, CurCol, rowmajor=(nrows>=ncols))
		
	except GotoLabel:
		pass
	except Exception as e:
		messagebox(str(e), "Import Error!")
