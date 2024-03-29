import wx

from _sci import activeworksheet, Frame, colnum2labels, assert_pkg, parent_path, makeicon


class pnlSort ( wx.Panel ):

	def __init__( self, parent):
		wx.Panel.__init__ ( self, parent)

		mainSizer = wx.BoxSizer( wx.HORIZONTAL )

		self.m_stTxt = wx.StaticText( self, wx.ID_ANY, u"Sort by:")
		self.m_stTxt.Wrap( -1 )	

		self.m_chCol = wx.Choice( self)
		self.m_chCol.SetSelection( 0 )

		self.m_chOrder = wx.Choice( self, wx.ID_ANY, choices = [ u"A-Z", u"Z-A" ])
		self.m_chOrder.SetSelection( 0 )

		mainSizer.Add( self.m_stTxt, 0, wx.ALL, 5 )
		mainSizer.Add( self.m_chCol, 0, wx.ALL, 5 )
		mainSizer.Add( self.m_chOrder, 0, wx.ALL, 5 )

		self.SetSizer( mainSizer )
		self.Layout()

		self.Bind( wx.EVT_INIT_DIALOG, self.OnInit )


	def GetSelectedCol(self)->tuple:
		"""
		returns selection as index and as string
		"""
		N = self.m_chCol.GetSelection()
		SelStr = self.m_chCol.GetStringSelection() 

		return N, SelStr
	
	def IsAscending(self)->bool:
		return self.m_chOrder.GetSelection() == 0


	
	def OnInit( self, event ):
		ws = activeworksheet()
		rng = ws.selection()
		
		TL, BR = rng.coords()
		Col_TL, Col_BR = TL[1] + 1, BR[1] + 1
		for i in range(Col_TL, Col_BR + 1):
			Label = colnum2labels(i)
			self.m_chCol.Append(Label)
		
		self.m_chCol.SetSelection(0)

		event.Skip()





class frmSort ( Frame ):

	def __init__( self, parent ):
		Frame.__init__ ( self, 
			parent, 
			title = "Sort",
			style=wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP )
		
		assert assert_pkg(pip = "pandas", name = "pandas") == True, "Pandas must be installed!"
		
		IconPath = parent_path(__file__) / "icons" / "sort.jpg"
		self.SetIcon(makeicon(IconPath))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		rng = None
		ws = activeworksheet()
		rng = ws.selection()
		
		assert rng!=None, "A selection must be made"

		self.m_pnlSort = pnlSort( self)
		self.m_pnlSort.InitDialog()
		self.m_chkHeaders = wx.CheckBox( self, wx.ID_ANY, u"Has Headers" )
		
		m_sdbSzr = wx.StdDialogButtonSizer()
		self.m_sdbSzrOK = wx.Button( self, wx.ID_OK )
		m_sdbSzr.AddButton( self.m_sdbSzrOK )
		self.m_sdbSzrCancel = wx.Button( self, wx.ID_CANCEL )
		m_sdbSzr.AddButton( self.m_sdbSzrCancel )
		m_sdbSzr.Realize()

		mnSzr = wx.BoxSizer( wx.VERTICAL )
		mnSzr.Add( self.m_pnlSort, 0, wx.EXPAND, 5 )
		mnSzr.Add( ( 0, 10), 0, wx.EXPAND, 5 )
		mnSzr.Add( self.m_chkHeaders, 0, wx.EXPAND, 5 )
		mnSzr.Add( ( 0, 10), 0, wx.EXPAND, 5 )
		mnSzr.Add( m_sdbSzr, 0, wx.EXPAND, 5 )

		self.Layout()
		self.SetSizerAndFit( mnSzr )	

		self.Centre( wx.BOTH )

		self.m_sdbSzrCancel.Bind( wx.EVT_BUTTON, self.OnCancel )
		self.m_sdbSzrOK.Bind( wx.EVT_BUTTON, self.OnOK )


	def OnCancel( self, event ):
		self.Close()
		event.Skip()


	def OnOK( self, event ):
		import pandas as pd
		from _sci.pandas import print_to_ws

		ws = activeworksheet()
		rng = ws.selection()
		TL, BR = rng.coords()

		HasHeaders = self.m_chkHeaders.GetValue()

		df = pd.DataFrame(rng.todict(HasHeaders))

		Headers = list(df)
		SelHeader = Headers[self.m_pnlSort.GetSelectedCol()[0]]
		df.sort_values(SelHeader, inplace=True, ascending=self.m_pnlSort.IsAscending())

		row = TL[0] + 1 if HasHeaders else TL[0]
		col = TL[1]
		print_to_ws(df, ws, row, col, indexes=False, headers=False)

		event.Skip()


if __name__=="__main__":
	try:
		frm = frmSort(None)
		sz = frm.GetSize()
		sz.x=300
		frm.SetSize(sz)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e), "Sort Error!")