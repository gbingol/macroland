import wx

from _sci import activeworksheet, Frame, colnum2labels, assert_pkg, parent_path



class pnlRemoveDups ( wx.Panel ):

	def __init__( self, parent):
		wx.Panel.__init__ ( self, parent)

		mainSizer = wx.BoxSizer( wx.HORIZONTAL )

		self.m_stTxtRemove = wx.StaticText( self, wx.ID_ANY, u"Remove by:")
		self.m_stTxtRemove.Wrap( -1 )	

		self.m_chCol = wx.Choice( self)
		self.m_chCol.SetSelection( 0 )

		self.m_stTxtKeep = wx.StaticText( self, wx.ID_ANY, u"Keep:")
		self.m_stTxtKeep.Wrap( -1 )	

		self.m_chKeep = wx.Choice( self, choices = ["First", "Last"])
		self.m_chKeep.SetSelection( 0 )

		mainSizer.Add( self.m_stTxtRemove, 0, wx.ALL, 5 )
		mainSizer.Add( self.m_chCol, 1, wx.EXPAND, 5 )
		mainSizer.Add( self.m_stTxtKeep, 0, wx.ALL, 5 )
		mainSizer.Add( self.m_chKeep, 1, wx.EXPAND, 5 )

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


	def KeepFirst(self)->bool:
		return self.m_chKeep.GetSelection() == 0

	
	def OnInit( self, event ):
		ws = activeworksheet()
		rng = ws.selection()

		self.m_chCol.Append("ALL")
		
		TL, BR = rng.coords()
		Col_TL, Col_BR = TL[1] + 1, BR[1] + 1
		for i in range(Col_TL, Col_BR + 1):
			Label = colnum2labels(i)
			self.m_chCol.Append(Label)
		
		self.m_chCol.SetSelection(0)

		event.Skip()





class frmRemoveDuplicates ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, 
			title = "Remove Duplicates", 
			style=wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP )
		
		assert assert_pkg(pip = "pandas", name = "pandas") == True, "Pandas must be installed!"
		
		IconPath = parent_path(__file__) / "icons" / "remove_duplicates.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		rng = None
		ws = activeworksheet()
		rng = ws.selection()
		
		assert rng!=None, "A selection must be made"

		self.m_pnlRemoveDups = pnlRemoveDups( self)
		self.m_pnlRemoveDups.InitDialog()
		self.m_chkHeaders = wx.CheckBox( self, wx.ID_ANY, u"Has Headers" )
		
		m_sdbSzr = wx.StdDialogButtonSizer()
		self.m_sdbSzrOK = wx.Button( self, wx.ID_OK )
		m_sdbSzr.AddButton( self.m_sdbSzrOK )
		self.m_sdbSzrCancel = wx.Button( self, wx.ID_CANCEL )
		m_sdbSzr.AddButton( self.m_sdbSzrCancel )
		m_sdbSzr.Realize()

		mnSzr = wx.BoxSizer( wx.VERTICAL )
		mnSzr.Add( self.m_pnlRemoveDups, 0, wx.EXPAND, 5 )
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
		N0 = len(df) #initial length 

		Sel = self.m_pnlRemoveDups.GetSelectedCol()[0]
		Keep = "first" if self.m_pnlRemoveDups.KeepFirst() else "last"
		Headers = list(df)
		if Sel == 0:
			df.drop_duplicates(keep=Keep, inplace=True)
		else:
			SelHeader = [Headers[Sel-1]]
			df.drop_duplicates(subset = SelHeader, keep=Keep, inplace = True)
		
		N = len(df) #length after removals
		N_Removed = N0-N
		
		if N_Removed>0:
			SubRange = rng.subrange(1 if HasHeaders else 0, 0)
			SubRange.clear()

			row = TL[0] + 1 if HasHeaders else TL[0]
			col = TL[1]
			print_to_ws(df, ws, row, col, indexes=False, headers=False)

		wx.MessageBox(str(N_Removed) + " entries removed")

		event.Skip()


if __name__=="__main__":
	try:
		frm = frmRemoveDuplicates(None)
		sz = frm.GetSize()
		sz.x=400
		frm.SetSize(sz)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e), "Duplicate Removal Error!")