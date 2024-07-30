import numbers
import wx

from scisuit.stats import test_t, test_t1_result

import _sci as _se



class frmtestt_1sample ( _se.Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"1-sample t-test")
		
		ParentPath = _se.parent_path(__file__)
		IconPath = ParentPath / "icons" / "t_test1sample.png"
		self.SetIcon(wx.Icon(str(IconPath)))
		
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )	
		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		self.m_stVar = wx.StaticText( self, label = "Data Range:")
		self.m_txtVar = _se.GridTextCtrl( self )

		WS = _se.Workbook().activeworksheet()
		rng = WS.selection()
		if rng != None and rng.ncols() == 1:
			self.m_txtVar.SetValue(str(rng))
		
		self.m_stMean = wx.StaticText( self, label = "Test Mean:")
		self.m_txtMean = _se.NumTextCtrl( self)	

		self.m_stConf = wx.StaticText( self, label = "Confidence Level:" )
		self.m_txtConf = _se.NumTextCtrl( self, val= "95", minval=0.0, maxval=100.0)
	
		self.m_stAlt = wx.StaticText( self, label = "Alternative:")
		self.m_chcAlt = wx.Choice( self, choices = [ "less than", "not equal", "greater than" ])
		self.m_chcAlt.SetSelection( 1 )

		fgSzr = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzr.AddGrowableCol( 1 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( self.m_stVar, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtVar, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stMean, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtMean, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stConf, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtConf, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stAlt, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_chcAlt, 0, wx.ALL, 5 )	

		sbSizer = wx.StaticBoxSizer( wx.StaticBox( self, wx.ID_ANY, "Inspect Selected Data" ), wx.HORIZONTAL )
		self.m_BtnInspect = wx.Button( sbSizer.GetStaticBox(), label = "Histogram/Box-Whisker" )
		sbSizer.Add( self.m_BtnInspect, 0, wx.ALL, 5 )

		self.m_pnlOutput = _se.pnlOutputOptions( self )
		
		sdbSizer = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = u"Compute" )
		sdbSizer.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label = u"Close" )
		sdbSizer.AddButton( self.m_sdbSizerCancel )
		sdbSizer.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( fgSzr, 0, wx.EXPAND, 5 )
		mainSizer.Add( sbSizer, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.ALL|wx.EXPAND, 5 )
		mainSizer.Add( sdbSizer, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( mainSizer )
		self.Layout()
		self.Centre( wx.BOTH )
		
		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtn )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKBtn )

		self.m_BtnInspect.Bind(wx.EVT_BUTTON, self.__OnPlotChart)

	
	def __OnCancelBtn( self, event ):
		self.Close()


	def __OnOKBtn( self, event ):
		try:
			assert self.m_txtVar.GetValue() != "", "Selection expected for variable."
			assert self.m_txtMean.GetValue() != "", "A value must be entered for the test mean."
			
			conflevel = float(self.m_txtConf.GetValue())/100
			assert conflevel>0 and conflevel<1, "Confidence interval must be in (0, 100)"

			Mu = float(self.m_txtMean.GetValue())

			AlterOpt = ["less", "two.sided", "greater"]
			Alternative = AlterOpt[self.m_chcAlt.GetSelection()]
			
			Data = _se.Range(self.m_txtVar.GetValue()).tolist()
			Data = [i for i in Data if isinstance(i, numbers.Real)]
			assert len(Data)>2, "At least 3 data points expected"
			
			pval, Res = test_t(x=Data, mu=Mu, alternative = Alternative, conflevel = conflevel)

			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."
			
			Vals = [["N", Res.N], ["Average", Res.mean], ["stdev",Res.stdev],
				["SE Mean", Res.SE], ["T",Res.tcritical], ["p value", pval]]
			
			InitCol = col
			for elem in Vals:
				WS[row, col] = elem[0]
				WS[row + 1, col] = elem[1]
				col += 1
			
			row += 3
			col = InitCol

			WS[row, col] = self.m_txtConf.GetValue() + \
				"% Confidence Interval for " + \
				"(" + str(round(Res.CI_lower, 4)) + ", " + str(round(Res.CI_upper, 4)) + ")"

		except Exception as e:
			_se.messagebox(str(e))
			return
	

	def __OnPlotChart(self, event):
		import scisuit.plot as plt

		try:	
			assert self.m_txtVar.GetValue() != "", "Have you made a valid selection yet?"

			xdata = _se.Range(self.m_txtVar.GetValue()).tolist()
			assert len(xdata) >=3, "Not enough data to proceed!"
			
			plt.hist(xdata)
			plt.figure()
			plt.boxplot(xdata)
			
			plt.show()
				
		except Exception as e:
			_se.messagebox(str(e), "Plot Error")
			return



if __name__ == "__main__":
	frm =frmtestt_1sample(None)
	frm.Show()