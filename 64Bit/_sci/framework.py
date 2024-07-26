from __SCISUIT import GUI as _gui # type: ignore


def statbar_write(text:str, n:int)->None:
	"""
	Writes the text to nth field in the status bar \n
	Note that n starts from 0.
	"""
	assert isinstance(text, str), "text must be string."
	assert isinstance(n, int), "n must be int."

	_gui.statbar_write(text, n)
	


def messagebox(msg:str, caption:str, yesno:bool = False)->None:
	"""
	Displays a messagebox. 

	Unlike wx.MessageBox, the owner of the messagebox is the framework and therefore 
	when its shown it blocks the framework.

	---

	msg: Message to be shown. \n
	caption: Caption of the messagebox dialog. \n
	yesno: Depending on the value shows different messageboxes.
	1) if True then shows a messagebox with Yes and No options. 
	   If user selects Yes then returns 1 otherwise returns 0.
	2) If False, then shows a messagebox with only OK button. 
	"""
	assert isinstance(msg, str), "msg must be string."
	assert isinstance(caption, str), "caption must be string."
	assert isinstance(yesno, bool), "yesno must be bool."
	return _gui.messagebox(msg, caption, yesno)