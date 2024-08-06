import wx
import wx.html

import scisuit.stats as stat

from _sci import (Frame, GridTextCtrl, NumTextCtrl, pnlOutputOptions,
				  Workbook, Range, 
				  parent_path, prettify)



class frmextensionmngr ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title="Extension Manager")

		IconPath = parent_path(__file__) / "icons" / "extensions.png"
		self.SetIcon(wx.Icon(str(IconPath)))
		
		self.m_split = wx.SplitterWindow(self, style= wx.SP_3D|wx.SP_LIVE_UPDATE)
		
		self.m_LWExt = wx.ListView(self.m_split)
		self.m_LWExt.SetFont(wx.Font(11, wx.FONTFAMILY_SWISS, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, "Arial"))
		ColumnHeader = wx.ItemAttr()
		ColumnHeader.SetFont(wx.Font(14, wx.FONTFAMILY_SWISS, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, "Arial"))
		self.m_LWExt.SetHeaderAttr(ColumnHeader)

		self.m_LWExt.InsertColumn(0, "Name", wx.LIST_FORMAT_LEFT)
		self.m_LWExt.InsertColumn(1, "Version", wx.LIST_FORMAT_LEFT)
		self.m_LWExt.InsertColumn(2, "Developer", wx.LIST_FORMAT_LEFT)
		self.m_LWExt.InsertColumn(3, "Description", wx.LIST_FORMAT_LEFT)

		self.m_HTMLWin = wx.html.HtmlWindow(self.m_split)

		self.m_split.SplitHorizontally(self.m_LWExt, self.m_HTMLWin, 0)
		self.m_split.SetSashGravity(0.30)


		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( self.m_split, 1, wx.EXPAND |wx.ALL, 5 )
		self.SetSizerAndFit( mainSizer )
		self.Layout()

		self.Centre( wx.BOTH )



	

if __name__ == "__main__":
	try:
		frm = frmextensionmngr(None)
		frm.Maximize()
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e), "Error")