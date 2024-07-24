import wx
from . import GridTextCtrl, Range, Worksheet


__all__ = ['pnlOutputOptions']


class pnlOutputOptions ( wx.Panel ):

	def __init__( self, parent, bgcolor = None):
		super().__init__ (parent)

		#imitate parent's background color to blend well
		if bgcolor == None:
			self.SetBackgroundColour(parent.GetBackgroundColour())
		else:
			if isinstance(bgcolor, wx.Colour) == False:
				raise TypeError("bgcolor must be of type wx.Colour")
			self.SetBackgroundColour(bgcolor)

		#header section
		sizerHeader = wx.BoxSizer( wx.HORIZONTAL )
		self.m_staticText = wx.StaticText( self, wx.ID_ANY, u"Ouput Options" )

		sizerHeader.Add( self.m_staticText, 0, wx.ALL, 5 )
		self.m_staticlineHeader = wx.StaticLine( self )
		sizerHeader.Add( self.m_staticlineHeader, 1, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

		#selection section
		self.m_radioSelection = wx.RadioButton( self, wx.ID_ANY, u"Selection")
		
		self.m_txtSelRange = GridTextCtrl( self)
		self.m_txtSelRange.Enable(False) #radiobox is selected, so disable it

		self.m_radioNewWS = wx.RadioButton( self, wx.ID_ANY, u"New Sheet")
		self.m_radioNewWS.SetValue( True ) #by default new worksheet
		
		fgSizer = wx.FlexGridSizer( 0, 2, 10, 0 )
		fgSizer.AddGrowableCol( 1 )
		fgSizer.SetFlexibleDirection( wx.BOTH )
		fgSizer.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSizer.Add( self.m_radioSelection, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtSelRange, 0, wx.ALL|wx.EXPAND, 5 )
		fgSizer.Add( self.m_radioNewWS, 0, wx.ALL, 5 )

		#footer section
		sizerFooter = wx.BoxSizer( wx.HORIZONTAL )
		self.m_stlineFooter = wx.StaticLine( self)
		sizerFooter.Add( self.m_stlineFooter, 1, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( sizerHeader, 0, wx.EXPAND, 5 )
		szrMain.Add( fgSizer, 0, wx.EXPAND, 5 )
		szrMain.Add( sizerFooter, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()

		self.m_radioSelection.Bind( wx.EVT_RADIOBUTTON, self.__OnRadioSelection )
		self.m_radioNewWS.Bind( wx.EVT_RADIOBUTTON, self.__OnRadioNewWS )



	def __OnRadioSelection( self, event ):
		self.m_txtSelRange.Enable()
		event.Skip()


	def __OnRadioNewWS( self, event ):
		self.m_txtSelRange.Enable(False)
		event.Skip()
	

	def __GetSelRange(self):
		if self.m_radioSelection.GetValue() == False or self.m_txtSelRange.GetValue() == "":
			return None

		try:
			rng = Range(self.m_txtSelRange.GetValue())
			return rng
		except Exception as e:
			return None

	

	def Get(self)->tuple:
		"""
		Returns worksheet (new or where range is), row and col indexes
		If there is a selection and selected range is not valid returns: None, -1, -1 
		If new worksheet returns: WS, 0, 0
		"""
		WS = None
		row, col = -1, -1
		
		if self.m_radioNewWS.GetValue(): #new worksheet
			WS = Worksheet()
			row, col = 0, 0
		else:
			SelRange = self.__GetSelRange()

			if SelRange != None:
				WS = SelRange.parent()
				row, col = SelRange.coords()[0] #[0]:top-left
		
		return WS, row, col

