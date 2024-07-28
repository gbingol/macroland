import wx
from _sci import (Workbook, CommandWindowDict, messagebox)
import numpy as _np



class GotoLabel(Exception): 
	"""Serves as a label"""
	pass


class dlgExpressionEval ( wx.Dialog ):

	def __init__( self, parent ):
		super().__init__ ( parent, title = "Enter expressions", style = wx.STAY_ON_TOP )
		
		msg = """
Enter a valid expression that returns ndarray|list|dict|str|int|float.

IMPORTANT: Any variables used in the expression must already be defined in the command window.

TIP: For 1 dimensional containers, selecting more rows than columns will write the result 
of the expression as row-wise, otherwise will be written as column-wise.
"""
		self.m_stTxt = wx.StaticText( self, label=msg )

		self.m_scintilla = wx.stc.StyledTextCtrl( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, 0)
		self.m_scintilla.SetUseTabs ( True )
		self.m_scintilla.SetTabWidth ( 4 )
		self.m_scintilla.SetIndent ( 4 )
		self.m_scintilla.SetTabIndents( True )
		self.m_scintilla.SetBackSpaceUnIndents( True )
		self.m_scintilla.SetViewEOL( False )
		self.m_scintilla.SetViewWhiteSpace( True )
		self.m_scintilla.SetMarginWidth( 2, 0 )
		self.m_scintilla.SetIndentationGuides( True )
		self.m_scintilla.SetReadOnly( False )
		self.m_scintilla.SetMarginWidth( 1, 0 )
		self.m_scintilla.SetMarginType( 0, wx.stc.STC_MARGIN_NUMBER )
		self.m_scintilla.SetMarginWidth( 0, self.m_scintilla.TextWidth( wx.stc.STC_STYLE_LINENUMBER, "_99999" ) )
		self.m_scintilla.MarkerDefine( wx.stc.STC_MARKNUM_FOLDER, wx.stc.STC_MARK_BOXPLUS )
		self.m_scintilla.MarkerSetBackground( wx.stc.STC_MARKNUM_FOLDER, wx.BLACK)
		self.m_scintilla.MarkerSetForeground( wx.stc.STC_MARKNUM_FOLDER, wx.WHITE)
		self.m_scintilla.MarkerDefine( wx.stc.STC_MARKNUM_FOLDEROPEN, wx.stc.STC_MARK_BOXMINUS )
		self.m_scintilla.MarkerSetBackground( wx.stc.STC_MARKNUM_FOLDEROPEN, wx.BLACK )
		self.m_scintilla.MarkerSetForeground( wx.stc.STC_MARKNUM_FOLDEROPEN, wx.WHITE )
		self.m_scintilla.MarkerDefine( wx.stc.STC_MARKNUM_FOLDERSUB, wx.stc.STC_MARK_EMPTY )
		self.m_scintilla.MarkerDefine( wx.stc.STC_MARKNUM_FOLDEREND, wx.stc.STC_MARK_BOXPLUS )
		self.m_scintilla.MarkerSetBackground( wx.stc.STC_MARKNUM_FOLDEREND, wx.BLACK )
		self.m_scintilla.MarkerSetForeground( wx.stc.STC_MARKNUM_FOLDEREND, wx.WHITE )
		self.m_scintilla.MarkerDefine( wx.stc.STC_MARKNUM_FOLDEROPENMID, wx.stc.STC_MARK_BOXMINUS )
		self.m_scintilla.MarkerSetBackground( wx.stc.STC_MARKNUM_FOLDEROPENMID, wx.BLACK)
		self.m_scintilla.MarkerSetForeground( wx.stc.STC_MARKNUM_FOLDEROPENMID, wx.WHITE)
		self.m_scintilla.MarkerDefine( wx.stc.STC_MARKNUM_FOLDERMIDTAIL, wx.stc.STC_MARK_EMPTY )
		self.m_scintilla.MarkerDefine( wx.stc.STC_MARKNUM_FOLDERTAIL, wx.stc.STC_MARK_EMPTY )
		self.m_scintilla.SetSelBackground( True, wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT ) )
		self.m_scintilla.SetSelForeground( True, wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHTTEXT ) )

		sdbSizer = wx.StdDialogButtonSizer()
		self.m_btnOK = wx.Button( self, wx.ID_OK )
		sdbSizer.AddButton( self.m_btnOK )
		self.m_btnCancel = wx.Button( self, wx.ID_CANCEL )
		sdbSizer.AddButton( self.m_btnCancel )
		sdbSizer.Realize()

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( self.m_stTxt, 0, wx.ALL|wx.EXPAND, 5 )
		szrMain.Add( self.m_scintilla, 1, wx.EXPAND |wx.ALL, 5 )
		szrMain.Add( sdbSizer, 0, wx.EXPAND|wx.ALIGN_CENTER_HORIZONTAL, 5 )

		self.SetSizer( szrMain )
		self.Layout()

		self.Centre( wx.BOTH )



if __name__ == '__main__':
	expression:str = None
	try:
		ws = Workbook().activeworksheet()

		msg = """
Enter a valid expression that returns ndarray|list|dict|str|int|float.

IMPORTANT: Any variables used in the expression must already be defined in the command window.

TIP: For 1 dimensional containers, selecting more rows than columns will write the result 
of the expression as row-wise, otherwise will be written as column-wise.
"""
		expression:str = wx.GetTextFromUser(msg, "Enter an expression")
		expression = expression.rstrip().lstrip()
		if expression == "":
			raise GotoLabel("")
		
		result = eval(expression, CommandWindowDict)
		if result == None:
			raise GotoLabel("")
		
		assert isinstance(result, _np.ndarray|list|dict|str|int|float), "expected ndarray|list|dict|str|int|float"

		rng = ws.selection()
		ncols, nrows = 0, 0
		if rng!=None:
			ncols, nrows = rng.ncols(), rng.nrows()
		
		CurRow, CurCol = ws.cursor()
		
		if isinstance(result, list|_np.ndarray):
			if isinstance(result, list):
				arr = _np.array(result)
			else:
				arr = result
				
			assert arr.ndim<=2, "Max dimension of the list can be 2."

			if arr.ndim == 2:
				ws.writelist2d(arr.tolist(), CurRow, CurCol)
			else:
				ws.writelist(arr.tolist(), CurRow, CurCol, rowmajor=(nrows>=ncols))
		
		elif isinstance(result, dict):
			ws.writedict(result, CurRow, CurCol, rowmajor=(nrows>=ncols))
		
		elif isinstance(result, int|float):
			ws[CurRow, CurCol] = str(result)
		
		elif isinstance(result, str):
			ws.writestr(result, CurRow, CurCol, rowmajor=(nrows>=ncols))
		
	except GotoLabel:
		pass
	except Exception as e:
		ErrMsg = str(e)
		try:
			exec(expression, CommandWindowDict)
		except Exception as e2:
			ErrMsg += "\n"
			ErrMsg += "Then attempted using exec.\n"
			ErrMsg += str(e2)
			messagebox(ErrMsg, "Import error Error!")
		
