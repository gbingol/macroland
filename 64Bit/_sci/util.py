import os
import pkgutil
import sys as _sys
import math as _math
from pathlib import Path as _Path


__doc__="util module of scisuit. Does not use any modules other than Python's builtins."




def prettify(v:float, pretty=True)->str:
	"""
	Based on the value of v, a prettier string representation is returned
	pretty=False, just returns string representation of v
	"""
	if not isinstance(v, float) or not pretty:
		return str(v)

	num = abs(v)
	if num<1E-12:
		return "0.0"
	
	NDigits = _math.ceil(abs(_math.log10(num)))

	if 1<num<1E5:
		return str(round(v, 5- NDigits))

	elif num>=1E5:
		return "{:.4E}".format(v)
	
	if 0<num<1:
		if NDigits<=4:
			return str(round(v, NDigits+2))
		
		return "{:.3E}".format(v)
	
	#cannot prettify
	return str(v)




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
	Name: package name, (wx)  
	pip: pip install name (wxPython)  

	## Return: 
	if package is already installed returns True  

	If missing, prompts the user (messagebox) to install the package,  
	if user clicks No, returns False,  
	if user clicks Yes, installation process begins and function returns True
	"""

	#package already installed
	x = pkgutil.iter_modules()
	for i in x:
		if i.ispkg and i.name == name:
			return True
	
	PyHome = _sys.exec_prefix
	PyExe = PyHome + os.sep + "python.exe"
	Cmd = "\"" + PyExe + "\""
	Cmd += " -m pip install " + pip

	Msg = name + " is missing. Wanna install? \n \n"
	Msg += "Choosing Yes will launch the OS terminal using the following command: \n \n"
	
	Msg += Cmd + "\n \n"

	Msg += "If No, you might have to manually install the package(s)."

	from .framework import Framework
	YesNo = Framework().messagebox(Msg, "Install " + name + "?", yesno=True)
	if not YesNo:
		return False

	os.system(Cmd)

	return True




def listOutdatedPkgs()->list[dict]:
	"""
	Returns a list of outdated Python packages by running pip command using subprocess.

	Each dict in the list has the format:  
	{"name": "scisuit", "version": "1.3.5", "latest_version": "1.3.6", "latest_filetype": "wheel"}
	"""
	import subprocess
	import json
	PyHome = _sys.exec_prefix
	PyExe = PyHome + os.sep + "python.exe"
	result = subprocess.run([PyExe, "-m", "pip", "list", "--outdated", "--format=json"], check=True, capture_output=True)
	jsonres = result.stdout.decode('utf-8')
	lst = json.load(jsonres)
	return lst



def parent_path(path:str, level = 0)->_Path:
	"""
	This is used so many times that a convenience function is deemed as appropriate.

	path: Relative or full path 
	
	if path is C:\\a\\b\\c.py  
	level=0 => C:\\a\\b   
	level=1 => C:\\a
	"""
	pt = _Path(path)

	if pt.is_absolute() == False:
		pt = pt.absolute()

	return pt.parents[level]


if __name__ == '__main__':
	x=1
	print(colnum2label(x))