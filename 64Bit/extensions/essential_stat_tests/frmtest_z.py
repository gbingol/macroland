import wx
import numbers


from scisuit.stats import test_z, test_z_Result
import _sci as _se



class frmtest_z ( _se.Frame ):

	def __init__( self, parent ):
		_se.Frame.__init__ ( self, parent, title = u"Z Test")

		ParentPath = _se.parent_path(__file__)
		IconPath = ParentPath / "icons" / "test_z.png"
		self.SetIcon(_se.makeicon(IconPath))
		
		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )

		self.m_stVar = wx.StaticText( self, label = u"Variable Range:")
		self.m_stVar.Wrap( -1 )
		self.m_txtVar = _se.GridTextCtrl( self)

		WS = _se.activeworksheet()
		rng:_se.Range = WS.selection()
		if rng != None and rng.ncols() == 1:
			self.m_txtVar.SetValue(str(rng))

		self.m_stMean = wx.StaticText( self, label = u"Test Mean:")
		self.m_stMean.Wrap( -1 )
		self.m_txtMean = _se.NumTextCtrl( self)

		self.m_stSigma = wx.StaticText( self, label = u"Sigma:")
		self.m_stSigma.Wrap( -1 )
		self.m_txtSigma = _se.NumTextCtrl( self)

		self.m_stConf = wx.StaticText( self, label = u"Confidence Level:")
		self.m_stConf.Wrap( -1 )
		self.m_txtConf = _se.NumTextCtrl( self, val= u"95", minval=0.0, maxval=100.0)
		
		self.m_stAlt = wx.StaticText( self, wx.ID_ANY, u"Alternative:")
		self.m_stAlt.Wrap( -1 )
		self.m_chcAlt = wx.Choice( self, choices = ["less than", "not equal", "greater than" ])
		self.m_chcAlt.SetSelection( 1 )
		
		fgSzr = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzr.AddGrowableCol( 1 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( self.m_stVar, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtVar, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stMean, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtMean, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stSigma, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtSigma, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stConf, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtConf, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stAlt, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_chcAlt, 0, wx.ALL, 5 )

		sbSizer = wx.StaticBoxSizer( wx.StaticBox( self, wx.ID_ANY, u"Inspect Data" ), wx.HORIZONTAL )
		self.m_BtnHistogram = wx.Button( sbSizer.GetStaticBox(), label = u"Histogram" )
		self.m_BtnBoxWhisker = wx.Button( sbSizer.GetStaticBox(), label = u"Box-Whisker")
		sbSizer.Add( self.m_BtnHistogram, 0, wx.ALL, 5 )
		sbSizer.Add( self.m_BtnBoxWhisker, 0, wx.ALL, 5 )

		self.m_pnlOutput = _se.pnlOutputOptions( self)	

		m_sdbSizer = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = "Compute" )
		m_sdbSizer.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label = "Close" )
		m_sdbSizer.AddButton( self.m_sdbSizerCancel )
		m_sdbSizer.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( fgSzr, 0, wx.EXPAND, 5 )
		mainSizer.Add( sbSizer, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.ALL|wx.EXPAND, 5 )
		mainSizer.Add( m_sdbSizer, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( mainSizer )
		self.Layout()
		self.Centre( wx.BOTH )
		
		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtnClick )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKBtnClick )

		self.m_BtnHistogram.Bind(wx.EVT_BUTTON, self.__OnPlotChart)
		self.m_BtnBoxWhisker.Bind(wx.EVT_BUTTON, self.__OnPlotChart)


	def __PrintValues(self, Vals:list, WS:_se.Worksheet, Row:int, Col:int):
		pval = Vals[0]
		R:test_z_Result = Vals[1]
		
		Header=["N", "Average", "stdev", "SE Mean", "z", "p-value"]
		Vals =[R.N, R.mean, R.stdev, R.SE, R.zcritical, pval] 
		for j in range(len(Header)):
			WS[Row, Col + j] = Header[j] 
			WS[Row + 1, Col + j] = Vals[j]
			
		Row += 2
		
		WS[Row + 1, Col] = self.m_txtConf.GetValue() + \
			"% Confidence Interval for " + \
			"(" + str(round(R.CI_lower, 4)) + ", " + str(round(R.CI_upper, 4)) + ")"
		
	
	
	def __OnCancelBtnClick( self, event ):
		self.Close()
		


	def __OnOKBtnClick( self, event ):
		try:
			assert self.m_txtVar.GetValue() != "", "Variable range cannot be blank."
			assert self.m_txtMean.GetValue() != "", "Value expected for test mean."
			assert self.m_txtSigma.GetValue() != "", "Value expected for sigma."
			assert self.m_txtConf.GetValue() != "", "Value expected for confidence level."
			
			conflevel = float(self.m_txtConf.GetValue())/100
			Mu = float(self.m_txtMean.GetValue())
			Sigma = float(self.m_txtSigma.GetValue()) #sd of population

			AlterOpt = ["less", "two.sided", "greater"]
			Alternative = AlterOpt[self.m_chcAlt.GetSelection()]

			xdata = _se.Range(self.m_txtVar.GetValue()).tolist()
			xdata = [i for i in xdata if isinstance(i, numbers.Real) ]
			
			pval, Results = test_z(x=xdata, mu = Mu, sd = Sigma, 
				alternative = Alternative, conflevel = conflevel)

			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."	

			self.__PrintValues([pval, Results], WS, row, col)

		except Exception as e:
			wx.MessageBox(str(e))
			return
		
	

	def __OnPlotChart(self, event):
		evtObj = event.GetEventObject()

		assert _se.assert_pkg(pip = "matplotlib", name = "matplotlib") == True, "matplotlib must be installed!"
		import matplotlib.pyplot as plt

		try:	
			assert self.m_txtVar.GetValue() != "", "Have you made a valid selection yet?"

			xdata = _se.Range(self.m_txtVar.GetValue()).tolist()
			assert len(xdata) >=3, "Not enough data to proceed!"
				
			if(evtObj == self.m_BtnHistogram):
				plt.hist(xdata)
			
			elif(evtObj == self.m_BtnBoxWhisker):
				plt.boxplot(xdata)
			
			plt.show()
		
		except Exception as e:
			wx.MessageBox(str(e), "Plot Error")
			return


if __name__ == "__main__":
	frm = frmtest_z(None)
	frm.Show()


