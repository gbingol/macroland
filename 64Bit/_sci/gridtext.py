import wx

from . import activeworksheet, Workbook


def _GetVariable(txt):
	ws = activeworksheet()
	rng = ws.selection()
	txt.SetValue(str(rng))


def _OnPageChanged(self):
	self.m_Worksheet.unbind("selecting", _GetVariable)
		
	self.m_Worksheet = activeworksheet()
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

		self.m_btnOK.Bind(wx.EVT_BUTTON, self.btnOK_OnButtonClick)
		self.Bind(wx.EVT_CLOSE, self.OnClose)
		
		self.m_Worksheet = activeworksheet()
		self.m_Worksheet.bind("selecting", _GetVariable, self.m_textCtrl)

		self.m_Workbook = Workbook()
		self.m_Workbook.bind("pagechanged", _OnPageChanged, self)
	

	def OnClose(self, event): 
		self.m_Worksheet.unbind("selecting", _GetVariable)
		self.m_Workbook.unbind("pagechanged", _OnPageChanged)
		self.Destroy()
		self.GetParent().GetTopLevelParent().Show()
		self.GetParent().SetValue(self.m_textCtrl.GetValue())
		event.Skip()


	def btnOK_OnButtonClick(self, event):
		self.Close()





class GridTextCtrl(wx.Control):
	def __init__(self, parent, bitmap = wx.NullBitmap):
		super().__init__(parent, style = wx.FULL_REPAINT_ON_RESIZE)

		BMP = bitmap if bitmap!=wx.NullBitmap else  wx.ArtProvider.GetBitmap(wx.ART_GO_UP)
		self.m_Txt = wx.TextCtrl(self)	
		self.m_Btn = wx.BitmapButton(self, bitmap = BMP)

		self.m_Btn.Bind(wx.EVT_BUTTON, self.OnButtonClicked)
		self.Bind(wx.EVT_PAINT, self.OnPaint)
		

	def DoGetBestSize(self):
		sz = self.m_Txt.GetBestSize()
		sz.y = int(sz.y*1.2)
		return sz

	
	def OnPaint(self, event):
		dc = wx.PaintDC(self)
		dc.Clear()
		sz = self.GetClientSize()

		TL = self.GetClientRect().GetTopLeft()

		w = sz.GetWidth()
		h = sz.GetHeight()

		self.m_Txt.SetSize(int(0.85*w), h)
		self.m_Btn.SetSize(int(0.15*w), h)
		self.m_Btn.SetPosition(wx.Point(int(TL.x + 0.85 * w), TL.y))


	def OnButtonClicked(self, event):
		frm = _frmGridSelection(self)
		frm.SetTitle(self.GetTopLevelParent().GetTitle())	
		frm.Show()

		self.GetTopLevelParent().Hide()


	def SetValue(self, str):
		self.m_Txt.SetValue(str)


	def GetValue(self):
		return self.m_Txt.GetValue()
