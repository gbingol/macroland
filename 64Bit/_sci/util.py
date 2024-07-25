import wx
import os
import pkgutil
import sys as _sys
import math as _math
from pathlib import Path as _Path



def colnum2label(num:int)->str:
	"""
	Finds the corresponding letter to the given number.
	For example A=1, B=2, Z=26, AA=27, AB=28, ACR=772, BCK=1441
	"""
	assert num>0, "num must be greater than 0"
	assert isinstance(num, int), "num type must be int"

	Str =""

	NChars = int(_math.ceil(_math.log(num) / _math.log(26.0)))
	if NChars == 1:
		modular = num % 26
		if modular == 0: modular = 26
		return chr(65 + modular - 1)
	
	val, tbl = num, []
	for _ in range(NChars-1):
		val /= 26
		tbl.append(chr(65 + int(val % 26) - 1))
	
	tbl.reverse()
	Str = "".join(tbl)
	Str += chr(65 + int(num % 26) - 1)

	return Str



def label2colnum(txt:str)->int:
	"""
	Finds the corresponding number to the given column label

	A=1, AB=27 ...
	"""
	_txt = txt.upper()
	retNum = 0
	length = len(_txt)
	n = 0

	for i in range(length - 1, -1, -1):
		val = ord(_txt[i]) - 65 + 1
		retNum += 26 ** n * val
		n += 1

	return retNum



def assert_pkg(name:str, pip:str)->bool:
	"""
	## Input: 
	Name: package name, (wx) \n
	pip: pip install name (wxPython) 

	## Return: 
	if package is already installed returns True \n

	If missing, prompts the user (wx.MessageBox) to install the package, \n
	if user clicks No, returns False, \n
	if user clicks Yes, installation process begins and function returns True
	"""

	#package already installed
	x = pkgutil.iter_modules()
	for i in x:
		if i.ispkg==True and i.name == name:
			return True
	
	PyHome = pyhomepath()
	PyExe = PyHome + os.sep + "python.exe"
	Cmd = "\"" + PyExe + "\""
	Cmd += " -m pip install " + pip

	Msg = name + " is missing. Wanna install? \n \n"
	Msg += "Choosing Yes will launch the terminal and installation process using the following command: \n \n"
	
	Msg += Cmd + "\n \n"

	Msg += "If you choose No, you might have to manually install the package to run the requiring app."

	YesNo = wx.MessageBox(Msg, "Install " + name + "?", wx.YES_NO)
	if YesNo == wx.NO:
		return False

	wx.Shell(Cmd)

	return True



def pyhomepath()->str:
	"""
	returns the Python Home Path
	"""
	return _sys.exec_prefix



def parent_path(path:str, level = 0)->_Path:
	"""
	This is used so many times that a convenience function is deemed appropriate.

	## Input: 
	path: Relative or full path 
	
	## Example:
	if path is C:\\a\\b\\c.py  \n
	level=0 => C:\\a\\b \n
	level=1 => C:\\a
	"""
	pt = _Path(path)

	if pt.is_absolute() == False:
		pt = pt.absolute()

	return pt.parents[level]


if __name__ == '__main__':
	x="AZ"
	print(label2colnum(x))