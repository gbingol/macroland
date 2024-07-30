import dataclasses
import typing
import wx
from _sci import (Workbook, CommandWindowDict, ScintillaCtrl, wxmessagebox, Frame)
from _sci.temporary import SYS_IMPORTVARIABLEAPP
import numpy as _np



@dataclasses.dataclass
class EvalResult:
	Success:bool = False
	ErrMsg: str =""
	Value:typing.Any = None
	newrow:bool = False #Next result is to the new row
	dim:int = 0

def _evalexpression(expr:str)->EvalResult:
	NewRow = False
	try:
		expression = expr.rstrip().lstrip()
		if expression[-1] == ";":
			NewRow = True
			expression = expression.removesuffix(";")

		if expression == "":
			return EvalResult()
		
		result = eval(expression, CommandWindowDict)	
		
		if isinstance(result, list|_np.ndarray):
			if isinstance(result, list):
				arr = _np.array(result)
			else:
				arr = result	
			if not arr.ndim<=2:
				return EvalResult()
			
			return EvalResult(Success=True, Value=arr.tolist(), newrow=NewRow, dim=arr.ndim)
		
		elif isinstance(result, dict):
			return EvalResult(Success=True, Value=result, newrow=NewRow)
		
		elif isinstance(result, str):
			return EvalResult(Success=True, Value=result, newrow=NewRow)
		
		elif isinstance(result, int|float):
			return EvalResult(Success=True, Value=result, newrow=NewRow)
		
		else:
			return EvalResult(ErrMsg=f"{expression} yielded unexpected return type")
		
		
	except Exception as e:
		try:
			exec(expression, CommandWindowDict)
			return EvalResult(Success=True)
		except Exception as e2:
			return EvalResult(ErrMsg=f"statement yielded {str(e)}")



def _WriteResult(result:EvalResult, row:int, col:int, rowmajor=False)->tuple[int, int]:
	ws = Workbook().activeworksheet()
	
	if isinstance(result.Value, list):
		if result.dim == 2:
			return ws.writelist2d(result.Value, row, col)
		else:
			return ws.writelist(result.Value, row, col, rowmajor=rowmajor)
		
	elif isinstance(result.Value, dict):
		return ws.writedict(result.Value, row, col, rowmajor=rowmajor)
		
	elif isinstance(result.Value, int|float):
		ws[row, col] = str(result.Value)
		return row, col
		
	elif isinstance(result.Value, str):
		return ws.writestr(result.Value, row, col, rowmajor=rowmajor)




class frmExpressionEval ( Frame ):
	def __init__( self, parent ):
		super().__init__ ( 
				parent, 
				title = "Enter expressions or assignments", 
				style = wx.CAPTION|wx.CLOSE_BOX|wx.RESIZE_BORDER|wx.STAY_ON_TOP)
		
		msg = \
"""Enter expressions which returns ndarray|list|dict|str|int|float or assignments.

INPUT: 
1) Line by line evaluation is performed.
2) Assignment (myvar=1) places the variable (myvar) in the command window's dictionary.
3) Importing module (import numpy as np) places the variable (np) in the command window's dictionary.
4) Variables used in an expression must already be defined in the command window's dictionary.

OUTPUT:
1) Each output will be written to the consecutive columns.
2) Use ; at the end of an expression to write the next expression's output to a new row."""

		self._stTxt = wx.StaticText( self, label=msg )

		self._sc = ScintillaCtrl( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, 0)	
		
		History = SYS_IMPORTVARIABLEAPP["history"]
		if len(History)>0:
			self._sc.SetValue(History[-1])

		self._chk = wx.CheckBox(self, label="When possible print a container's content row-wise.")
		self._chk.SetValue(True)

		sdbSizer = wx.StdDialogButtonSizer()
		self.m_btnOK = wx.Button( self, wx.ID_OK )
		sdbSizer.AddButton( self.m_btnOK )
		self.m_btnCancel = wx.Button( self, wx.ID_CANCEL )
		sdbSizer.AddButton( self.m_btnCancel )
		sdbSizer.Realize()

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( self._stTxt, 0, wx.ALL|wx.EXPAND, 5 )
		szrMain.Add( self._chk, 0, wx.ALL|wx.EXPAND, 5 )
		szrMain.Add( self._sc, 1, wx.EXPAND |wx.ALL, 5 )
		szrMain.Add( sdbSizer, 0, wx.EXPAND|wx.ALL, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()
		self.Centre( wx.BOTH )

		self.m_btnOK.Bind(wx.EVT_BUTTON, self.__OnOK)
		self.m_btnCancel.Bind(wx.EVT_BUTTON, self.__OnCancel)

	
	def __OnCancel(self, event):
		self.Close()


	def __OnOK(self, event:wx.CommandEvent):
		try:
			ws = Workbook().activeworksheet()
			CurRow, CurCol = ws.cursor()
			
			Text = str(self._sc.GetText())
			Lines = Text.splitlines()
			Lines = list(filter(None, Lines))

			Results:list[EvalResult] = []
			for s in Lines:
				result = _evalexpression(s)
				if result.Success and result.Value != None:
					Results.append(result)

			row, col = CurRow, CurCol
			for result in Results:
				row, col = _WriteResult(result, row, col, rowmajor=self._chk.GetValue())
				
				if result.newrow: 
					row += 1
					col = CurCol
				else:
					col += 1
					row = CurRow
			
			SYS_IMPORTVARIABLEAPP["history"].append(self._sc.GetValue())
				
		except Exception as e:
			wxmessagebox(str(e))




if __name__ == '__main__':
	d = frmExpressionEval(None)
	d.Show()
	
		
		
