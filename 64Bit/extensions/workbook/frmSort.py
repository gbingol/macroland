import wx

from _sci import Workbook, Frame, colnum2label, parent_path


class pnlSort ( wx.Panel ):

	def __init__( self, parent):
		super().__init__ (parent)

		self.m_stTxt = wx.StaticText( self, wx.ID_ANY, u"Sort by:")

		self.m_chCol = wx.Choice( self)
		self.m_chCol.SetSelection( 0 )

		self.m_chOrder = wx.Choice( self, wx.ID_ANY, choices = [ u"A-Z", u"Z-A" ])
		self.m_chOrder.SetSelection( 0 )

		mainSizer = wx.BoxSizer( wx.HORIZONTAL )
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
		ws = Workbook().activeworksheet()
		rng = ws.selection()
		
		TL, BR = rng.coords()
		Col_TL, Col_BR = TL[1] + 1, BR[1] + 1
		for i in range(Col_TL, Col_BR + 1):
			Label = colnum2label(i)
			self.m_chCol.Append(Label)
		
		self.m_chCol.SetSelection(0)

		event.Skip()





class frmSort ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, 
			title = "Sort",
			style=wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP )

		
		IconPath = parent_path(__file__) / "icons" / "sort.jpg"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		ws = Workbook().activeworksheet()

		#keep the initial range as user might click somewhere else after frame shown
		self.m_rng = ws.selection() 	
		assert self.m_rng!=None, "A selection must be made"

		self.m_pnlSort = pnlSort( self)
		self.m_pnlSort.InitDialog()
		self.m_chkHeaders = wx.CheckBox( self, wx.ID_ANY, u"Has Headers" )
		
		sdbSzr = wx.StdDialogButtonSizer()
		self.m_btnOK = wx.Button( self, wx.ID_OK )
		sdbSzr.AddButton( self.m_btnOK )
		self.m_btnCancel = wx.Button( self, wx.ID_CANCEL )
		sdbSzr.AddButton( self.m_btnCancel )
		sdbSzr.Realize()

		mnSzr = wx.BoxSizer( wx.VERTICAL )
		mnSzr.Add( self.m_pnlSort, 0, wx.EXPAND, 5 )
		mnSzr.Add( ( 0, 10), 0, wx.EXPAND, 5 )
		mnSzr.Add( self.m_chkHeaders, 0, wx.EXPAND, 5 )
		mnSzr.Add( ( 0, 10), 0, wx.EXPAND, 5 )
		mnSzr.Add( sdbSzr, 0, wx.EXPAND, 5 )

		self.Layout()
		self.SetSizerAndFit( mnSzr )	

		self.Centre( wx.BOTH )

		self.m_btnCancel.Bind( wx.EVT_BUTTON, self.__OnCancel )
		self.m_btnOK.Bind( wx.EVT_BUTTON, self.__OnOK )


	def __OnCancel( self, event:wx.CommandEvent ):
		self.Close()
		event.Skip()


	def __OnOK( self, event:wx.CommandEvent ):
		try:
			def sortFunc(e):
				return isinstance(e, str), e
			
		
			selCol = self.m_pnlSort.GetSelectedCol()[0]
			df:list[list] = self.m_rng.tolist(axis=1)

			for lst in df[:]:
				x = [e for e in lst if isinstance(e, str|float|int)]
				if len(x) == 0:
					df.remove(lst)
			
			dfSorted = sorted(df, 
					 	key = lambda x: sortFunc(x[selCol]), 
						reverse=not self.m_pnlSort.IsAscending())

			self.m_rng.clear()

			TL, _ = self.m_rng.coords()
			self.m_rng.parent().writelist2d(dfSorted, *TL)
					
		except Exception as e:
			wx.MessageBox(str(e), "Sort Error!")

		event.Skip()


if __name__=="__main__":
	try:
		frm = frmSort(None)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e), "Initialization Error!")