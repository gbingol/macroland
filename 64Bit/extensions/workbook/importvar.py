import keyword
import builtins
import types

import wx
import wx.stc as stc
from _sci import (Workbook, CommandWindowDict, messagebox)
import numpy as _np



class GotoLabel(Exception): 
	"""Serves as a label"""
	pass


class frmExpressionEval ( wx.Dialog ):

	def __init__( self, parent ):
		super().__init__ ( parent, title = "Enter expressions", style = wx.STAY_ON_TOP|wx.RESIZE_BORDER  )
		
		msg = """
Enter a valid expression that returns ndarray|list|dict|str|int|float.

IMPORTANT: Any variables used in the expression must already be defined in the command window.

TIP: For 1 dimensional containers, selecting more rows than columns will write the result 
of the expression as row-wise, otherwise will be written as column-wise.
"""
		self._stTxt = wx.StaticText( self, label=msg )

		self._sc = stc.StyledTextCtrl( self, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, 0)
		self._sc.SetBufferedDraw(True)
		self._sc.StyleClearAll()
		self._sc.SetLexer(stc.STC_LEX_PYTHON)
		self._sc.SetWordChars("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMONPQRSTUVWXYZ_")

		builtinfuncnames = [name for name, obj in vars(builtins).items() 
                          if isinstance(obj, types.BuiltinFunctionType)]
		self._sc.SetKeyWords(0,  " ".join([str(e) for e in keyword.kwlist])) #
		self._sc.SetKeyWords(1, " ".join([str(e) for e in builtinfuncnames])); #Keywords
		self._sc.SetUseTabs ( True )
		self._sc.SetTabWidth ( 4 )
		self._sc.SetIndent ( 4 )
		self._sc.SetTabIndents( True )
		self._sc.SetBackSpaceUnIndents( True )
		self._sc.SetViewEOL( False )
		self._sc.SetViewWhiteSpace( True )
		self._sc.SetMarginWidth( 2, 0 )
		self._sc.SetIndentationGuides( True )
		self._sc.SetReadOnly( False )
		self._sc.SetMarginWidth( 1, 0 )
		self._sc.SetMarginType( 0, stc.STC_MARGIN_NUMBER )
		self._sc.SetMarginWidth( 0, self._sc.TextWidth( stc.STC_STYLE_LINENUMBER, "_99999" ) )
		self._sc.MarkerDefine( stc.STC_MARKNUM_FOLDER, stc.STC_MARK_BOXPLUS )
		self._sc.MarkerSetBackground( stc.STC_MARKNUM_FOLDER, wx.BLACK)
		self._sc.MarkerSetForeground( stc.STC_MARKNUM_FOLDER, wx.WHITE)
		self._sc.MarkerDefine( stc.STC_MARKNUM_FOLDEROPEN, stc.STC_MARK_BOXMINUS )
		self._sc.MarkerSetBackground( stc.STC_MARKNUM_FOLDEROPEN, wx.BLACK )
		self._sc.MarkerSetForeground( stc.STC_MARKNUM_FOLDEROPEN, wx.WHITE )
		self._sc.MarkerDefine( stc.STC_MARKNUM_FOLDERSUB, stc.STC_MARK_EMPTY )
		self._sc.MarkerDefine( stc.STC_MARKNUM_FOLDEREND, stc.STC_MARK_BOXPLUS )
		self._sc.MarkerSetBackground( stc.STC_MARKNUM_FOLDEREND, wx.BLACK )
		self._sc.MarkerSetForeground( stc.STC_MARKNUM_FOLDEREND, wx.WHITE )
		self._sc.MarkerDefine( stc.STC_MARKNUM_FOLDEROPENMID, stc.STC_MARK_BOXMINUS )
		self._sc.MarkerSetBackground( stc.STC_MARKNUM_FOLDEROPENMID, wx.BLACK)
		self._sc.MarkerSetForeground( stc.STC_MARKNUM_FOLDEROPENMID, wx.WHITE)
		self._sc.MarkerDefine( stc.STC_MARKNUM_FOLDERMIDTAIL, stc.STC_MARK_EMPTY )
		self._sc.MarkerDefine( stc.STC_MARKNUM_FOLDERTAIL, stc.STC_MARK_EMPTY )
		self._sc.SetSelBackground( True, wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT ) )
		self._sc.SetSelForeground( True, wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHTTEXT ) )

		grey = wx.Colour(128, 128, 128)
		self._sc.MarkerDefine(stc.STC_MARKNUM_FOLDEROPEN, stc.STC_MARK_BOXMINUS, wx.WHITE, grey)
		self._sc.MarkerDefine(stc.STC_MARKNUM_FOLDER, stc.STC_MARK_BOXPLUS, wx.WHITE, grey)
		self._sc.MarkerDefine(stc.STC_MARKNUM_FOLDERSUB, stc.STC_MARK_VLINE, wx.WHITE, grey)
		self._sc.MarkerDefine(stc.STC_MARKNUM_FOLDERTAIL, stc.STC_MARK_LCORNER, wx.WHITE, grey)
		self._sc.MarkerDefine(stc.STC_MARKNUM_FOLDEREND, stc.STC_MARK_BOXPLUSCONNECTED, wx.WHITE, grey)
		self._sc.MarkerDefine(stc.STC_MARKNUM_FOLDEROPENMID, stc.STC_MARK_BOXMINUSCONNECTED, wx.WHITE, grey)
		self._sc.MarkerDefine(stc.STC_MARKNUM_FOLDERMIDTAIL, stc.STC_MARK_TCORNER, wx.WHITE, grey)

		
		self._sc.MarkerDefine(1, stc.STC_MARK_ROUNDRECT, wx.WHITE, wx.RED)
		self._sc.MarkerDefine(2, stc.STC_MARK_ARROW, wx.BLACK, wx.GREEN)

		BG = wx.Colour(255, 255, 255)
		Font = wx.Font(11, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, "Consolas")

		IDs = [stc.STC_P_COMMENTBLOCK, stc.STC_P_COMMENTLINE, stc.STC_P_TRIPLEDOUBLE, stc.STC_P_STRING, 
		 	stc.STC_P_CHARACTER, stc.STC_P_OPERATOR, stc.STC_P_IDENTIFIER, stc.STC_P_WORD, 
			stc.STC_P_WORD2, stc.STC_P_DEFNAME, stc.STC_P_CLASSNAME, stc.STC_P_DECORATOR, stc.STC_P_NUMBER]

		for id in IDs:
			self._sc.StyleSetBackground(id, BG)
			self._sc.StyleSetFont(id, Font)
		
		comments_FG = wx.Colour(127, 127, 127) 
		comments_BG = BG
		comments_Font = wx.Font(11, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_ITALIC, wx.FONTWEIGHT_NORMAL, False, "Consolas")

		strings_FG = wx.Colour(0, 127, 0)
		strings_Font = wx.Font(11, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_ITALIC, wx.FONTWEIGHT_NORMAL, False, "Consolas")

		ReservedWords_FG = wx.Colour(14, 1, 126)
		ReservedWords_Font = wx.Font(11, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, "Consolas")

		self._sc.StyleSetForeground(stc.STC_P_COMMENTBLOCK, comments_FG)
		self._sc.StyleSetFont(stc.STC_P_COMMENTBLOCK, comments_Font)

		self._sc.StyleSetForeground(stc.STC_P_COMMENTLINE, comments_FG)
		self._sc.StyleSetFont(stc.STC_P_COMMENTLINE, comments_Font)

		#triple quote for __doc__
		self._sc.StyleSetForeground(stc.STC_P_TRIPLEDOUBLE, comments_FG)
		self._sc.StyleSetFont(stc.STC_P_TRIPLEDOUBLE, comments_Font)

		#double quote
		self._sc.StyleSetForeground(stc.STC_P_STRING, strings_FG)
		self._sc.StyleSetFont(stc.STC_P_STRING, strings_Font)

		#single quote
		self._sc.StyleSetForeground(stc.STC_P_CHARACTER, strings_FG)
		self._sc.StyleSetFont(stc.STC_P_CHARACTER, strings_Font)

		self._sc.StyleSetForeground(stc.STC_P_OPERATOR, wx.Colour(0, 0, 0))
		self._sc.StyleSetForeground(stc.STC_P_IDENTIFIER, wx.Colour(0, 0, 0))

		#reserved words
		self._sc.StyleSetForeground(stc.STC_P_WORD, ReservedWords_FG)
		self._sc.StyleSetFont(stc.STC_P_WORD, ReservedWords_Font)

		#builtins words
		self._sc.StyleSetForeground(stc.STC_P_WORD2, wx.Colour(153, 153, 0))

		self._sc.StyleSetForeground(stc.STC_P_DEFNAME, wx.Colour(70, 130, 180)) #steel blue
		self._sc.StyleSetForeground(stc.STC_P_CLASSNAME, wx.Colour(32, 178, 170))
		self._sc.StyleSetForeground(stc.STC_P_DECORATOR, wx.Colour(153, 153, 0)) #dark yellow
		self._sc.StyleSetForeground(stc.STC_P_NUMBER, wx.Colour(127, 0, 0))

		

		self._sc.SetCaretLineVisible(True)

		CaretLineBG = wx.Colour(230, 235, 235)
		self._sc.SetCaretLineBackground(CaretLineBG)

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

		self.SetSizer( szrMain )
		self.Layout()

		self.Centre( wx.BOTH )



if __name__ == '__main__':
	d = frmExpressionEval(None)
	d.ShowModal()
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
		
