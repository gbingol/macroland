import numbers
import wx

from scisuit.stats import test_f, test_f_Result
import _sci as _se


class frmtest_f ( _se.Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"F Test")
		
		ParentPath = _se.parent_path(__file__)
		IconPath = ParentPath / "icons" / "test_f.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )

		self.m_stVar1 = wx.StaticText( self, wx.ID_ANY, u"Sample 1:")
		self.m_txtVar1 = _se.GridTextCtrl( self)
		
		self.m_stVar2 = wx.StaticText( self, wx.ID_ANY, u"Sample 2:")
		self.m_txtVar2 = _se.GridTextCtrl( self)

		WS = _se.Workbook().activeworksheet()
		rng:_se.Range = WS.selection()

		if rng != None and rng.ncols() == 2:
			rng1 = rng.subrange(0, 0, -1, 1)
			rng2= rng.subrange(0, 1, -1, 1)
			self.m_txtVar1.SetValue(str(rng1))
			self.m_txtVar2.SetValue(str(rng2))
		
		self.m_stDiff = wx.StaticText( self, wx.ID_ANY, u"Assumed ratio:")
		self.m_txtRatio = _se.NumTextCtrl( self, val=u"1.0", minval=1, maxval=10)
		
		self.m_stConf = wx.StaticText( self, wx.ID_ANY, u"Confidence Level:")
		self.m_txtConf = _se.NumTextCtrl( self, val= u"95", minval=0.0, maxval=100.0)

		self.m_stAlt = wx.StaticText( self, wx.ID_ANY, u"Alternative:")
		Choices = [ u"less than", u"not equal", u"greater than" ]
		self.m_chcAlt = wx.Choice( self, choices = Choices)
		self.m_chcAlt.SetSelection( 1 )

		fgSzr = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzr.AddGrowableCol( 1 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( self.m_stVar1, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtVar1, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stVar2, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtVar2, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stDiff, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtRatio, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stConf, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtConf, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stAlt, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_chcAlt, 0, wx.ALL, 5 )

		self.m_pnlOutput = _se.pnlOutputOptions( self)
		
		m_sdbSizer = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = "Compute" )
		m_sdbSizer.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label = "Close" )
		m_sdbSizer.AddButton( self.m_sdbSizerCancel )
		m_sdbSizer.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( fgSzr, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.ALL|wx.EXPAND, 5 )
		mainSizer.Add( m_sdbSizer, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( mainSizer )
		self.Layout()
		self.Centre( wx.BOTH )
	
		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtnClick )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKBtnClick )


	def __del__( self ):
		pass

	
	def __PrintValues(self, Vals:list, WS:_se.Worksheet, Row:int, Col:int):
		pval = Vals[0]
		Res:test_f_Result = Vals[1]
		Alternative = Vals[2]
		
		Header=["df", "variance"]
		for j in range(len(Header)):
			WS[Row, Col + 1 + j] = Header[j] #+1 is for indentation
			
		Row += 1
		
		ListVals = [
			["Sample 1", Res.df1, Res.var1],
			["Sample 2", Res.df2, Res.var2],
			[None, None, None],
			["F-critical", Res.Fcritical, None],
			["p-value", pval, None]]
		
			
		for List in ListVals:
			if(List[0] == None):
				Row += 1
				continue
				
			for i in range(len(List)):
				if(List[i] == None):
					continue
				WS[Row, Col + i] = List[i] 
			
			Row += 1	

		WS[Row + 1, Col] = self.m_txtConf.GetValue() + \
			"% Confidence Interval for " + \
			Alternative + \
			"(" + str(round(Res.CI_lower, 4)) + ", " + str(round(Res.CI_upper, 4)) + ")"


	
	def __OnCancelBtnClick( self, event ):
		self.Close()
	


	def __OnOKBtnClick( self, event ):
		try:
			assert self.m_txtVar1.GetValue() != "", "Selection expected for sample #1"
			assert self.m_txtVar2.GetValue() != "", "Selection expected for sample #2"	
			assert self.m_txtRatio.GetValue() != "", "Value expected assumed ratio."
			assert self.m_txtConf.GetValue() != "", "Value expected confidence level."
			
			conflevel=float(self.m_txtConf.GetValue())/100
			Ratio = float(self.m_txtRatio.GetValue())

			AlterOpt = ["less", "two.sided", "greater"]
			Alternative = AlterOpt[self.m_chcAlt.GetSelection()]

			xdata = _se.Range(self.m_txtVar1.GetValue()).tolist()
			ydata = _se.Range(self.m_txtVar2.GetValue()).tolist()

			xdata = [i for i in xdata if isinstance(i, numbers.Real) ]
			ydata = [i for i in ydata if isinstance(i, numbers.Real) ]
			
			pval, Result = test_f(x=xdata, y=ydata, ratio = Ratio,
				alternative = Alternative, conflevel = conflevel)

			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."	

			self.__PrintValues([pval, Result, Alternative], WS, row, col)
		except Exception as e:
			wx.MessageBox(str(e))
			return


if __name__ == "__main__":
	frm = frmtest_f(None)
	frm.Show()
