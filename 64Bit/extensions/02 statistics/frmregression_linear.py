import math

import numpy as np
import scisuit.stats as stat
import wx

from _sci import (Frame, GridTextCtrl, NumTextCtrl, Range, Worksheet,
                  activeworksheet, parent_path, pnlOutputOptions)


def _round(num:float)->float:
	if not isinstance(num, float):
		return num

	_num = float(num)	
	Digits = math.log10(abs(_num))
	if Digits>=3:
		return round(_num, 1)
	
	if(Digits>=0):
		return round(_num, 2)
	
	return round(_num, 4)




class frmregression_linear (Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"Linear Regression")

		#Regression equation coefficients to be used for residuals
		self.m_Coefficients = None
		self.m_Response = None
		self.m_Factors = None

		IconPath = parent_path(__file__) / "icons" / "regression.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 208, 232, 232 ) )


		self.m_lblResponse = wx.StaticText( self, label = u"Response:")
		self.m_lblResponse.Wrap( -1 )
		self.m_txtResponse = GridTextCtrl( self)
		
		self.m_lblFactors = wx.StaticText( self, label = u"Factor(s):")
		self.m_lblFactors.Wrap( -1 )
		self.m_txtFactors = GridTextCtrl( self)

		WS = activeworksheet()
		rng:Range = WS.selection()

		if rng != None and rng.ncols() >= 2:
			N = 1 if rng.ncols() == 2 else rng.ncols() - 1 #ncols for factors
			rng1 = rng.subrange(0, 0, -1, 1)
			rng2= rng.subrange(0, 1, -1, N)
			self.m_txtResponse.SetValue(str(rng1))
			self.m_txtFactors.SetValue(str(rng2))

		self.m_lblConfidence = wx.StaticText( self, label = u"Confidence Level:")
		self.m_lblConfidence.Wrap( -1 )
		self.m_txtConfidence = NumTextCtrl( self, val= u"95", minval=0.0, maxval=100.0)
		self.m_chkZeroIntercept = wx.CheckBox( self, label = u"intercept = 0")

		fgSizer = wx.FlexGridSizer( 0, 2, 5, 0 )
		fgSizer.AddGrowableCol( 1 )
		fgSizer.SetFlexibleDirection( wx.BOTH )
		fgSizer.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSizer.Add( self.m_lblResponse, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtResponse, 0, wx.ALL|wx.EXPAND, 5 )
		fgSizer.Add( self.m_lblFactors, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtFactors, 0, wx.ALL|wx.EXPAND, 5 )
		fgSizer.Add( self.m_lblConfidence, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtConfidence, 0, wx.ALL|wx.EXPAND, 5 )
		fgSizer.Add( self.m_chkZeroIntercept, 0, wx.ALL, 5 )
		fgSizer.Add( ( 0, 0), 1, wx.EXPAND, 5 )


		self.m_chkStats = wx.CheckBox( self, label = u"Include stats (ANOVA, R2, table of coeffs)")
		self.m_chkStats.SetValue(True)	

		sbSizer = wx.StaticBoxSizer( wx.StaticBox( self, label=u"Inspect Computation Results (Residual Plots)" ) )
		self.m_BtnHistogram = wx.Button( sbSizer.GetStaticBox(), label = u"Histogram" )
		self.m_BtnFitsResiduals = wx.Button( sbSizer.GetStaticBox(), label = u"Fits vs Residuals")
		sbSizer.Add( self.m_BtnHistogram, 0, wx.ALL, 5 )
		sbSizer.Add( self.m_BtnFitsResiduals, 0, wx.ALL, 5 )

		self.m_pnlOutput = pnlOutputOptions( self)
		
		m_sdbSizer = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = "Compute" )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label = "Close" )
		m_sdbSizer.AddButton( self.m_sdbSizerOK )
		m_sdbSizer.AddButton( self.m_sdbSizerCancel )
		m_sdbSizer.Realize()

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( fgSizer, 0, wx.EXPAND, 5 )
		szrMain.Add( self.m_chkStats, 0, wx.ALL, 5 )
		szrMain.Add( (0, 15), 1, wx.EXPAND, 5  ) #spacer
		szrMain.Add( sbSizer, 0, wx.ALL, 5 )
		szrMain.Add( (0, 15), 1, wx.EXPAND, 5  ) #spacer
		szrMain.Add( self.m_pnlOutput, 0, wx.EXPAND |wx.ALL, 5 )
		szrMain.Add( m_sdbSizer, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()
		self.Centre( wx.BOTH )

		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtnClick )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKBtnClick )

		self.m_BtnHistogram.Bind(wx.EVT_BUTTON, self.__OnPlotChart)
		self.m_BtnFitsResiduals.Bind(wx.EVT_BUTTON, self.__OnPlotChart)



	def __del__( self ):
		pass


	def __OnCancelBtnClick( self, event ):
		self.Close()
	

	def __PrintValues(self, Vals:list, WS:Worksheet, Row:int, Col:int):
		Coeffs=Vals[0]
		Stats:stat.linregressResult = Vals[1]

		WS[Row, Col] = "Linear Regression Table"
		WS[Row+1, Col] = str(round(Stats.R2, 3))

		Row += 3

		Headers = [ "", "df","SS", "MS","F", "p-value"]
		for i in range(len(Headers)):
			WS[Row, Col + i] = Headers[i]
		
		Row += 1

		AOV = Stats.ANOVA
		DF_Total = AOV["DF_Regression"] + AOV["DF_Residual"]

		AOV_Vals = [
			["Regression", AOV["DF_Regression"], AOV["SS_Regression"] , AOV["MS_Regression"], AOV["Fvalue"], AOV["pvalue"]],
			["Residual", AOV["DF_Residual"], AOV["SS_Residual"] , AOV["MS_Residual"]],
			["Total", DF_Total, AOV["SS_Total"]]]
		
		for List in AOV_Vals:
			if(List[0] == None):
				Row += 1
				continue
				
			for i in range(len(List)): 
				WS[Row, Col+i] = str(_round(List[i])) 
				
			Row += 1

		
		Row += 2

		CoeffStat = Stats.coeffstat #list
		CoeffHeaders = [ "", "Coefficient","Std Err", "T Value", "p-value", "CI"]
		for i in range(len(CoeffHeaders)):
			WS[Row, Col + i] = CoeffHeaders[i]
		
		Row += 1
		
		HasIntercept = not self.m_chkZeroIntercept.GetValue()

		j = 0
		for i in range(len(CoeffStat)):
			Dic = CoeffStat[i]

			j = i
			if(HasIntercept == False):
				j = i + 1

			if(i == 0 and HasIntercept):
				WS[Row, Col] = "Intercept"
			else:
				WS[Row, Col] = "Variable " + str(j)
			
			
			WS[Row, Col + 1] = str(_round(Dic["coeff"]))
			WS[Row, Col + 2] = str(_round(Dic["SE"])) 
			WS[Row, Col + 3] = str(_round(Dic["tvalue"]))
			WS[Row, Col + 4] = str(_round(Dic["pvalue"]))
			WS[Row, Col + 5] = str(round(Dic["CILow"], 3)) + ", " + str(round(Dic["CIHigh"], 3))

			Row += 1
		
		return



	def __OnOKBtnClick( self, event ):
		try:
			assert self.m_txtResponse.GetValue() != "", "A range must be selected for response."
			assert self.m_txtFactors.GetValue() != "Factors range cannot be empty"			
			assert self.m_txtConfidence.GetValue() != "", "A range must be selected for response."
				
			conflevel = float(self.m_txtConfidence.GetValue())/100
			Alpha = 1 - conflevel

			assert conflevel>0 or conflevel<1, "Confidence level must be in range (0, 100)"
			
			self.m_Response = np.asarray(Range(self.m_txtResponse.GetValue()).tolist(), dtype=np.float64)
			FactorsRng = Range(self.m_txtFactors.GetValue())

			NFactors = FactorsRng.ncols()
			self.m_Factors = []

			if NFactors == 1:
				self.m_Factors = np.asarray(FactorsRng.tolist(), dtype=np.float64)
			else:
				self.m_Factors = np.asarray(FactorsRng.tolist(axis = 1), dtype=np.float64)
		
			HasIntercept = not self.m_chkZeroIntercept.GetValue()

			self.m_Regression = stat.linregress(self.m_Response, self.m_Factors, HasIntercept, Alpha)
			self.m_Coefficients = self.m_Regression.compute()

			if self.m_chkStats.GetValue():
				StatSummary = self.m_Regression.summary()
				
			WS, Row, Col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."
			
			#if no stats required just print the equation
			if self.m_chkStats.GetValue() == False:
				WS[Row, Col] = str(self.m_Regression)
				return
			
			#detailed stats required
			self.__PrintValues([self.m_Coefficients, StatSummary], WS, Row, Col)
		
		except Exception as e:
			wx.MessageBox(str(e))
			return
		
		
		event.Skip()
	



	def __OnPlotChart(self, event):
		evtObj = event.GetEventObject()

		import scisuit.plot as plt

		try:	
			assert self.m_Coefficients!= None, "Have you performed the computation yet?"
			Residuals, Fits = self.m_Regression.residuals()

			assert len(Residuals) >=3, "Not enough data to proceed!"

			if(evtObj == self.m_BtnHistogram):
				plt.hist(Residuals, density=True)
				plt.title("Histogram of Residuals")
			
			elif(evtObj == self.m_BtnFitsResiduals):
				plt.scatter(y = Residuals, x=Fits)
				plt.title("Fitted Values vs Residuals")

			plt.show()
				
		except Exception as e:
			wx.MessageBox(str(e), "Plot Error")
			return
	


if __name__ == "__main__":
	frm = frmregression_linear(None)
	frm.Show()