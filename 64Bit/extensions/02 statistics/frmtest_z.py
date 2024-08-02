import wx
import numbers

from scisuit.stats import test_z, test_z_Result
import scisuit.plot as plt

from _sci import (Frame, GridTextCtrl, NumTextCtrl, pnlOutputOptions,
				  Workbook, Range, parent_path, prettify)


class frmtest_z ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"Z Test")

		ParentPath = parent_path(__file__)
		IconPath = ParentPath / "icons" / "test_z.png"
		self.SetIcon(wx.Icon(str(IconPath)))
		
		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )

		self.m_stVar = wx.StaticText( self, label = u"Variable Range:")
		self.m_txtVar = GridTextCtrl( self)

		WS = Workbook().activeworksheet()
		rng = WS.selection()
		if rng != None and rng.ncols() == 1:
			self.m_txtVar.SetValue(str(rng))

		self.m_stMean = wx.StaticText( self, label = u"Test Mean:")
		self.m_txtMean = NumTextCtrl( self)

		self.m_stSigma = wx.StaticText( self, label = u"Sigma:")
		self.m_txtSigma = NumTextCtrl( self)

		self.m_stConf = wx.StaticText( self, label = u"Confidence Level:")
		self.m_txtConf = NumTextCtrl( self, val= u"95", minval=0.0, maxval=100.0)
		
		self.m_stAlt = wx.StaticText( self, wx.ID_ANY, u"Alternative:")
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

		sbSizer = wx.StaticBoxSizer( wx.StaticBox( self, label="Inspect Data" ) )
		self.m_BtnInspect = wx.Button( sbSizer.GetStaticBox(), label = u"Histogram/Box-Whisker" )
		sbSizer.Add( self.m_BtnInspect, 0, wx.ALL, 5 )

		self.m_pnlOutput = pnlOutputOptions( self)	

		sdbSzr = wx.StdDialogButtonSizer()
		self.m_btnOK = wx.Button( self, wx.ID_OK, label="Compute" )
		sdbSzr.AddButton( self.m_btnOK )
		self.m_btnCancel = wx.Button( self, wx.ID_CANCEL, label="Close" )
		sdbSzr.AddButton( self.m_btnCancel )
		sdbSzr.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( fgSzr, 0, wx.EXPAND, 5 )
		mainSizer.Add( sbSizer, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.ALL|wx.EXPAND, 10 )
		mainSizer.Add( sdbSzr, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( mainSizer )
		self.Layout()
		self.Centre( wx.BOTH )
		
		self.m_btnCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtnClick )
		self.m_btnOK.Bind( wx.EVT_BUTTON, self.__OnOKBtnClick )
		self.m_BtnInspect.Bind(wx.EVT_BUTTON, self.__OnPlotChart)
	
	
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

			xdata = Range(self.m_txtVar.GetValue()).tolist()
			xdata = [i for i in xdata if isinstance(i, numbers.Real) ]
			
			pval, Res = test_z(x=xdata, mu = Mu, sd = Sigma, 
				alternative = Alternative, conflevel = conflevel)

			WS, Row, Col = self.m_pnlOutput.Get()
			assert WS != None, "Ouput Options: Selected range is invalid."	
			prtfy = self.m_pnlOutput.Prettify()

			Header=["N", "Average", "stdev", "SE Mean", "z", "p-value"]
			Vals =[Res.N, Res.mean, Res.stdev, Res.SE, Res.zcritical, pval] 
			WS.writelist(Header, Row, Col, rowmajor=False)
			Row += 1
			WS.writelist(Vals, Row, Col, rowmajor=False, pretty=prtfy)
			
			Row += 2

			Txt = f"{self.m_txtConf.GetValue()}% Confidence Interval for "
			Txt += f"( {prettify(Res.CI_lower, prtfy)}, {prettify(Res.CI_upper, prtfy)} )"
			WS[Row + 1, Col] = Txt
		
		except Exception as e:
			wx.MessageBox(str(e))
		


	def __OnPlotChart(self, event):
		try:	
			assert self.m_txtVar.GetValue() != "", "Have you made a valid selection yet?"

			xdata = Range(self.m_txtVar.GetValue()).tolist()
			assert len(xdata) >=3, "Not enough data to proceed!"
			
			plt.hist(xdata)
			plt.figure()
			plt.boxplot(xdata)
			
			plt.show()
		
		except Exception as e:
			wx.MessageBox(str(e), "Plot Error")


if __name__ == "__main__":
	try:
		frm = frmtest_z(None)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e), "Error")


