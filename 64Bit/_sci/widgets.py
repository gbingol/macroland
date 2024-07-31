import builtins
import keyword
import string
import types
from cmath import inf


import wx
import wx.stc as _stc


from .icell import Workbook, Worksheet, Range
from .framework import Framework


def _GetVariable(txt):
	ws = Workbook().activeworksheet()
	rng = ws.selection()
	txt.SetValue(str(rng))


def _OnPageChanged(self):
	self.m_Worksheet.unbind("selecting", _GetVariable)
		
	self.m_Worksheet = Workbook().activeworksheet()
	self.m_Worksheet.bind("selecting", _GetVariable, self.m_textCtrl)




class _frmGridSelection (wx.Frame):
	def __init__(self, parent):
		super().__init__(parent, style=wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP )

		self.SetSizeHints(wx.DefaultSize, wx.DefaultSize)

		self.SetIcon(wx.Icon(wx.ArtProvider.GetBitmap(wx.ART_PLUS)))
		
		self.m_textCtrl = wx.TextCtrl(self, value = parent.GetValue())
		self.m_btnOK = wx.BitmapButton(self, wx.ID_ANY, wx.ArtProvider.GetBitmap(wx.ART_GO_DOWN))

		szrMain = wx.BoxSizer(wx.HORIZONTAL)
		szrMain.Add( self.m_textCtrl, 3, wx.ALL, 5 )
		szrMain.Add( self.m_btnOK, 1, wx.ALL, 5 )
		szrMain.SetMinSize(400, -1)
		self.SetSizerAndFit(szrMain)
		self.Layout()

		self.m_btnOK.Bind(wx.EVT_BUTTON, self.__OnbtnOK)
		self.Bind(wx.EVT_CLOSE, self.__OnClose)
		
		self.m_Worksheet = Workbook().activeworksheet()
		self.m_Worksheet.bind("selecting", _GetVariable, self.m_textCtrl)

		self.m_Workbook = Workbook()
		self.m_Workbook.bind("pagechanged", _OnPageChanged, self)
	

	def __OnClose(self, event): 
		self.m_Worksheet.unbind("selecting", _GetVariable)
		self.m_Workbook.unbind("pagechanged", _OnPageChanged)
		self.Destroy()
		self.GetParent().GetTopLevelParent().Show()
		self.GetParent().SetValue(self.m_textCtrl.GetValue())
		event.Skip()


	def __OnbtnOK(self, event):
		self.Close()





class GridTextCtrl(wx.Control):
	def __init__(self, parent, bitmap = wx.NullBitmap):
		super().__init__(parent, style = wx.FULL_REPAINT_ON_RESIZE)

		BMP = bitmap if bitmap!=wx.NullBitmap else  wx.ArtProvider.GetBitmap(wx.ART_GO_UP)
		self.m_Txt = wx.TextCtrl(self)	
		self.m_Btn = wx.BitmapButton(self, bitmap = BMP)

		self.m_Btn.Bind(wx.EVT_BUTTON, self.__OnButtonClicked)
		self.Bind(wx.EVT_PAINT, self.__OnPaint)
		

	def DoGetBestSize(self):
		sz = self.m_Txt.GetBestSize()
		sz.y = int(sz.y*1.2)
		return sz

	
	def __OnPaint(self, event):
		dc = wx.PaintDC(self)
		dc.Clear()
		sz = self.GetClientSize()

		TL = self.GetClientRect().GetTopLeft()

		w = sz.GetWidth()
		h = sz.GetHeight()

		self.m_Txt.SetSize(int(0.85*w), h)
		self.m_Btn.SetSize(int(0.15*w), h)
		self.m_Btn.SetPosition(wx.Point(int(TL.x + 0.85 * w), TL.y))


	def __OnButtonClicked(self, event):
		frm = _frmGridSelection(self)
		frm.SetTitle(self.GetTopLevelParent().GetTitle())	
		frm.Show()

		self.GetTopLevelParent().Hide()


	def SetValue(self, str):
		self.m_Txt.SetValue(str)


	def GetValue(self):
		return self.m_Txt.GetValue()


#----------------------------------------------------------------------------


class Frame(wx.Frame):
	"""
	Provides a resizeable and a closeable frame
	"""
	def __init__( self, 
		parent=None, 
		id=wx.ID_ANY, 
		title=wx.EmptyString, 
		pos=wx.DefaultPosition, 
		size=wx.DefaultSize, 
		style=wx.CAPTION|wx.CLOSE_BOX|wx.MINIMIZE_BOX|wx.RESIZE_BORDER|wx.TAB_TRAVERSAL, 
		name=wx.FrameNameStr ):

		wx.Frame.__init__ ( self, parent, id = id, title = title, pos = pos, size = size, style = style,name=name )

		self.SetBackgroundColour( wx.Colour( 240, 240, 240 ) )
		self.Bind(wx.EVT_CLOSE, self.__OnClose)


	def __OnClose(self, event):
		self.Hide()
		self.Destroy()

		event.Skip()



#----------------------------------------------------------------


class NumTextCtrl(wx.TextCtrl):
	"""
	A text ctrl that only allows numeric entries
	Decimal separator used is .
	"""
	def __init__(self, 
			  parent, 
			  id = wx.ID_ANY, 
			  val:str = "", 
			  minval:float = -inf, 
			  maxval:float = inf):
		super().__init__(parent, id)

		self.m_Min = minval
		self.m_Max = maxval
		self.m_InitVal = val

		if minval>-inf and maxval<inf:
			self.SetToolTip(self._ToRange(minval, maxval))

		if val != "":
			try:
				numVal=float(val)
				self.SetValue(val)
			except ValueError as ve:
				pass

		self.Bind(wx.EVT_CHAR, self.__OnChar)
		self.Bind(wx.EVT_TEXT, self.__OnText)
		self.Bind( wx.EVT_KILL_FOCUS, self.__OnKillFocus )
	

	def _ToRange(self, minval:float, maxval:float):
		return "Expected range [" + str(minval) + "," + str(maxval)+ "]"

	
	def __OnKillFocus(self, event):
		val = self.GetValue()
		if val != "":
			try:
				numVal=float(val)
			except ValueError as ve:
				self.SetValue("")
		
		event.Skip()


	def __OnText(self, event):
		if self.GetValue() == "":
			event.Skip()
			return
			
		NumVal = float(self.GetValue())
		if NumVal<self.m_Min or NumVal>self.m_Max:
			Framework().messagebox(self._ToRange(self.m_Min, self.m_Max))

			#reset the value so that user will not be bugged when trying to recover from a mistake
			self.SetValue(self.m_InitVal)
		else:
			event.Skip()


	def __OnChar(self, event):
		key = event.GetKeyCode()
		val = self.GetValue()
		
		if key == wx.WXK_NONE:
			pass 

		elif chr(key) in string.digits:
			event.Skip()

		elif key==wx.WXK_DELETE or key==wx.WXK_BACK or key==wx.WXK_HOME:
			event.Skip()
		
		elif key==wx.WXK_LEFT or key == wx.WXK_RIGHT:
			event.Skip()
		
		#dont allow duplicate separators
		elif chr(key)=='.' and '.' not in val:
			event.Skip()
		
		elif (chr(key) =='E' or chr(key)=='e') and ('E' not in val) and ('e' not in val):
			#if there is no character then E or e not make any sense
			if val != "":
				#if first character is minus then we need at least 2 characters and second one must be digit
				if val[0]=='-':
					if len(val)>=2 and val[1] in string.digits:
						event.Skip()
				else:
					#if first character is not minus and E or e not already entered, allow it
					event.Skip()

		
		#only allow minus at the beginning
		elif chr(key)=='-':
			if(val == wx.EmptyString):
				event.Skip()
			else:
				HasE= ('E' in val) or ('e' in val)
				#if there is already minus at the beginning dont allow to add more
				if val[0] == '-' and not HasE:
					self.SetValue(val[0:])
				else:
					if HasE:
						event.Skip()
					else:
						self.SetValue('-' + val)
		
		elif chr(key) == ',':
			Framework().messagebox("Use decimal point (.) as the decimal separator")




#----------------------------------------------------

class pnlOutputOptions ( wx.Panel ):

	def __init__( self, parent, bgcolor = None):
		super().__init__ (parent)

		#imitate parent's background color to blend well
		if bgcolor == None:
			self.SetBackgroundColour(parent.GetBackgroundColour())
		else:
			if isinstance(bgcolor, wx.Colour) == False:
				raise TypeError("bgcolor must be of type wx.Colour")
			self.SetBackgroundColour(bgcolor)

		#header section
		sizerHeader = wx.BoxSizer( wx.HORIZONTAL )
		self.m_staticText = wx.StaticText( self, wx.ID_ANY, u"Ouput Options" )

		sizerHeader.Add( self.m_staticText, 0, wx.ALL, 5 )
		self.m_staticlineHeader = wx.StaticLine( self )
		sizerHeader.Add( self.m_staticlineHeader, 1, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

		#selection section
		self.m_radioSelection = wx.RadioButton( self, wx.ID_ANY, u"Selection")
		
		self.m_txtSelRange = GridTextCtrl( self)
		self.m_txtSelRange.Enable(False) #radiobox is selected, so disable it

		self.m_radioNewWS = wx.RadioButton( self, wx.ID_ANY, u"New Sheet")
		self.m_radioNewWS.SetValue( True ) #by default new worksheet
		
		fgSizer = wx.FlexGridSizer( 0, 2, 10, 0 )
		fgSizer.AddGrowableCol( 1 )
		fgSizer.SetFlexibleDirection( wx.BOTH )
		fgSizer.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSizer.Add( self.m_radioSelection, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtSelRange, 0, wx.ALL|wx.EXPAND, 5 )
		fgSizer.Add( self.m_radioNewWS, 0, wx.ALL, 5 )

		#footer section
		sizerFooter = wx.BoxSizer( wx.HORIZONTAL )
		self.m_stlineFooter = wx.StaticLine( self)
		sizerFooter.Add( self.m_stlineFooter, 1, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( sizerHeader, 0, wx.EXPAND, 5 )
		szrMain.Add( fgSizer, 0, wx.EXPAND, 5 )
		szrMain.Add( sizerFooter, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()

		self.m_radioSelection.Bind( wx.EVT_RADIOBUTTON, self.__OnRadioSelection )
		self.m_radioNewWS.Bind( wx.EVT_RADIOBUTTON, self.__OnRadioNewWS )



	def __OnRadioSelection( self, event ):
		self.m_txtSelRange.Enable()
		event.Skip()


	def __OnRadioNewWS( self, event ):
		self.m_txtSelRange.Enable(False)
		event.Skip()
	

	def __GetSelRange(self):
		if self.m_radioSelection.GetValue() == False or self.m_txtSelRange.GetValue() == "":
			return None

		try:
			rng = Range(txt=self.m_txtSelRange.GetValue())
			return rng
		except Exception as e:
			return None

	

	def Get(self)->tuple[Worksheet|None, int, int]:
		"""
		Returns worksheet (new or where range is), row and col indexes
		If there is a selection and selected range is not valid returns: None, -1, -1 
		If new worksheet returns: WS, 0, 0
		"""
		WS = None
		row, col = -1, -1
		
		if self.m_radioNewWS.GetValue(): #new worksheet
			WS = Worksheet()
			row, col = 0, 0
		else:
			SelRange = self.__GetSelRange()

			if SelRange != None:
				WS = SelRange.parent()
				row, col = SelRange.coords()[0] #[0]:top-left
		
		return WS, row, col



#------------------------------------------------------------------

class ScintillaCtrl(_stc.StyledTextCtrl):
	def __init__(self, parent, id=wx.ID_ANY, pos=wx.DefaultPosition, size=wx.DefaultSize, style=0):
		super().__init__(parent=parent, id=id, pos=pos, size=size, style=style)
		self.SetBufferedDraw(True)
		self.StyleClearAll()
		self.SetLexer(_stc.STC_LEX_PYTHON)
		self.SetWordChars("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMONPQRSTUVWXYZ_")

		builtinfuncnames = [name for name, obj in vars(builtins).items() 
                          if isinstance(obj, types.BuiltinFunctionType)]
		self.SetKeyWords(0,  " ".join([str(e) for e in keyword.kwlist])) #
		self.SetKeyWords(1, " ".join([str(e) for e in builtinfuncnames])); #Keywords
		self.SetUseTabs ( True )
		self.SetTabWidth ( 4 )
		self.SetIndent ( 4 )
		self.SetTabIndents( True )
		self.SetBackSpaceUnIndents( True )
		self.SetViewEOL( False )
		self.SetViewWhiteSpace( True )
		self.SetMarginWidth( 2, 0 )
		self.SetIndentationGuides( True )
		self.SetReadOnly( False )
		self.SetMarginWidth( 1, 0 )
		self.SetMarginType( 0, _stc.STC_MARGIN_NUMBER )
		self.SetMarginWidth( 0, self.TextWidth( _stc.STC_STYLE_LINENUMBER, "_99999" ) )
		self.MarkerDefine( _stc.STC_MARKNUM_FOLDER, _stc.STC_MARK_BOXPLUS )
		self.MarkerSetBackground( _stc.STC_MARKNUM_FOLDER, wx.BLACK)
		self.MarkerSetForeground( _stc.STC_MARKNUM_FOLDER, wx.WHITE)
		self.MarkerDefine( _stc.STC_MARKNUM_FOLDEROPEN, _stc.STC_MARK_BOXMINUS )
		self.MarkerSetBackground( _stc.STC_MARKNUM_FOLDEROPEN, wx.BLACK )
		self.MarkerSetForeground( _stc.STC_MARKNUM_FOLDEROPEN, wx.WHITE )
		self.MarkerDefine( _stc.STC_MARKNUM_FOLDERSUB, _stc.STC_MARK_EMPTY )
		self.MarkerDefine( _stc.STC_MARKNUM_FOLDEREND, _stc.STC_MARK_BOXPLUS )
		self.MarkerSetBackground( _stc.STC_MARKNUM_FOLDEREND, wx.BLACK )
		self.MarkerSetForeground( _stc.STC_MARKNUM_FOLDEREND, wx.WHITE )
		self.MarkerDefine( _stc.STC_MARKNUM_FOLDEROPENMID, _stc.STC_MARK_BOXMINUS )
		self.MarkerSetBackground( _stc.STC_MARKNUM_FOLDEROPENMID, wx.BLACK)
		self.MarkerSetForeground( _stc.STC_MARKNUM_FOLDEROPENMID, wx.WHITE)
		self.MarkerDefine( _stc.STC_MARKNUM_FOLDERMIDTAIL, _stc.STC_MARK_EMPTY )
		self.MarkerDefine( _stc.STC_MARKNUM_FOLDERTAIL, _stc.STC_MARK_EMPTY )
		self.SetSelBackground( True, wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHT ) )
		self.SetSelForeground( True, wx.SystemSettings.GetColour(wx.SYS_COLOUR_HIGHLIGHTTEXT ) )

		grey = wx.Colour(128, 128, 128)
		self.MarkerDefine(_stc.STC_MARKNUM_FOLDEROPEN, _stc.STC_MARK_BOXMINUS, wx.WHITE, grey)
		self.MarkerDefine(_stc.STC_MARKNUM_FOLDER, _stc.STC_MARK_BOXPLUS, wx.WHITE, grey)
		self.MarkerDefine(_stc.STC_MARKNUM_FOLDERSUB, _stc.STC_MARK_VLINE, wx.WHITE, grey)
		self.MarkerDefine(_stc.STC_MARKNUM_FOLDERTAIL, _stc.STC_MARK_LCORNER, wx.WHITE, grey)
		self.MarkerDefine(_stc.STC_MARKNUM_FOLDEREND, _stc.STC_MARK_BOXPLUSCONNECTED, wx.WHITE, grey)
		self.MarkerDefine(_stc.STC_MARKNUM_FOLDEROPENMID, _stc.STC_MARK_BOXMINUSCONNECTED, wx.WHITE, grey)
		self.MarkerDefine(_stc.STC_MARKNUM_FOLDERMIDTAIL, _stc.STC_MARK_TCORNER, wx.WHITE, grey)

		
		self.MarkerDefine(1, _stc.STC_MARK_ROUNDRECT, wx.WHITE, wx.RED)
		self.MarkerDefine(2, _stc.STC_MARK_ARROW, wx.BLACK, wx.GREEN)

		BG = wx.Colour(255, 255, 255)
		Font = wx.Font(11, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, "Consolas")

		IDs = [_stc.STC_P_COMMENTBLOCK, _stc.STC_P_COMMENTLINE, _stc.STC_P_TRIPLEDOUBLE, _stc.STC_P_STRING, 
		 	_stc.STC_P_CHARACTER, _stc.STC_P_OPERATOR, _stc.STC_P_IDENTIFIER, _stc.STC_P_WORD, 
			_stc.STC_P_WORD2, _stc.STC_P_DEFNAME, _stc.STC_P_CLASSNAME, _stc.STC_P_DECORATOR, _stc.STC_P_NUMBER]

		for id in IDs:
			self.StyleSetBackground(id, BG)
			self.StyleSetFont(id, Font)
		
		comments_FG = wx.Colour(127, 127, 127) 
		comments_BG = BG
		comments_Font = wx.Font(11, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_ITALIC, wx.FONTWEIGHT_NORMAL, False, "Consolas")

		strings_FG = wx.Colour(0, 127, 0)
		strings_Font = wx.Font(11, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_ITALIC, wx.FONTWEIGHT_NORMAL, False, "Consolas")

		ReservedWords_FG = wx.Colour(14, 1, 126)
		ReservedWords_Font = wx.Font(11, wx.FONTFAMILY_DEFAULT, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_BOLD, False, "Consolas")

		self.StyleSetForeground(_stc.STC_P_COMMENTBLOCK, comments_FG)
		self.StyleSetFont(_stc.STC_P_COMMENTBLOCK, comments_Font)

		self.StyleSetForeground(_stc.STC_P_COMMENTLINE, comments_FG)
		self.StyleSetFont(_stc.STC_P_COMMENTLINE, comments_Font)

		#triple quote for __doc__
		self.StyleSetForeground(_stc.STC_P_TRIPLEDOUBLE, comments_FG)
		self.StyleSetFont(_stc.STC_P_TRIPLEDOUBLE, comments_Font)

		#double quote
		self.StyleSetForeground(_stc.STC_P_STRING, strings_FG)
		self.StyleSetFont(_stc.STC_P_STRING, strings_Font)

		#single quote
		self.StyleSetForeground(_stc.STC_P_CHARACTER, strings_FG)
		self.StyleSetFont(_stc.STC_P_CHARACTER, strings_Font)

		self.StyleSetForeground(_stc.STC_P_OPERATOR, wx.Colour(0, 0, 0))
		self.StyleSetForeground(_stc.STC_P_IDENTIFIER, wx.Colour(0, 0, 0))

		#reserved words
		self.StyleSetForeground(_stc.STC_P_WORD, ReservedWords_FG)
		self.StyleSetFont(_stc.STC_P_WORD, ReservedWords_Font)

		#builtins words
		self.StyleSetForeground(_stc.STC_P_WORD2, wx.Colour(153, 153, 0))

		self.StyleSetForeground(_stc.STC_P_DEFNAME, wx.Colour(70, 130, 180)) #steel blue
		self.StyleSetForeground(_stc.STC_P_CLASSNAME, wx.Colour(32, 178, 170))
		self.StyleSetForeground(_stc.STC_P_DECORATOR, wx.Colour(153, 153, 0)) #dark yellow
		self.StyleSetForeground(_stc.STC_P_NUMBER, wx.Colour(127, 0, 0))

		self.SetCaretLineVisible(True)

		CaretLineBG = wx.Colour(230, 235, 235)
		self.SetCaretLineBackground(CaretLineBG)
