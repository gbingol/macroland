import numbers
import wx

import math

from scisuit.stats import test_t, test_tpaired_result
from _sci import (Frame, GridTextCtrl, NumTextCtrl, pnlOutputOptions,
				  Workbook, Range, parent_path, prettify)



class frmtestt_paired ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"Paired t-test")
		
		ParentPath = parent_path(__file__)
		IconPath = ParentPath / "icons" / "t_testpaired.png"
		self.SetIcon(wx.Icon(str(IconPath)))
		
		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )

		self.m_stVar1 = wx.StaticText( self, wx.ID_ANY, u"First sample:")
		self.m_txtVar1 = GridTextCtrl( self)
		
		self.m_stVar2 = wx.StaticText( self, wx.ID_ANY, u"Second sample:")
		self.m_txtVar2 = GridTextCtrl( self)

		WS = Workbook().activeworksheet()
		rng = WS.selection()
		if rng != None and rng.ncols() == 2:
			rng1 = rng.subrange(0, 0, -1, 1)
			rng2= rng.subrange(0, 1, -1, 1)
			self.m_txtVar1.SetValue(str(rng1))
			self.m_txtVar2.SetValue(str(rng2))
		
		self.m_stMean = wx.StaticText( self, wx.ID_ANY, u"Mean difference:")
		self.m_txtMean = NumTextCtrl( self,  val= u"0.0")
		
		self.m_stConf = wx.StaticText( self, wx.ID_ANY, u"Confidence Level:")
		self.m_txtConf = NumTextCtrl( self, val= u"95", minval=0.0, maxval=100.0)
		
		self.m_stAlt = wx.StaticText( self, wx.ID_ANY, u"Alternative:")
		self.m_chcAlt = wx.Choice( self, choices = [ u"less than", u"not equal", u"greater than" ])
		self.m_chcAlt.SetSelection( 1 )
		
		fgSzr = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzr.AddGrowableCol( 1 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( self.m_stVar1, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtVar1, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stVar2, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtVar2, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stMean, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtMean, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stConf, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtConf, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stAlt, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_chcAlt, 0, wx.ALL, 5 )

		sbSzr = wx.StaticBoxSizer( wx.StaticBox( self, wx.ID_ANY, u"Inspect Selected Data" ), wx.HORIZONTAL )
		self.m_BtnBoxPlot = wx.Button( sbSzr.GetStaticBox(), label = u"Box-Whisker Plot" )
		sbSzr.Add( self.m_BtnBoxPlot, 0, wx.ALL, 5 )

		self.m_pnlOutput = pnlOutputOptions( self )
		
		szrSdb = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = u"Compute" )
		szrSdb.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label = u"Close" )
		szrSdb.AddButton( self.m_sdbSizerCancel )
		szrSdb.Realize()

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( fgSzr, 0, wx.EXPAND, 5 )
		szrMain.Add( sbSzr, 0, wx.ALL|wx.EXPAND, 5 )
		szrMain.Add( self.m_pnlOutput, 0, wx.ALL|wx.EXPAND, 5 )
		szrMain.Add( szrSdb, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()
		self.Centre( wx.BOTH )
	
		self.m_BtnBoxPlot.Bind(wx.EVT_BUTTON, self.__OnBtnBoxWhiskerPlot)
		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtn)
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKBtn)



	def __OnCancelBtn( self, event ):
		self.Close()	



	def __OnOKBtn( self, event ):
		try:
			assert self.m_txtVar1.GetValue() != "", "Selection expected (var #1)"
			assert self.m_txtVar2.GetValue() != "", "Selection expected (var #2)"	
			assert self.m_txtMean.GetValue() != "", "Value expected for assumed mean difference."
			
			conflevel = float(self.m_txtConf.GetValue())/100
			assert conflevel>0 and conflevel<1, "Confidence interval must be in (0, 100)"

			MeanDiff = float(self.m_txtMean.GetValue())

			AlterOpt = ["less", "two.sided", "greater"]
			Alternative = AlterOpt[self.m_chcAlt.GetSelection()]

			xdata = Range(self.m_txtVar1.GetValue()).tolist()
			ydata = Range(self.m_txtVar2.GetValue()).tolist()

			xdata = [i for i in xdata if isinstance(i, numbers.Real)]
			ydata = [i for i in ydata if isinstance(i, numbers.Real)]
			
			pval, Res = test_t(x=xdata, y=ydata, mu=MeanDiff, paired = True, 
				alternative = Alternative, conflevel = conflevel)

			WS, Row, Col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: Selected range is invalid."
			prtfy = self.m_pnlOutput.Prettify()
				
			Header = ["N", "Mean", "Std Dev", "SE Mean"]
			WS.writelist(Header, Row, Col+1, rowmajor=False)#+1 is for indentation
						
			Row += 1
			
			N = Res.N
			Vals = [["Sample 1", N, Res.xaver, Res.s1, Res.s1/math.sqrt(N)],
				["Sample 2", N, Res.yaver, Res.s2, Res.s2/math.sqrt(N)],
				["Difference"," ", Res.mean, Res.stdev],
				[None],
				["t-critical", Res.tcritical],
				["p-value", pval]]
		
			Row, Col = WS.writelist2d(Vals, Row, Col, pretty=prtfy)

			Row += 1
			
			Txt = f"{self.m_txtConf.GetValue()}% Confidence Interval for "
			Txt += f"( {prettify(Res.CI_lower, prtfy)}, {prettify(Res.CI_upper, prtfy)} )"
			WS[Row, Col] = Txt

		except Exception as e:
			wx.MessageBox(str(e))
			return

	

	def __OnBtnBoxWhiskerPlot(self, event):
		import scisuit.plot as plt
		try:
			assert self.m_txtVar1.GetValue() != "", "Have you yet made a selection for (var #1)"
			assert self.m_txtVar2.GetValue() != "", "Have you yet made a selection for (var #2)"	

			xdata = Range(self.m_txtVar1.GetValue()).tolist()
			ydata = Range(self.m_txtVar2.GetValue()).tolist()

			xdata = [i for i in xdata if isinstance(i, numbers.Real)]
			ydata = [i for i in ydata if isinstance(i, numbers.Real)]

			plt.boxplot(xdata)
			plt.boxplot(ydata)

			plt.show()
			
		except Exception as e:
			wx.MessageBox(str(e), "Plot Error")


if __name__ == "__main__":
	frm = frmtestt_paired(None)
	frm.Show()
