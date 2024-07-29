import dataclasses
import typing
import wx
from _sci import (Workbook, CommandWindowDict, ScintillaCtrl, messagebox)
import numpy as _np



@dataclasses.dataclass
class EvalResult:
	Success:bool = False
	ErrMsg: str =""
	Value:typing.Any = None

def _evalexpression(expr:str)->EvalResult:
	try:
		expression = expr.rstrip().lstrip()
		if expression == "":
			return EvalResult(Success=False, ErrMsg="", Value=None)
		
		result = eval(expression, CommandWindowDict)
		if result == None:
			return EvalResult(Success=False, ErrMsg="", Value=None)
		
		if not isinstance(result, _np.ndarray|list|dict|str|int|float):
			return EvalResult(Success=False, ErrMsg=f"{expression} yielded unexpected return type", Value=None)
		
		if isinstance(result, list|_np.ndarray):
			if isinstance(result, list):
				arr = _np.array(result)
			else:
				arr = result	
			if not arr.ndim<=2:
				return EvalResult(Success=False, ErrMsg=f"{expression} yielded more than 2D return type", Value=None)
			
			return EvalResult(Success=True, ErrMsg="", Value=arr.tolist())
		
		elif isinstance(result, dict|str|int|float):
			return EvalResult(Success=True, ErrMsg="", Value=result)
		
	except Exception as e:
		try:
			exec(expression, CommandWindowDict)
			return EvalResult(Success=True, ErrMsg="", Value=None)
		except Exception as e2:
			return EvalResult(Success=False, ErrMsg=f"statement yielded {str(e)}", Value=None)



def _WriteResult(result: typing.Any, row:int, col:int, rowmajor=False)->tuple[int, int]:
	ws = Workbook().activeworksheet()
	
	if isinstance(result, list|_np.ndarray):
		if isinstance(result, list):
			arr = _np.array(result)
		else:
			arr = result

		if arr.ndim == 2:
			return ws.writelist2d(arr.tolist(), row, col)
		else:
			return ws.writelist(arr.tolist(), row, col, rowmajor=rowmajor)
		
	elif isinstance(result, dict):
		return ws.writedict(result, row, col, rowmajor=rowmajor)
		
	elif isinstance(result, int|float):
		ws[row, col] = str(result)
		return row, col
		
	elif isinstance(result, str):
		return ws.writestr(result, row, col, rowmajor=rowmajor)




class dlgExpressionEval ( wx.Dialog ):

	def __init__( self, parent ):
		super().__init__ ( 
				parent, 
				title = "Enter expressions", 
				style = wx.DEFAULT_DIALOG_STYLE|wx.STAY_ON_TOP|wx.RESIZE_BORDER  )
		
		msg = """
Enter expressions which returns ndarray|list|dict|str|int|float or assignments.

IMPORTANT: 
1) Line by line evaluation is performed.
2) Assignment (myvar=1) places the variable (myvar) in the command window's dictionary.
3) Variables used in an expression must already be defined in the command window's dictionary.

TIP: 
If applicable, for 1 dimensional containers, selecting more rows than columns will write the result 
of the expression as row-wise, otherwise will be written as column-wise.
"""
		self._stTxt = wx.StaticText( self, label=msg )
		self._sc = ScintillaCtrl( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, 0)	

		sdbSizer = wx.StdDialogButtonSizer()
		self.m_btnOK = wx.Button( self, wx.ID_OK )
		sdbSizer.AddButton( self.m_btnOK )
		self.m_btnCancel = wx.Button( self, wx.ID_CANCEL )
		sdbSizer.AddButton( self.m_btnCancel )
		sdbSizer.Realize()

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( self._stTxt, 0, wx.ALL|wx.EXPAND, 5 )
		szrMain.Add( self._sc, 1, wx.EXPAND |wx.ALL, 5 )
		szrMain.Add( sdbSizer, 0, wx.EXPAND|wx.ALL, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()

		self.Centre( wx.BOTH )

		self.m_btnOK.Bind(wx.EVT_BUTTON, self.__OnOK)


	def __OnOK(self, event:wx.CommandEvent):
		ws = Workbook().activeworksheet()
		CurRow, CurCol = ws.cursor()
		rng = ws.selection()
		ncols, nrows = 0, 0
		
		if rng!=None:
			ncols, nrows = rng.ncols(), rng.nrows()
		
		Text = str(self._sc.GetText())
		Lines = Text.splitlines()
		Lines = list(filter(None, Lines))

		Results:list[EvalResult] = []
		for s in Lines:
			result = _evalexpression(s)
			if result.Success == True and result.Value != None:
				Results.append(result)

		rowmajor = nrows>=ncols
		
		row, col = CurRow, CurCol
		for result in Results:
			row, col = _WriteResult(result.Value, row, col, rowmajor=rowmajor)
			row += 1




if __name__ == '__main__':
	d = dlgExpressionEval(None)
	d.ShowModal()
	
		
		
