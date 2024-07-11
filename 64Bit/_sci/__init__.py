import math as _math

from __SCISUIT import GUI as __gui


Range = __gui.Range
Worksheet = __gui.Worksheet
Workbook = __gui.Workbook
activeworksheet = __gui.activeworksheet


from .Frame import Frame
from .gridtext import GridTextCtrl
from .pnlOutputOptions import pnlOutputOptions
from .numtext import NumTextCtrl
from .makeicon import makeicon
from .pkg import pkg_installed, assert_pkg
from .path import parent_path


def statbar_write(text:str, n:int)->None:
	"""
	Writes the text to nth field in the status bar \n
	Note that n starts from 0.
	"""
	__gui.statbar_write(text, n)
	

def messagebox(msg:str, caption:str)->None:
	"""
	Display a messagebox
	Unlike wx.MessageBox, owner of the messagebox is the framework
	"""
	__gui.messagebox(msg, caption)


def colnum2labels(num:int):
	"""
	Finds the corresponding letter to num.
	For example A=1, B=2, Z=26, AA=27, AB=28, ACR=772, BCK=1441
	"""

	assert num>0, "num must be greater than 0"
	if(type(num) != int):
		raise TypeError("num type must be int")

	Str =""

	NChars = int(_math.ceil(_math.log(num) / _math.log(26.0)))
	if NChars == 1:
		modular = num % 26
		if (modular == 0):
			modular = 26
		Str = chr(65 + modular - 1)
		return Str
	
	val = num
	tbl = []
	for i in range(NChars-1):
		val /= 26
		tbl.append(chr(65 + int(val % 26) - 1))
	
	tbl.reverse()
	Str = "".join(tbl)
	Str += chr(65 + int(num % 26) - 1)

	return Str