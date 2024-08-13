import wx

import scisuit.plot as plt
from _sci import (Frame, GridTextCtrl, Worksheet,
				  Workbook, Range, Framework)




class frmBubbleChart ( Frame ):

	def __init__( self, parent ):
		super().__init__ (
			parent, 
			title = "Bubble Chart", 
			style = wx.CAPTION|wx.CLOSE_BOX|wx.RESIZE_BORDER|wx.STAY_ON_TOP)

		lblX = wx.StaticText( self, label = "x:")
		self.m_txtX = GridTextCtrl( self)

		lblY = wx.StaticText( self, label = "y:")
		self.m_txtY = GridTextCtrl( self)
		
		lblSize = wx.StaticText( self, label = "size:")
		self.m_txtSize = GridTextCtrl( self)

		WS = Workbook().activeworksheet()
		rng = WS.selection()

		assert rng != None, "A selection must be made."
		assert rng.ncols() == 3, "Exactly 3 columns of data should be selected."

		_rngX = rng.subrange(0, 0, -1, 1)
		_rngY= rng.subrange(0, 1, -1, 1)
		_rngSize= rng.subrange(0, 2, -1, 1)
		self.m_txtX.SetValue(str(_rngX))
		self.m_txtY.SetValue(str(_rngY))
		self.m_txtSize.SetValue(str(_rngSize))

		fgSzr = wx.FlexGridSizer( 0, 2, 6, 0 )
		fgSzr.AddGrowableCol( 1 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( lblX, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtX, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( lblY, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtY, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( lblSize, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtSize, 0, wx.ALL|wx.EXPAND, 5 )
		
		
		sdbSzr = wx.StdDialogButtonSizer()
		self.m_btnOK = wx.Button( self, wx.ID_OK, label="OK" )
		sdbSzr.AddButton( self.m_btnOK )
		self.m_btnCancel = wx.Button( self, wx.ID_CANCEL, label = "Close" )
		sdbSzr.AddButton( self.m_btnCancel )
		sdbSzr.Realize()

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( fgSzr, 0, wx.EXPAND, 5 )
		szrMain.Add( sdbSzr, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()
		self.Centre( wx.BOTH )


		self.m_btnCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtn )
		self.m_btnOK.Bind( wx.EVT_BUTTON, self.__OnOKBtn )



	def __OnCancelBtn( self, event ):
		self.Close()
		event.Skip()
	

	def __OnOKBtn( self, event ):
		try:	
			_X = Range(self.m_txtX.GetValue()).tolist()
			_Y = Range(self.m_txtY.GetValue()).tolist()
			_Size = Range(self.m_txtSize.GetValue()).tolist()

			X = [i for i in _X if isinstance(i, int|float)]
			Y = [i for i in _Y if isinstance(i, int|float)]
			Size = [i for i in _Size if isinstance(i, int|float)]

			assert len(X)>=2 and len(Y)>=2 and len(Size)>=2, "At least 2 valid numeric data expected."
			assert len(X) == len(Y), "X and Y must have same size."
			assert len(Y) == len(Size), "Y and Size must have same size."

			plt.bubble(x=X, y=Y, s=Size)
			
			self.Close() #Close before plot's event loop takes over

			plt.show()
		except Exception as e:
			wx.MessageBox(str(e), "Plot Error")

	


if __name__ == "__main__":
	try:
		frm = frmBubbleChart(None)
		frm.Show()
	except Exception as e:
		Framework().messagebox(str(e), "Plot Error")