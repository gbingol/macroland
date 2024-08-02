import numbers
import wx

import numpy as np

from scisuit.stats import test_sign, test_sign_Result
from _sci import (GridTextCtrl, NumTextCtrl, Range, Workbook, Worksheet, 
				  parent_path, Frame, pnlOutputOptions)


class frmtest_sign ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"Sign Test")

		IconPath = parent_path(__file__) / "icons" / "test_sign.png"
		self.SetIcon(wx.Icon(str(IconPath)))
	
		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )

		self.m_stVar = wx.StaticText( self, label = u"Variable Range:")
		self.m_txtVar = GridTextCtrl( self)

		WS = Workbook().activeworksheet()
		rng: Range = WS.selection()
		if rng != None and rng.ncols() == 1:
			self.m_txtVar.SetValue(str(rng))
		
		self.m_stSample2 = wx.StaticText( self, label = u"Second Sample Range:")
		self.m_stSample2.Enable( False )
		self.m_txtSample2 = GridTextCtrl( self)
		self.m_txtSample2.Enable( False )

		self.m_stMedian = wx.StaticText( self, label="Test Median:")
		self.m_txtMedian = NumTextCtrl( self, val = u"0.0")
		
		self.m_stConf = wx.StaticText( self, label="Confidence Level:")
		self.m_txtConf = NumTextCtrl( self, val="95", minval=0.0, maxval=100.0)
		
		self.m_stAlt = wx.StaticText( self, label="Alternative:")
		self.m_chcAlt = wx.Choice( self, choices = ["less than", "not equal", "greater than" ])
		self.m_chcAlt.SetSelection( 1 )
		
		self.m_chkPaired = wx.CheckBox( self, label="Paired test")

		fgSzr = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzr.AddGrowableCol( 1 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( self.m_stVar, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtVar, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stSample2, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtSample2, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stMedian, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtMedian, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stConf, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtConf, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stAlt, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_chcAlt, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_chkPaired, 0, wx.ALL, 5 )

		self.m_pnlOutput = pnlOutputOptions( self)
		
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
	
		self.m_chkPaired.Bind( wx.EVT_CHECKBOX, self.__chkPaired_OnCheckBox )
		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelButtonClick )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKButtonClick )


	
	def __chkPaired_OnCheckBox( self, event:wx.CommandEvent ):
		if event.IsChecked():
			self.m_stVar.SetLabel("First Sample Range:")
		else:
			self.m_stVar.SetLabel("Variable Range:")
		
		self.m_stSample2.Enable(event.IsChecked())
		self.m_txtSample2.Enable(event.IsChecked())


	def __OnCancelButtonClick( self, event:wx.CommandEvent ):
		self.Close()



	def __OnOKButtonClick( self, event ):
		try:
			assert self.m_txtVar.GetValue() != "", "Selection expected for variable #1."
			
			if self.m_chkPaired.GetValue():
				assert self.m_txtSample2.GetValue != "", "Selection expected for second sample."
				
			Conf = float(self.m_txtConf.GetValue())/100
			AsmdMd = float(self.m_txtMedian.GetValue()) #assumed median

			Sel = self.m_chcAlt.GetSelection()
			Alt = (["less", "two.sided", "greater"])[Sel]
			AltSign = (["<", "!=", ">"])[Sel]
			
			XX, YY, Diff = None, None, None #Diff = XX-YY
			
			X = Range(self.m_txtVar.GetValue()).tolist()
			XX = np.asarray([i for i in X if isinstance(i, numbers.Real)])		
			
			if self.m_chkPaired.GetValue():
				Y = Range(self.m_txtSample2.GetValue()).tolist()
				YY = np.asarray([i for i in Y if isinstance(i, numbers.Real)])
				assert len(XX) == len(YY), "Paired test: Variables must be of same size."		
				Diff = XX - YY
			

			pval, Res = test_sign(x = XX, y = YY, md = AsmdMd, alternative = Alt, conflevel = Conf)
			
			N = len(XX) #in case of Arr2, len(Arr1)=len(Arr2)
			NG = len(np.argwhere(Diff > AsmdMd if Diff!=None else XX>AsmdMd))
			NE = len(np.argwhere(Diff == AsmdMd if Diff!=None else XX == AsmdMd))
			
			CompMd = np.median(XX if Diff==None else Diff) #computed median

			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: Range is invalid."
			prtfy = self.m_pnlOutput.Prettify()
				
			Lower, Interp, Upper = Res.lower, Res.interpolated, Res.upper
			Vals = [
				["N", N],
				[f"N>{AsmdMd}", NG],
				[f"N={AsmdMd}", NE],
				[None],
				["Median", CompMd],
				[None],
				[f"Median={AsmdMd} vs Median{AltSign}{AsmdMd}"],
				["p-value", pval],
				[None],
				["CONFIDENCE INTERVALS"],
				["Lower Achieved", Lower.prob, Lower.CILow, Lower.CIHigh],
				["Interpolated", Interp.prob, Interp.CILow, Interp.CIHigh],
				["Interpolated", Upper.prob, Upper.CILow, Upper.CIHigh]]
			
			WS.writelist2d(Vals, row, col, pretty=prtfy)

		except Exception as e:
			wx.MessageBox(str(e))


if __name__ == "__main__":
	frm = frmtest_sign(None)
	frm.Show()


