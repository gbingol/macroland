import numbers
import wx

from scisuit.stats import test_f, test_f_Result
from _sci import (Frame, GridTextCtrl, NumTextCtrl, pnlOutputOptions,
				  Workbook, Range, 
				  parent_path, prettify)


class frmtest_f ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = "F Test")
		
		IconPath = parent_path(__file__) / "icons" / "test_f.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )

		self.m_stVar1 = wx.StaticText( self, wx.ID_ANY, u"Sample 1:")
		self.m_txtVar1 = GridTextCtrl( self)
		
		self.m_stVar2 = wx.StaticText( self, wx.ID_ANY, u"Sample 2:")
		self.m_txtVar2 = GridTextCtrl( self)

		WS = Workbook().activeworksheet()
		rng:Range = WS.selection()

		if rng != None and rng.ncols() == 2:
			rng1 = rng.subrange(0, 0, -1, 1)
			rng2= rng.subrange(0, 1, -1, 1)
			self.m_txtVar1.SetValue(str(rng1))
			self.m_txtVar2.SetValue(str(rng2))
		
		self.m_stDiff = wx.StaticText( self, wx.ID_ANY, u"Assumed ratio:")
		self.m_txtRatio = NumTextCtrl( self, val=u"1.0", minval=1, maxval=10)
		
		self.m_stConf = wx.StaticText( self, wx.ID_ANY, u"Confidence Level:")
		self.m_txtConf = NumTextCtrl( self, val= u"95", minval=0.0, maxval=100.0)

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

		self.m_pnlOutput = pnlOutputOptions( self)
		
		sdbSzr = wx.StdDialogButtonSizer()
		self.m_btnOK = wx.Button( self, wx.ID_OK, label = "Compute" )
		sdbSzr.AddButton( self.m_btnOK )
		self.m_btnCancel = wx.Button( self, wx.ID_CANCEL, label = "Close" )
		sdbSzr.AddButton( self.m_btnCancel )
		sdbSzr.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( fgSzr, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.ALL|wx.EXPAND, 10 )
		mainSizer.Add( sdbSzr, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( mainSizer )
		self.Layout()
		self.Centre( wx.BOTH )
	
		self.m_btnCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtnClick )
		self.m_btnOK.Bind( wx.EVT_BUTTON, self.__OnOKBtnClick )


	
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

			xdata = Range(self.m_txtVar1.GetValue()).tolist()
			ydata = Range(self.m_txtVar2.GetValue()).tolist()

			xdata = [i for i in xdata if isinstance(i, numbers.Real) ]
			ydata = [i for i in ydata if isinstance(i, numbers.Real) ]
			
			pval, Res = test_f(x=xdata, y=ydata, ratio = Ratio,
				alternative = Alternative, conflevel = conflevel)

			WS, Row, Col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."	
			prtfy = self.m_pnlOutput.Prettify()

			WS.writelist(["df", "variance"], Row, Col+1, rowmajor=False) #+1 is for indentation
			
			Row += 1
			
			ListVals = [
				["Sample 1", Res.df1, Res.var1],
				["Sample 2", Res.df2, Res.var2],
				[None, None, None],
				["F-critical", Res.Fcritical, None],
				["p-value", pval, None]]
					
			Row, Col = WS.writelist2d(ListVals, Row, Col, pretty=self.m_pnlOutput.Prettify())

			Txt = f"{self.m_txtConf.GetValue()}% Confidence Interval for {Alternative} "
			Txt += f"( {prettify(Res.CI_lower, prtfy)}, {prettify(Res.CI_upper, prtfy)} )"
			WS[Row + 1, Col] = Txt

		except Exception as e:
			wx.MessageBox(str(e))
			return


if __name__ == "__main__":
	frm = frmtest_f(None)
	frm.Show()
