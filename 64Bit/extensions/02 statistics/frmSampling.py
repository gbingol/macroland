import wx
import numpy as _np

import _sci as _se


class frmSampling ( _se.Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"Sampling")
		
		ParentPath = _se.parent_path(__file__)
		IconPath = ParentPath / "icons" /  "sampling.png"
		self.SetIcon(_se.makeicon(IconPath))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 255, 192, 130 ) )

		self.m_staticTxtInput = wx.StaticText( self, wx.ID_ANY, u"Input:")
		self.m_staticTxtInput.Wrap( -1 )
		self.m_txtInput = _se.GridTextCtrl( self)

		WS = _se.activeworksheet()
		Rng = WS.selection()
		if Rng != None:
			self.m_txtInput.SetValue(str(Rng))

		#input range section
		inputSizer = wx.BoxSizer( wx.HORIZONTAL )
		inputSizer.Add( self.m_staticTxtInput, 0, wx.ALL, 5 )
		inputSizer.Add( self.m_txtInput, 1, wx.ALL, 5 )

		self.m_chkHeaders = wx.CheckBox( self, wx.ID_ANY, u"Has Headers")
		self.m_chkHeaders.SetValue(True)
	
		self.m_pnlOutput = _se.pnlOutputOptions( self )		

		m_sdbSizer = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = "Compute" )
		m_sdbSizer.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label = "Close" ) 
		m_sdbSizer.AddButton( self.m_sdbSizerCancel ) 
		m_sdbSizer.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( inputSizer, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_chkHeaders, 0, wx.ALL, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.EXPAND |wx.ALL, 5 )
		mainSizer.Add( m_sdbSizer, 0, 0, 5 )   
		
		self.SetSizerAndFit( mainSizer )
		self.Layout() 
		self.Centre( wx.BOTH )  

		
		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelButton )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKButton )



	def __OnCancelButton( self, event ):
		frm.Close()
	
		

	def __OnOKButton( self, event ):
		try:
			import pandas as pd
			from _sci.pandas import print_to_ws
			
			assert self.m_txtInput.GetValue() != "", "A data range must be selected"
			InputRng = _se.Range(self.m_txtInput.GetValue())

			WS, Row, Col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."	

			HasHeaders = self.m_chkHeaders.GetValue()
			df = pd.DataFrame(InputRng.todict(HasHeaders))
			Desc = df.describe()

			print_to_ws(Desc, WS, Row, Col)

		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return



if __name__=="__main__":
	try:
		frm = frmSampling(None)
		sz = frm.GetSize()
		sz.x=400
		frm.SetSize(sz)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e), "Error!")