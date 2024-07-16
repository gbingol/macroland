import math as _math
import types as _types

from __SCISUIT import GUI as __gui # type: ignore
from __SCISUIT import COMMANDWINDOW as __cmd

Workbook = __gui.Workbook
_CWorksheet = __gui.Worksheet
_CRange = __gui.Range
CommandWindowDict:dict = __cmd.__dict__


class Range:
	pass



class Worksheet:
	def __init__(self, name="", nrows=1000, ncols=50, active = None) -> None:
		"""
		active: Any None object (only used by activeworksheet function)
		"""
		self._WS = _CWorksheet(name=name, nrows=nrows, ncols=ncols, active=active)


	def __setitem__(self, key, value):
		"""
		key: (row, col)
		value can be: 
		1) dict->{value=, fgcolor=, bgcolor="R G B", style="italic|normal", weight="bold|normal", underline="single|none"}
		2) Any object (string value of the object is used)
		"""
		self._WS[key] = value

	def __getitem__(self, key)->str:
		"""key must be row, col"""
		return self._WS[key]


	def appendcols(self, n=1)->bool:
		"""appends columns"""
		assert isinstance(n, int), "n must be int."
		assert n>0, "n>0 expected."

		return self._WS.appendcols(n)
	

	def appendrows(self, n=1)->bool:
		"""appends rows"""
		assert isinstance(n, int), "n must be int."
		assert n>0, "n>0 expected."

		return self._WS.appendrows(n)


	def bind(self, *args)->None:
		"""binds a callback function"""
		assert len(args)>=2, "At least 2 arguments expected"
		assert isinstance(args[0], str), "1st argument must be string (event names)"
		assert isinstance(args[1], _types.FunctionType), "2nd argument must be function"

		eventNames = ["selecting", "selected"]
		assert args[0] in eventNames, str(eventNames) + " are expected event names"

		self._WS.bind(*args)


	def unbind(self, event:str, func:_types.FunctionType)->None:
		"""unbinds the function that was bound with given signature"""
		self._WS.unbind(event, func)


	def cursor(self)->tuple[int, int]:
		"""returns the row and column coords of grid cursor"""
		return self._WS.cursor()
	

	def name(self)->str:
		"""returns the name of the worksheet"""
		return self._WS.name()
	

	def ncols(self)->int:
		"""returns the number of columns"""
		return self._WS.ncols()
	

	def nrows(self)->int:
		"""returns the number of rows"""
		return self._WS.nrows()
	

	def selection(self)->Range:
		"""returns the selected cells as Range object"""
		return self._WS.selection()
	

	def print_list(self, values:list|list[list], row=0, col=0, rowmajor=True)->None:

		for value in values:
			if isinstance(value, list):
				self.print_list(value, row, col, rowmajor=False)
				row += 1
			else:
				self._WS[row, col] = str(value)

				if rowmajor: row += 1
				else: col += 1





class Range:
	def __init__(self, txt:str) -> None:
		self._txt = txt
		self._rng = _CRange(txt)

	def col(self, pos:int)->list:
		"""returns the column as Python list"""
		assert isinstance(pos, int), "pos must be int."
		assert pos>=0, "pos>0 expected."

		return self._rng.col(pos)
	
	def coords(self)->tuple[tuple, tuple]:
		"""returns the top-left and bottom-right coordinates"""
		return self._rng.coords()
	
	def ncols(self)->int:
		"""returns the number of columns"""
		return self._rng.ncols()

	def nrows(self)->int:
		"""returns the number of rows"""
		return self._rng.nrows()
	
	def parent(self)->Worksheet:
		"""returns the Worksheet which owns the Range"""
		return self._rng.parent()
	
	def select(self)->None:
		"""selects the range"""
		self._rng.select()

	def subrange(self, row:int, col:int, nrows=-1, ncols=-1)->Range:
		"""returns a range defined within current range"""
		assert isinstance(row, int), "row must be int."
		assert isinstance(col, int), "col must be int."
		assert isinstance(nrows, int), "nrows must be int."
		assert isinstance(ncols, int), "ncols must be int."

		assert row>=0 and col>=0, "row and col >=0 expected."

		return self._rng.subrange(row, col, nrows, ncols)
	
	def tolist(self, axis=-1)->list|list[list]:
		"""
		returns the whole range as 1D/2D list

		axis=-1 -> 1D list \n
		axis=0 -> 2D list (row-wise collected)\n
		axis=1 -> 2D list (column-wise collected)
		"""
		assert isinstance(axis, int), "axis must be int."
		assert axis in [-1, 0, 1], "axis must be -1 or 0 or 1"
		
		return self._rng.tolist(axis)
	
	def todict(self, headers=True)->dict:
		"""returns the whole range as 1D/2D list"""
		return self._rng.todict(headers)




def activeworksheet()->Worksheet:
	"""returns the currently selected worksheet"""
	return Worksheet(active =True)


def statbar_write(text:str, n:int)->None:
	"""
	Writes the text to nth field in the status bar \n
	Note that n starts from 0.
	"""
	assert isinstance(text, str), "text must be string."
	assert isinstance(n, int), "n must be int."

	__gui.statbar_write(text, n)
	

def messagebox(msg:str, caption:str)->None:
	"""
	Display a messagebox
	Unlike wx.MessageBox, owner of the messagebox is the framework
	"""
	return __gui.messagebox(msg, caption)


def colnum2labels(num:int):
	"""
	Finds the corresponding letter to num.
	For example A=1, B=2, Z=26, AA=27, AB=28, ACR=772, BCK=1441
	"""

	assert num>0, "num must be greater than 0"
	assert isinstance(num, int), "num type must be int"

	Str =""

	NChars = int(_math.ceil(_math.log(num) / _math.log(26.0)))
	if NChars == 1:
		modular = num % 26
		if modular == 0: modular = 26
		Str = chr(65 + modular - 1)
		
		return Str
	
	val = num
	tbl = []
	for _ in range(NChars-1):
		val /= 26
		tbl.append(chr(65 + int(val % 26) - 1))
	
	tbl.reverse()
	Str = "".join(tbl)
	Str += chr(65 + int(num % 26) - 1)

	return Str



from .Frame import Frame
from .gridtext import GridTextCtrl
from .pnlOutputOptions import pnlOutputOptions
from .numtext import NumTextCtrl
from .makeicon import makeicon
from .pkg import assert_pkg
from .path import parent_path