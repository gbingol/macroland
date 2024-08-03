import numbers
import wx
import math

import scisuit.plot as plt
from scisuit.stats import aov2, aov2_results
from _sci import (Frame, GridTextCtrl, Worksheet, pnlOutputOptions,
				  Workbook, Range, 
				  parent_path, prettify)




class frmanova_twofactor ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = "Two-factor ANOVA")

		self.m_Results:dict = None
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )
		
		IconPath = parent_path(__file__) / "icons" / "anova2factor.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		self.m_lblY = wx.StaticText( self, label = "Response:")
		self.m_txtY = GridTextCtrl( self)

		self.m_lblX1 = wx.StaticText( self, label = "Factor 1:")
		self.m_txtX1 = GridTextCtrl( self)
		
		self.m_lblX2 = wx.StaticText( self, label = "Factor 2:")
		self.m_txtX2 = GridTextCtrl( self)

		WS = Workbook().activeworksheet()
		rng:Range = WS.selection()

		if rng != None and rng.ncols() == 3:
			rngResp = rng.subrange(0, 0, -1, 1)
			rng1= rng.subrange(0, 1, -1, 1)
			rng2= rng.subrange(0, 2, -1, 1)
			self.m_txtY.SetValue(str(rngResp))
			self.m_txtX1.SetValue(str(rng1))
			self.m_txtX2.SetValue(str(rng2))


		fgSzr = wx.FlexGridSizer( 0, 2, 6, 0 )
		fgSzr.AddGrowableCol( 1 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( self.m_lblY, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtY, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_lblX1, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtX1, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_lblX2, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtX2, 0, wx.ALL|wx.EXPAND, 5 )
		

		sbSzr = wx.StaticBoxSizer( wx.StaticBox( self, label = "Inspect Computation Results (Residual Plots)" ), wx.HORIZONTAL )
		self.m_btnHist = wx.Button( sbSzr.GetStaticBox(), label = "Histogram" )
		self.m_btnFitRes = wx.Button( sbSzr.GetStaticBox(), label = "Fits vs Residuals")
		sbSzr.Add( self.m_btnHist, 0, wx.ALL, 5 )
		sbSzr.Add( self.m_btnFitRes, 0, wx.ALL, 5 )

		
		self.m_pnlOutput = pnlOutputOptions( self)
		
		sdbSzr = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = "Compute" )
		sdbSzr.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label = "Close" )
		sdbSzr.AddButton( self.m_sdbSizerCancel )
		sdbSzr.Realize()

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( fgSzr, 0, wx.EXPAND, 5 )
		szrMain.Add( sbSzr, 0, wx.EXPAND, 5 )
		szrMain.Add( self.m_pnlOutput, 0, wx.EXPAND |wx.ALL, 10 )
		szrMain.Add( sdbSzr, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()
		self.Centre( wx.BOTH )


		self.m_btnHist.Bind(wx.EVT_BUTTON, self.__OnPlotChart)
		self.m_btnFitRes.Bind(wx.EVT_BUTTON, self.__OnPlotChart)

		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtn )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKBtn )



	def __OnCancelBtn( self, event ):
		self.Close()
		event.Skip()
	

	def __OnOKBtn( self, event ):
		try:
			assert self.m_txtY.GetValue() != "", "A range must be selected for response."

			FactorsRngOK = self.m_txtX1.GetValue() != "" and self.m_txtX2.GetValue() != ""
			assert FactorsRngOK, "Factors range cannot be empty, a selection must be made"
			
			YY = Range(self.m_txtY.GetValue()).tolist()
			X1 = Range(self.m_txtX1.GetValue()).tolist() 
			X2 = Range(self.m_txtX2.GetValue()).tolist() 
		
			self.m_Results:aov2_results = aov2(
				y = [i for i in YY if isinstance(i, numbers.Real)], 
				x1 = [i for i in X1 if isinstance(i, numbers.Real)], 
				x2 = [i for i in X2 if isinstance(i, numbers.Real)])
			
			WS, Row, Col = self.m_pnlOutput.Get()
			assert WS != None, "Ouput Options: Selected range is invalid."
			prtfy = self.m_pnlOutput.Prettify()
					
			Headers = [ "Source", "df","SS", "MS","F-value", "p-value"]
			WS.writelist(Headers, Row, Col, rowmajor=False)
		
			Row += 1
			
			Res = self.m_Results
			Total_DF = Res.DFFact1 + Res.DFFact2 + Res.DFinteract + Res.DFError
			Total_SS = Res.SSFact1 + Res.SSFact2 + Res.SSinteract + Res.SSError

			ListVals = [
				["Factor #1", Res.DFFact1, Res.SSFact1, Res.MSFact1, Res.FvalFact1,  Res.pvalFact1],
				["Factor #2", Res.DFFact2, Res.SSFact2,  Res.MSFact2, Res.FvalFact2, Res.pvalFact2],
				["Interaction", Res.DFinteract, Res.SSinteract, Res.MSinteract, Res.Fvalinteract, Res.pvalinteract],
				[None],
				["Error", Res.DFError, Res.SSError, Res.MSError],
				[None],
				["Total", Total_DF , Total_SS]]
			
			WS.writelist2d(ListVals, Row, Col, pretty=prtfy)

		except Exception as e:
			wx.MessageBox(str(e), "Error")

	

	def __OnPlotChart(self, event):
		evtObj = event.GetEventObject()

		try:	
			assert self.m_Results != None, "Have you performed the computation yet?"
			
			Residuals = self.m_Results.Residuals
			Fits = self.m_Results.Fits

			assert len(Residuals) >=3, "Not enough data to proceed!"
				
			if(evtObj == self.m_btnHist):
				plt.hist(Residuals, density=True)
				plt.title("Histogram of Residuals")
			
			elif(evtObj == self.m_btnFitRes):
				plt.scatter(y = Residuals, x=Fits)
				plt.title("Fitted Values vs Residuals")

			plt.show()
		
		except Exception as e:
			wx.MessageBox(str(e), "Plot Error")



if __name__ == "__main__":
	try:
		frm = frmanova_twofactor(None)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e), "Plot Error")