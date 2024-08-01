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
			return Worksheet(search=name)
		
		pos = ws
		assert pos>=0, "pos >=0 expected"
		return Worksheet(search=pos)


	@staticmethod
	def activeworksheet()->Worksheet:
		"""returns the currently selected worksheet"""
		return Worksheet(name ="", nrows=-1, ncols=-1)





#------------------------------------------------------------


class Worksheet:
	def __init__(self, name="", nrows=1000, ncols=50, search=None) -> None:
		"""
		search: For internal use to search an existing worksheet by name or by position.
		Please prefer to use Workbook().findworksheet function instead.
		"""
		if search != None:
			self._WS = _gui.Worksheet(search = search)	
		else:
			self._WS = _gui.Worksheet(name=name, nrows=nrows, ncols=ncols)


	def _isOK(self)->bool:
		"""
		Check if Worksheet is still usable. In most cases there is no need
		to use this member function.

		For Worksheet objects, this check is performed behind the scenes. However, 
		Range objects need to be aware if the owner Worksheet is still "ok". Therefore, this 
		member function has been added for internal use.
		"""
		return self._WS.isOK()


	def __str__(self):
		return self.name()


	def __eq__(self, other: object) -> bool:
		if not isinstance(other, Worksheet):
			return False
		
		return self.name() == other.name() and \
				self.ncols() == other.ncols() and \
				self.nrows() == other.nrows()



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
			if axis == 0:
				rows, cols = cols, rows

			assert rows.start>=0 and cols.start>=0, "slices' start must be >0"
			assert rows.stop!=None and cols.stop!=None, "slices' stop must be defined."
			assert rows.stop>rows.start and cols.stop>cols.start, "slices' stops > starts expected."

			#assert (rows.stop-rows.start)<self.nrows(), "Too many rows requested."
			#assert (cols.stop-cols.start)<self.ncols(), "Too many cols requested."

			retVal = []
			for r in range(rows.start, rows.stop, rows.step or 1):
				Arr = []
				for c in range(cols.start, cols.stop, cols.step or 1):
					val = self.getcellvalue(r, c) if axis in [-1, 1] else self.getcellvalue(c, r)
					Arr.append(val)

				retVal.append(Arr)
			
			#flatten the list
			if axis == -1: 
				return [item for row in retVal for item in row]
			
			return retVal
	
	
	def getcellvalue(self, row:int, col:int)->None|str|int|float:
		"""returns the contents of the cell"""
		assert isinstance(row, int) and isinstance(col, int), "row and col must be int."
		assert row>=0 and col>=0, "row and col must be >=0" 
		assert row<self.nrows() and col<self.ncols(), "row or column does not exist."

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
		assert row<self.nrows() and col<self.ncols(), "row or column does not exist."
		
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
	

	def select(self, tl:tuple[int, int], br:tuple[int, int]):
		"""
		selects the given coordinates

		---
		tl: Top-left coordinates \n
		br: Bottom-right coordinates
		"""
		self._WS.select(tl[0], tl[1], br[0], br[1])


	def selection(self)->Range|None:
		"""returns the selected cells as Range object"""
		coords = self.sel_coords()
		
		return Range(ws=self, tl=coords[0], br=coords[1]) if coords != None else None

	
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
				self.setcellvalue(row, col, str(value))

			if rowmajor: row += 1
			else: col += 1
		
		return row, col
	

	def writelist2d(self, values:list[list], row=0, col=0, rowmajor=False)->tuple[int, int]:
		for value in values:
			self.writelist(value, row, col, rowmajor=rowmajor)
			if rowmajor: col += 1
			else: row += 1
		
		return row, col


	
	def writedict(self, values:dict, row=0, col=0, rowmajor=True)->tuple[int, int]:
		r = 0 if rowmajor else 1
		c = 1 if rowmajor else 0
		
		for key, value in values.items():
			self[row, col] = str(key)
			self[row + r, col + c] = str(value) if value!=None else ""

			if rowmajor: row += 1
			else: col += 1
		
		return row, col


	def writestr(self, text:str, row:int, col:int, rowmajor=True)->tuple[int, int]:
		lst = text.splitlines()
		return self.writelist(lst, row, col, rowmajor)



#--------------------------------------------------------------------


class Range:
	def __init__(self, 
			txt:str|None=None,
			ws:Worksheet|None=None, 
			tl:tuple[int, int]|None=None, 
			br:tuple[int, int]|None=None) -> None:
		
		self._isOK = True
		self._txt=txt
		self._ws=ws
		self._TL=tl
		self._BR=br
		
		if txt!=None and isinstance(txt, str):
			self._txt = txt.rstrip().lstrip()
			if self._txt == "":
				return
			
			l = self._txt.split("!")
			if len(l)!=2:
				raise RuntimeError("Invalid range. Range string must contain '!' character.")
			
			wsname = l[0]
			self._ws = Workbook().findworksheet(wsname)
			if self._ws == None:
				raise RuntimeError("Invalid range. Worksheet does not exist.")

			#the string is now in the form of "A1:C3"
			s:str = l[1]
			s = s.rstrip().lstrip()
			l = s.split(":")
			if len(l)!=2:
				raise RuntimeError("Invalid range. Range string must contain ':'  after '!' character.")
			
			import re
			TLCoords = re.split("([a-zA-Z]+)", l[0])
			TLCoords = list(filter(None, TLCoords))
			if len(TLCoords)!=2:
				raise RuntimeError("Invalid range. After ! character, expected format is e.g. A1:B2 ")
			
			self._TL = (int(TLCoords[1])-1, label2colnum(TLCoords[0])-1)
			Row, Col = self._TL
			assert Row>=0, "top-left row >=0 expected."
			assert Col>=0, "top-left col >=0 expected."

			BRCoords = re.split("([a-zA-Z]+)", l[1])
			BRCoords = list(filter(None, BRCoords))
			if len(BRCoords)!=2:
				raise RuntimeError("Invalid range. After ! character, expected format is e.g. A1:B2 ")
			
			self._BR = (int(BRCoords[1])-1, label2colnum(BRCoords[0])-1)
			Row, Col = self._BR
			assert Row>=0, "bottom-right row >=0 expected."
			assert Col>=0, "bottom-right col >=0 expected."

			assert self._BR[0]>=self._TL[0], "bottom-right row >= top-left row expected."
			assert self._BR[1]>=self._TL[1], "bottom-right column >= top-left column expected."
		
		else:
			assert isinstance(ws, Worksheet), f"type{ws} is not acceptable, must be Worksheet" 
			assert isinstance(tl, tuple), "tl must be tuple(int, int)"
			assert isinstance(br, tuple), "br must be tuple(int, int)"
			self._ws = ws
			self._TL = tl
			self._BR = br
			self._txt = f"{self._ws.name()}!{colnum2label(self._TL[1]+1)}{self._TL[0]+1}:{colnum2label(self._BR[1]+1)}{self._BR[0]+1}"
		


	@staticmethod
	def merge(r1:Range, r2:Range)->Range:
		assert isinstance(r1, Range), "r1 must be Range object"
		assert isinstance(r2, Range), "r2 must be Range object"

		assert r1.parent() == r2.parent(), "r1 and r2 must belong to the same Worksheet"

		TL1, BR1 = r1.coords()
		TL2, BR2 = r2.coords()

		TL_row = min(TL1[0], TL2[0])
		TL_col = min(TL1[1], TL2[1])

		BR_row = max(BR1[0], BR2[0])
		BR_col = max(BR1[1], BR2[1])

		ws = r1.parent()

		return Range(ws=ws, tl=(TL_row, TL_col), br=(BR_row, BR_col))



	def __str__(self):
		assert self.parent()._isOK(), "Range is not usable anymore."
		return self._txt
	


	def clear(self):
		"""Clears the range (contents and format)"""
		assert self.parent()._isOK(), "Range is not usable anymore."

		tl, br = self._TL, self._BR
		for i in range(tl[0], br[0]+1):
			for j in range(tl[1], br[1]+1):
				self._ws[i,j] = None


	def col(self, pos:int)->list:
		"""returns the column as Python list"""
		assert self.parent()._isOK(), "Range is not usable anymore."
		
		assert isinstance(pos, int), "pos must be int."
		assert 0<= pos < self.ncols(), " 0<= pos < ncols() expected."

		return self.subrange(0, pos, ncols=1).tolist()
	

	def coords(self)->tuple[tuple, tuple]:
		"""returns the top-left and bottom-right coordinates"""
		assert self.parent()._isOK(), "Range is not usable anymore."
		return self._TL, self._BR
	

	def ncols(self)->int:
		"""returns the number of columns"""
		assert self.parent()._isOK(), "Range is not usable anymore."

		tl, br = self._TL, self._BR
		return br[1] - tl[1] + 1 


	def nrows(self)->int:
		"""returns the number of rows"""
		assert self.parent()._isOK(), "Range is not usable anymore."

		tl, br = self._TL, self._BR
		return br[0] - tl[0] + 1 
	

	def parent(self)->Worksheet:
		"""returns the Worksheet which owns the Range"""
		return self._ws
	

	def select(self)->None:
		"""selects the range"""
		assert self.parent()._isOK(), "Range is not usable anymore."
		self.parent().select(*self._TL, *self._BR)


	def subrange(
			self, 
			row:int, 
			col:int, 
			nrows:int=-1, 
			ncols:int=-1)->Range:
		
		"""returns a range defined within current range"""
		assert self.parent()._isOK(), "Range is not usable anymore."

		assert isinstance(row, int), "row must be int."
		assert isinstance(col, int), "col must be int."

		if nrows<0:
			nrows = self.nrows()-row
		
		if ncols<0:
			ncols = self.ncols()-col

		assert row>=0 and col>=0, "row and col >=0 expected."
		assert (row+nrows)<=self.nrows(), "rows exceeded"
		assert (col+ncols)<=self.ncols(), "cols exceeded"

		TL = (row + self._TL[0], col + self._TL[1])
		BR = (TL[0] + nrows -1, TL[1] + ncols -1)

		return Range(ws=self.parent(), tl=TL, br=BR)

	
	def tolist(self, axis=-1)->list|list[list]:
		"""
		returns the whole range as 1D/2D list

		axis=-1 -> 1D list \n
		axis=0 -> 2D list (row-wise collected)\n
		axis=1 -> 2D list (column-wise collected)
		"""
		assert self.parent()._isOK(), "Range is not usable anymore."

		assert isinstance(axis, int), "axis must be int."
		assert axis in [-1, 0, 1], "axis must be -1 or 0 or 1"

		tl, br = self._TL, self._BR
		tl_r, tl_c = tl
		br_r, br_c = br 
		
		if axis == -1:
			return self._ws[tl_r:br_r+1, tl_c:br_c+1]
		
		return self._ws[tl_r:br_r+1, tl_c:br_c+1, axis]
	

	def todict(self, headers=False)->dict:
		"""
		returns the whole range as dictionary.
		If headers is True, then the entry at first row of each column will be used 
		as a key for the corresponding column. 
		If headers=False a key (Col1, Col2, ...) will be generated as key.
		"""
		assert self.parent()._isOK(), "Range is not usable anymore."

		Arr:list[list] = self.tolist(axis=0)

		d ={}
		i=1
		for lst in Arr:
			key = lst[0] if headers else f"Col{i}"
			value = lst[1:] if headers else lst
			d[str(key)] = value
			i += 1
		
		return d
