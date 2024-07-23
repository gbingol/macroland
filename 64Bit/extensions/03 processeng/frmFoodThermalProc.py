import numpy as np
import wx
from _sci import (Frame, GridTextCtrl, NumTextCtrl, Range, activeworksheet,
                  parent_path, pnlOutputOptions)


def FindAvg(y):
	return [sum(y[i:i+2])/2.0 for i in range(len(y))]



class frmFoodThermalProc ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent,title = u"Food Thermal Processing") 
		
		self.SetBackgroundColour( wx.Colour( 255, 199, 142 ) )
		
		IconPath = parent_path(__file__) / "icons" / "thermalprocessing.jpg"
		self.SetIcon(wx.Icon(str(IconPath)))
		
		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		self.m_st_t = wx.StaticText( self, label=u"Time:")
		self.m_txt_t = GridTextCtrl( self )
		
		self.m_st_T = wx.StaticText( self, label=u"Temperature(s):")
		self.m_txt_T = GridTextCtrl( self )

		WS = activeworksheet()
		rng:Range = WS.selection()

		if rng != None and rng.ncols() >= 2:
			ncols_T = 1 if rng.ncols() == 2 else rng.ncols() - 1 #ncols for temperature
			rng1 = rng.subrange(0, 0, -1, 1)
			rng2= rng.subrange(0, 1, -1, ncols_T)
			self.m_txt_t.SetValue(str(rng1))
			self.m_txt_T.SetValue(str(rng2))

		self.m_st_D_t = wx.StaticText( self, label=u"D (time):")
		self.m_txt_D_t = NumTextCtrl( self)
		
		self.m_st_D_T = wx.StaticText( self, label=u"D (temperature):")
		self.m_txt_D_T = NumTextCtrl( self)
		
		self.m_st_Z = wx.StaticText( self, label=u"z-value:")
		self.m_txt_Z = NumTextCtrl( self)
		
		self.m_stRefT = wx.StaticText( self, wx.ID_ANY, u"Ref Temperature:")
		self.m_txtRefT = NumTextCtrl( self, val="121")

		fgSzr = wx.FlexGridSizer( 0, 2, 5, 0 )
		fgSzr.AddGrowableCol( 1 )
		fgSzr.SetFlexibleDirection( wx.HORIZONTAL )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( self.m_st_t, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txt_t, 1, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_st_T, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txt_T, 1, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_st_D_t, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txt_D_t, 1, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_st_D_T, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txt_D_T, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_st_Z, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txt_Z, 1, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stRefT, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtRefT, 0, wx.ALL|wx.EXPAND, 5 )

		self.m_pnlOutput = pnlOutputOptions( self)

		sdbSizer = wx.StdDialogButtonSizer()
		self.m_ComputeBtn = wx.Button( self, wx.ID_OK, label = "Compute" )
		sdbSizer.AddButton( self.m_ComputeBtn )
		self.m_CloseBtn = wx.Button( self, wx.ID_CANCEL, label = "Close" )
		sdbSizer.AddButton( self.m_CloseBtn )
		sdbSizer.Realize()

		mainSzr = wx.BoxSizer( wx.VERTICAL )
		mainSzr.Add( fgSzr, 1, wx.EXPAND, 5 )
		mainSzr.Add( self.m_pnlOutput, 0, wx.EXPAND |wx.ALL, 5 )
		mainSzr.Add( sdbSizer, 0, wx.ALL|wx.ALIGN_CENTER_HORIZONTAL, 5 )

		self.SetSizerAndFit( mainSzr )
		self.Layout()
		self.Centre( wx.BOTH )

		self.m_CloseBtn.Bind( wx.EVT_BUTTON, self.__OnCancelBtn )
		self.m_ComputeBtn.Bind( wx.EVT_BUTTON, self.__OnBtnCompute )



	def __OnCancelBtn( self, event ):
		self.Close()
		event.Skip()
	


	def Compute(self, 
			 t:np.ndarray, 
			 T:np.ndarray, 
			 Dval_time:float, 
			 Dval_T:float, 
			 zvalue:float, 
			 Ref_T:float):
		assert len(t) == len(T), "Length of time and temperature data must be equal."
		
		DValue = Dval_time*10.0**((Dval_T-T)/zvalue) 
		LethRt = 10.0**((T-Ref_T)/zvalue) #Lethal rate
		
		#Although this is fairly slow, for this application it is fast enough. 
		FValue = [np.trapezoid(x=t[0:i], y=LethRt[0:i]) for i in range(1, len(t)+1)]
		
		dt = np.diff(t)
		avg_T = np.asarray(FindAvg(T), dtype=np.float64)
		DVal_avg = Dval_time*10.0**((Dval_T-avg_T)/zvalue)
		LogRed = dt/DVal_avg
		
		TotalLogRed = np.cumsum(LogRed)
		TotalLogRed = np.insert(TotalLogRed, 0, 0.0) # at time=0 TotalLogRed(1)=0

		return [LethRt, DValue, TotalLogRed, FValue]
		

	
	def __PrintVals(self, WS, Row, Col, time:np.ndarray, Temperatures:list, Results:list):
		Headers=["Time ", "Temperature", "Lethality Rate", "D Value","Total Log Reduction", "F-Value"]

		for i in range(len(Temperatures)):
			WS[Row, Col] = {'value':"Col #" + str(i+1) , 'weight':"bold"}
			Row += 1
	
			for k in range(len(Headers)):
				WS[Row, Col + k] = {'value': Headers[k], 'style':"italic"}


			for j in range(len(time)):
				Row += 1
				CurResult = Results[i]

				LethalRate = CurResult[0][j]
				DValue = CurResult[1][j]
				TotalLogRed = CurResult[2][j]
				FValue = CurResult[3][j]

				WS[Row, Col] = str(time[j])
				WS[Row, Col+1] = str(Temperatures[i][j])
				WS[Row, Col+2] = str(round(LethalRate, 3))
				WS[Row, Col+3] = str(round(DValue, 3))
				WS[Row, Col+4] = str(round(TotalLogRed, 3))
				WS[Row, Col+5] = str(round(FValue, 2))
			
			Row += 2 #result of next computation
				


	def __OnBtnCompute( self, event ):
		try:
			assert self.m_txt_Z.GetValue() != "" , "z-value cannot be blank"
			assert self.m_txt_D_T.GetValue() != "", "D(Temperature) cannot be blank"
			assert self.m_txt_D_t.GetValue() != "", "D(time) cannot be blank"
		
			zvalue = float(self.m_txt_Z.GetValue())
			Dvalue_Temp = float(self.m_txt_D_T.GetValue())
			Dvalue_Time = float(self.m_txt_D_t.GetValue())
			RefTemp = float(self.m_txtRefT.GetValue())
		
			assert Dvalue_Time>0 and  zvalue>0, "D- and z-values >0 expected"

			time = np.asarray(Range(self.m_txt_t.GetValue()).tolist(), dtype=np.float64)
			range_T = Range(self.m_txt_T.GetValue())
	

			Results, Temperatures = [], []

			for i in range(range_T.ncols()):				
				temperature = np.asarray(range_T.col(i), dtype=np.float64) 
				result = self.Compute(time, temperature, Dvalue_Time, Dvalue_Temp, zvalue, RefTemp) 			
				Results.append(result)
				Temperatures.append(temperature)
			
			WS, Row, Col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."

			self.__PrintVals(WS, Row, Col, time, Temperatures, Results )
			
		except Exception as e:
			wx.MessageBox(str(e), "Error")



if __name__ == "__main__":
	frm = frmFoodThermalProc(None) 
	frm.Show()

