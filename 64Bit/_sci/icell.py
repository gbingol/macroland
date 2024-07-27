from __future__ import annotations

import types as _types

from __SCISUIT import GUI as _gui # type: ignore
from .util import label2colnum, colnum2label



class Workbook:
	def __len__(self):
		return _gui.worksheetcount()


	@staticmethod
	def bind(event:str, func:_types.FunctionType, *args)->None:
		"""
		Binds a callback function

		---

		event: name of the event \n
		func: A function that will be called when event happens \n
		args: Any parameter of the func.
		"""
		assert isinstance(event, str), "event argument must be string (event names)"
		assert isinstance(func, _types.FunctionType), "func argument must be function"

		eventNames = ["pagechanged"]
		assert event in eventNames, str(eventNames) + " are expected event names"

		_gui.Bind(event, func, *args)
	
	
	@staticmethod
	def unbind(event:str, func:_types.FunctionType)->None:
		"""unbinds the function that was bound with given signature"""
		assert isinstance(event, str), "event argument must be string (event names)"
		assert isinstance(func, _types.FunctionType), "func argument must be function"

		_gui.Unbind(event, func)


	@staticmethod
	def findworksheet(ws:str|int)->Worksheet|None:
		"""
		Finds the worksheet with given name or position. 

		---
		Types of ws:
		1) str: It is the name of the worksheet. When searched the trailing and leading 
		   whitespaces are removed from the name.
		2) int: Position of the worksheet in the workbook
		
		"""
		assert isinstance(ws, str|int), "ws must be str or int."

		if isinstance(ws, str):
			name = ws
			name = name.rstrip()
			name = name.lstrip()
			return _gui.findworksheet(name)
		
		pos = ws
		assert pos>=0, "pos >=0 expected"
		return _gui.findworksheet(pos)


	@staticmethod
	def activeworksheet()->Worksheet:
		"""returns the currently selected worksheet"""
		return Worksheet(active =True)





#------------------------------------------------------------


class Worksheet:
	def __init__(self, name="", nrows=1000, ncols=50, active = None) -> None:
		"""active: Any None object (reserved for activeworksheet function)"""
		self._WS = _gui.Worksheet(name=name, nrows=nrows, ncols=ncols, active=active)


	def __str__(self):
		return self.name()

	def __setitem__(self, key, value):
		assert isinstance(key, tuple), "key must be tuple"
		assert len(key)>=2, "At least 2 keys expected"
		assert isinstance(key[0], int) and isinstance(key[1], int), "Both keys must be ints (row, col)"

		row, col = key[0], key[1]

		if value == None:
			self.setcellvalue(row, col, "")

		elif isinstance(value, int|float):
			self.setcellvalue(row, col, str(value))

		elif isinstance(value, list):
			if isinstance(value[0], list):
				self.writelist2d(value, row, col)
			else:
				self.writelist(value, row, col, rowmajor=len(key)==2)
		
		elif isinstance(value, dict):
			self.writedict(value, row, col, rowmajor=len(key)==2)

		elif isinstance(value, str):
			self.writestr(value, row, col, rowmajor=len(key)==2)
		
		else:
			raise TypeError("Expected types: int|float|list|dict|str")



	def __getitem__(self, key)->str:
		assert isinstance(key, tuple), "key must be tuple"
		assert len(key)>=2, "at least 2 keys expected"
		
		#ws[row, col], example ws[0,0]
		if len(key)==2 and isinstance(key[0], int) and isinstance(key[1], int):
			return self.getcellvalue(row=key[0], col=key[1])
		
		#ws[0:2, 1:4] or ws[0:2, 1:4, -1] 
		if len(key)>=2 and isinstance(key[0], slice) and isinstance(key[1], slice):
			axis = -1 if len(key)==2 else key[2]
			assert isinstance(axis, int), "last parameter (axis) must be int."
			assert -1<=axis<=1, "-1<=axis<=1 expected"
			
			rows:slice = key[0]
			cols:slice = key[1]

			assert rows.start>=0 and cols.start>=0, "slices' start must be >0"
			assert rows.stop!=None and cols.stop!=None, "slices' stop must be defined."
			assert rows.stop>rows.start and cols.stop>cols.start, "slices' stops > starts expected."

			assert (rows.stop-rows.start)<self.nrows(), "Too many rows requested."
			assert (cols.stop-cols.start)<self.ncols(), "Too many cols requested."

			retVal = []
			for i in range(rows.start, rows.stop, rows.step or 1):
				Arr = []
				for j in range(cols.start, cols.stop, cols.step or 1):
					val = self.getcellvalue(i, j) if axis == 1 else self.getcellvalue(j, i)
					Arr.append(val)

				retVal.append(Arr)
			
			#flatten the list
			if axis== -1: 
				return [item for row in retVal for item in row]
			
			return retVal
	
	
	def getcellvalue(self, row:int, col:int)->None|str|int|float:
		"""returns the contents of the cell"""
		assert isinstance(row, int) and isinstance(col, int), "row and col must be int."
		assert row>=0 and col>=0, "row and col must be >=0" 	
		txt:str = self._WS.getvalue(row, col)
		txt = txt.rstrip().lstrip()

		if txt == "": return None
		try:
			i = int(txt)
			return i
		except Exception:
			try:
				f = float(txt)
				return f
			except Exception:
				pass
	
		return txt
	


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


	def setcellitalic(self, row:int, col:int, italic=True):
		"""sets the cell style italic or normal"""
		assert isinstance(row, int) and isinstance(col, int), "row and col must be int."
		assert row>=0 and col>=0, "row and col must be >=0" 
		assert isinstance(italic, bool), "italic must be bool."
		
		self._WS.setcellfont(row, col, style=italic, weight=None, decoration=None)


	def setcellbold(self, row:int, col:int, bold=True):
		"""sets the cell weight bold or normal"""
		assert isinstance(row, int) and isinstance(col, int), "row and col must be int."
		assert row>=0 and col>=0, "row and col must be >=0" 
		assert isinstance(bold, bool), "bold must be bool."
		
		self._WS.setcellfont(row, col, style=None, weight=bold, decoration=None)


	def setcellunderlined(self, row:int, col:int, underlined=True):
		"""sets the cell decoration underlined or none."""
		assert isinstance(row, int) and isinstance(col, int), "row and col must be int."
		assert row>=0 and col>=0, "row and col must be >=0" 
		assert isinstance(underlined, bool), "underlined must be bool."
		
		self._WS.setcellfont(row, col, style=None, weight=None, decoration=underlined)


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
	

	def writelist(self, values:list, row=0, col=0, rowmajor=True)->tuple[int, int]:
		"""writes a list to worksheet, returns row and col position"""
		for value in values:
			if value != None:
				self._WS[row, col] = str(value)

			if rowmajor: row += 1
			else: col += 1
		
		return row, col
	

	def writelist2d(self, values:list[list], row=0, col=0)->tuple[int, int]:
		for value in values:
				self.writelist(value, row, col, rowmajor=False)
				row += 1
		
		return row, col


	
	def writedict(self, values:dict, row=0, col=0, rowmajor=True)->tuple[int, int]:
		r = 0 if rowmajor else 1
		c = 1 if rowmajor else 0
		
		for key, value in values.items():
			self._WS[row, col] = str(key)
			self._WS[row + r, col + c] = str(value) if value!=None else ""

			if rowmajor: row += 1
			else: col += 1
		
		return row, col


	def writestr(self, text:str, row:int, col:int, rowmajor=True)->tuple[int, int]:
		lst = text.splitlines()
		return self.writelist(lst, row, col, rowmajor)



#--------------------------------------------------------------------


class Range:
	def __init__(self, 
			ws:Worksheet|None=None, 
			tl:tuple[int, int]|None=None, 
			br:tuple[int, int]|None=None,  
			txt:str|None=None) -> None:
		
		if isinstance(txt, str):
			self._txt = txt.rstrip().lstrip()
			l = self._txt.split("!")
			if len(l)!=2:
				raise RuntimeError("Invalid range. Range string must contain '!' character.")
			
			wsname = l[0]
			self._ws = Workbook().findworksheet(wsname)
			if self._ws == None:
				raise RuntimeError("Invalid range. Worksheet does not exist.")

			#the string is now in the form of "A1:C3"
			s:str = l[1]
			l = s.split(":")
			if len(l)!=2:
				raise RuntimeError("Invalid range. Range string must contain ':'  after '!' character.")
			
			import re
			TLCoords = re.split("([a-zA-Z]+)", l[0])
			TLCoords = list(filter(None, TLCoords))
			if len(TLCoords)!=2:
				raise RuntimeError("Invalid range. After ! character, expected format is e.g. A1:B2 ")
			
			self._TL = (int(TLCoords[1]), label2colnum(TLCoords[0]))
			Row, Col = self._TL
			assert Row>=0, "top-left row >=0 expected."
			assert Col>=0, "top-left col >=0 expected."

			BRCoords = re.split("([a-zA-Z]+)", l[1])
			BRCoords = list(filter(None, BRCoords))
			if len(BRCoords)!=2:
				raise RuntimeError("Invalid range. After ! character, expected format is e.g. A1:B2 ")
			
			self._BR = (int(BRCoords[1]), label2colnum(BRCoords[0]))
			Row, Col = self._BR
			assert Row>=0, "bottom-right row >=0 expected."
			assert Col>=0, "bottom-right col >=0 expected."

			assert self._BR[0]>self._TL[0], "bottom-right row must be greater than top-left row"
			assert self._BR[1]>self._TL[1], "bottom-right column must be greater than top-left column"
		
		else:
			assert isinstance(ws, Worksheet), "ws must be worksheet"
			assert isinstance(tl, tuple), "tl must be tuple(int, int)"
			assert isinstance(br, tuple), "br must be tuple(int, int)"
			self._ws = ws
			self._TL = tl
			self._BR = br
			self._txt = f"{self._ws.name}! {colnum2label(self._TL[1])}{self._TL[0]}:{colnum2label(self._BR[1])}{self._BR[0]}"


	def clear(self):
		"""Clears the range (contents and format)"""
		tl, br = self._TL, self._BR
		for i in range(tl[0], br[0]+1):
			for j in range(tl[1], br[1]+1):
				self._ws[i,j]=None


	def col(self, pos:int)->list:
		"""returns the column as Python list"""
		assert isinstance(pos, int), "pos must be int."
		assert pos>=0, "pos>0 expected."

		return self._rng.col(pos)
	
	def coords(self)->tuple[tuple, tuple]:
		"""returns the top-left and bottom-right coordinates"""
		return self._TL, self._BR
	
	def ncols(self)->int:
		"""returns the number of columns"""
		tl, br = self._TL, self._BR
		return br[1] - tl[1] + 1 


	def nrows(self)->int:
		"""returns the number of rows"""
		tl, br = self._TL, self._BR
		return br[0] - tl[0] + 1 
	
	def parent(self)->Worksheet:
		"""returns the Worksheet which owns the Range"""
		return self._ws
	
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
