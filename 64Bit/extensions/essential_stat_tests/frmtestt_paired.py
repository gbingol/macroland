import numbers
import wx

import math

from scisuit.stats import test_t, test_tpaired_result
import _sci as _se



class frmtestt_paired ( _se.Frame ):

	def __init__( self, parent ):
		_se.Frame.__init__ ( self, parent, title = u"Paired t-test")
		
		ParentPath = _se.parent_path(__file__)
		IconPath = ParentPath / "icons" / "t_testpaired.png"
		self.SetIcon(_se.makeicon(IconPath))
		
		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )

		self.m_stVar1 = wx.StaticText( self, wx.ID_ANY, u"First sample:")
		self.m_stVar1.Wrap( -1 )
		self.m_txtVar1 = _se.GridTextCtrl( self)
		
		self.m_stVar2 = wx.StaticText( self, wx.ID_ANY, u"Second sample:")
		self.m_stVar2.Wrap( -1 )
		self.m_txtVar2 = _se.GridTextCtrl( self)

		WS = _se.activeworksheet()
		rng:_se.Range = WS.selection()
		if rng != None and rng.ncols() == 2:
			rng1 = rng.subrange(0, 0, -1, 1)
			rng2= rng.subrange(0, 1, -1, 1)
			self.m_txtVar1.SetValue(str(rng1))
			self.m_txtVar2.SetValue(str(rng2))
		
		self.m_stMean = wx.StaticText( self, wx.ID_ANY, u"Mean difference:")
		self.m_stMean.Wrap( -1 )
		self.m_txtMean = _se.NumTextCtrl( self,  val= u"0.0")
		
		self.m_stConf = wx.StaticText( self, wx.ID_ANY, u"Confidence Level:")
		self.m_stConf.Wrap( -1 )
		self.m_txtConf = _se.NumTextCtrl( self, val= u"95", minval=0.0, maxval=100.0)
		
		self.m_stAlt = wx.StaticText( self, wx.ID_ANY, u"Alternative:")
		self.m_stAlt.Wrap( -1 )
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

		sbSzr = wx.StaticBoxSizer( wx.StaticBox( self, wx.ID_ANY, u"Inspect Data" ), wx.HORIZONTAL )
		self.m_BtnBoxPlot = wx.Button( sbSzr.GetStaticBox(), label = u"Box-Whisker Plot" )
		sbSzr.Add( self.m_BtnBoxPlot, 0, wx.ALL, 5 )

		self.m_pnlOutput = _se.pnlOutputOptions( self )
		
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



	def __PrintValues(self, Vals:list, WS:_se.Worksheet, Row:int, Col:int):
		pval = Vals[0]
		R:test_tpaired_result = Vals[1]
		
		Header=["N", "Mean", "Std Dev", "SE Mean"]
		for j in range(len(Header)):
			WS[Row, Col + 1 + j] = Header[j] #+1 is for indentation
			
		Row += 1
		
		N = R.N
		HeaderVals = [
			["Sample 1", N, R.xaver, R.s1, R.s1/math.sqrt(N)],
			["Sample 2", N, R.yaver, R.s2, R.s2/math.sqrt(N)],
			["Difference"," ", R.mean, R.stdev],
			[None, None, None],
			["t-critical", R.tcritical],
			["p-value", pval]]
		
			
		for List in HeaderVals:
			if(List[0] == None):
				Row += 1
				continue
				
			for i in range(len(List)):
				WS[Row, Col + i] = List[i] 
			
			Row += 1
		
		WS[Row + 1, Col] = self.m_txtConf.GetValue() + \
			"% Confidence Interval " + \
			"(" + str(round(R.CI_lower, 4)) + ", " + str(round(R.CI_upper, 4)) + ")"
		
		


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

			xdata = _se.Range(self.m_txtVar1.GetValue()).tolist()
			ydata = _se.Range(self.m_txtVar2.GetValue()).tolist()

			xdata = [i for i in xdata if isinstance(i, numbers.Real)]
			ydata = [i for i in ydata if isinstance(i, numbers.Real)]
			
			pval, Result = test_t(x=xdata, y=ydata, mu=MeanDiff, paired = True, 
				alternative = Alternative, conflevel = conflevel)

			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."
				
			self.__PrintValues([pval, Result], WS, row, col)

		except Exception as e:
			wx.MessageBox(str(e))
			return

	

	def __OnBtnBoxWhiskerPlot(self, event):
		assert _se.assert_pkg(pip = "matplotlib", name = "matplotlib") == True, "matplotlib must be installed!"
		import matplotlib.pyplot as plt
		try:
			assert self.m_txtVar1.GetValue() != "", "Have you yet made a selection for (var #1)"
			assert self.m_txtVar2.GetValue() != "", "Have you yet made a selection for (var #2)"	

			xdata = _se.Range(self.m_txtVar1.GetValue()).tolist()
			ydata = _se.Range(self.m_txtVar2.GetValue()).tolist()

			xdata = [i for i in xdata if isinstance(i, numbers.Real)]
			ydata = [i for i in ydata if isinstance(i, numbers.Real)]

			figs, axs = plt.subplots(1, 2)
			axs[0].boxplot(xdata)
			axs[1].boxplot(ydata)

			plt.show()
			
		except Exception as e:
			wx.MessageBox(str(e), "Plot Error")
			return


if __name__ == "__main__":
	frm = frmtestt_paired(None)
	frm.Show()
