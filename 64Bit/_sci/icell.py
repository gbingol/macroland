from __future__ import annotations

import types as _types

from __SCISUIT import GUI as __gui # type: ignore


_CWorksheet = __gui.Worksheet
_CRange = __gui.Range
_BindFunction = __gui.Bind
_UnbindFunction = __gui.Unbind
_FindWorksheet = __gui.findworksheet
_WorksheetCount = __gui.worksheetcount


class Workbook:
	def __len__(self):
		return _WorksheetCount()


	@staticmethod
	def bind(event:str, func:_types.FunctionType, *args)->None:
		"""
		Binds a callback function

		event: name of the event \n
		func: A function that will be called when event happens \n
		args: Any parameter of the func.
		"""
		assert isinstance(event, str), "event argument must be string (event names)"
		assert isinstance(func, _types.FunctionType), "func argument must be function"

		eventNames = ["pagechanged"]
		assert event in eventNames, str(eventNames) + " are expected event names"

		_BindFunction(event, func, *args)
	
	
	@staticmethod
	def unbind(event:str, func:_types.FunctionType)->None:
		"""unbinds the function that was bound with given signature"""
		assert isinstance(event, str), "event argument must be string (event names)"
		assert isinstance(func, _types.FunctionType), "func argument must be function"

		_UnbindFunction(event, func)


	@staticmethod
	def findworksheet(param:str|int)->Worksheet|None:
		"""Finds the worksheet with given name (trailing and leading whitespaces are removed)"""
		assert isinstance(param, str|int), "param must be string|int."

		if isinstance(param, str):
			_name = param.rstrip()
			_name = _name.lstrip()
			return _FindWorksheet(_name)
		else:
			assert param>=0, "param >=0 expected"
			return _FindWorksheet(param)


	@staticmethod
	def activeworksheet()->Worksheet:
		"""returns the currently selected worksheet"""
		return Worksheet(active =True)




class Worksheet:
	def __init__(self, name="", nrows=1000, ncols=50, active = None) -> None:
		"""active: Any None object (only used by activeworksheet function)"""
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
	
	
	def getcellvalue(self, row:int, col:int)->str:
		"""returns the contents of the cell"""
		assert isinstance(row, int) and isinstance(col, int), "row and col must be int."
		assert row>=0 and col>=0, "row and col must be >=0" 
		
		return self._WS.getvalue(row, col)
	


	def setcellvalue(self, row:int, col:int, value:str):
		"""sets the contents of the cell"""
		assert isinstance(row, int) and isinstance(col, int), "row and col must be int."
		assert row>=0 and col>=0, "row and col must be >=0" 
		
		self._WS.setvalue(row, col, value)



	def setcellcolor(self, row:int, col:int, color:str, target="fg"):
		"""sets the color of foreground (fg) or background(bg) of the cell"""
		assert isinstance(row, int) and isinstance(col, int), "row and col must be int."
		assert row>=0 and col>=0, "row and col must be >=0" 
		assert isinstance(color, str), "color must be str."
		assert isinstance(target, str), "color must be str."
		assert target in ["fg", "bg"], "target values are: 'fg' or 'bg'"
		
		self._WS.setcellcolor(row, col, color, target)


	
	def getcellcolor(self, row:int, col:int, target="fg")->tuple[int, int, int]:
		"""gets the color (R, G, B) of foreground (fg) or background(bg) of the cell"""
		assert isinstance(row, int) and isinstance(col, int), "row and col must be int."
		assert row>=0 and col>=0, "row and col must be >=0" 
		assert isinstance(target, str), "color must be str."
		assert target in ["fg", "bg"], "target values are: 'fg' or 'bg'"
		
		return self._WS.getcellcolor(row, col, target)



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


	def bind(self, event:str, func:_types.FunctionType, *args)->None:
		"""binds a callback function"""
		assert isinstance(event, str), "event argument must be string (event names)"
		assert isinstance(func, _types.FunctionType), "func argument must be function"

		eventNames = ["selecting", "selected"]
		assert event in eventNames, str(eventNames) + " are expected event names"

		self._WS.bind(event, func, *args)


	def unbind(self, event:str, func:_types.FunctionType)->None:
		"""unbinds the function that was bound with given signature"""
		assert isinstance(event, str), "event argument must be string (event names)"
		assert isinstance(func, _types.FunctionType), "func argument must be function"

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
	
	def sel_coords(self)->tuple[tuple, tuple]|None:
		"""
		returns the selected areas coordinates (A1 is (0, 0)).

		The first tuple contains top-left's row and column numbers 
		The second tuple contains bottom-right's.

		If no selection is made, returns None.
		"""
		return self._WS.sel_coords()
	

	def writelist(self, values:list, row=0, col=0, rowmajor=True)->None:
		for value in values:
			if value != None:
				self._WS[row, col] = str(value)

			if rowmajor: row += 1
			else: col += 1
	

	def writelist2d(self, values:list[list], row=0, col=0)->None:
		for value in values:
				self.writelist(value, row, col, rowmajor=False)
				row += 1

	
	def writedict(self, values:dict, row=0, col=0, rowmajor=True)->None:
		r = 0 if rowmajor else 1
		c = 1 if rowmajor else 0
		
		for key, value in values.items():
			self._WS[row, col] = str(key)
			self._WS[row + r, col + c] = str(value) if value!=None else ""

			if rowmajor: row += 1
			else: col += 1


	def writestr(self, text:str, row:int, col:int, rowmajor=True)->None:
		lst = text.splitlines()
		self.writelist(lst, row, col, rowmajor)


class Range:
	def __init__(self, txt:str) -> None:
		self._txt = txt
		self._rng = _CRange(txt)

	def clear(self):
		"""Clears the range (contents and format)"""
		self._rng.clear()

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
