import importlib
import wx

from _sci import Workbook, Frame, parent_path, CommandWindowDict, wxmessagebox



class pnlRange (wx.Panel ):
	def __init__( self, parent):
		super().__init__ (parent)
	
	def get(self, Rng):
		return Rng



class pnlDict ( wx.Panel ):
	def __init__( self, parent):
		super().__init__ (parent)

		self.m_chkHasHeader = wx.CheckBox( self, wx.ID_ANY, u"First Row is Header")
		
		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( self.m_chkHasHeader, 0, wx.ALL|wx.EXPAND, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()
	
	def get(self, Rng):
		return Rng.todict(self.m_chkHasHeader.GetValue())
	



class pnlList ( wx.Panel ):
	def __init__( self, parent):
		super().__init__ (parent)

		self.m_stText = wx.StaticText( self, label = "Axis:")
		self.m_rdAxisNone = wx.RadioButton( self, label = "None")
		self.m_rdAxisNone.SetValue( True )
		self.m_rdAxis0 = wx.RadioButton( self, label = "0")
		self.m_rdAxis1 = wx.RadioButton( self, label = "1")
		
		szrOpts = wx.BoxSizer( wx.HORIZONTAL )
		szrOpts.Add( self.m_stText, 0, wx.ALL, 5 )
		szrOpts.Add( self.m_rdAxisNone, 0, wx.ALL, 5 )
		szrOpts.Add( self.m_rdAxis0, 0, wx.ALL, 5 )
		szrOpts.Add( self.m_rdAxis1, 0, wx.ALL, 5 )

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( szrOpts, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()
	

	def get(self, Rng):
		if self.m_rdAxisNone.GetValue():
			return Rng.tolist()

		Axis = 0 if self.m_rdAxis0.GetValue() else 1
		return Rng.tolist(Axis)




class frmCreateVar ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent,
			title = u"Create Variable", 
			style=wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP)

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		IconPath = parent_path(__file__) / "icons" / "py_logo32.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.m_Panels = [
			["dict", pnlDict], 
			["list", pnlList],
			["Range", pnlRange]]

		"""
		Note that this frame is non-blocking (modeless) to MacroLand framework. 
		Therefore user might change the selection any time after the frame is shown; 
		however we work with the initial one.
		"""
		WS = Workbook().activeworksheet()
		self.m_Range = WS.selection()
		self.m_pnlInput = wx.Panel( self)

		self.m_stName = wx.StaticText( self.m_pnlInput, label = "Variable Name:")
		self.m_txtName = wx.TextCtrl( self.m_pnlInput)
		self.m_stType = wx.StaticText( self.m_pnlInput, label = "Type:")

		self.m_chType = wx.Choice( self.m_pnlInput, choices = [e[0] for e in self.m_Panels])
		self.m_chType.SetSelection( 0 )

		fgSizer = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSizer.AddGrowableCol( 1 )
		fgSizer.SetFlexibleDirection( wx.BOTH )
		fgSizer.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSizer.Add( self.m_stName, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtName, 1, wx.ALL, 5 )
		fgSizer.Add( self.m_stType, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_chType, 0, wx.ALL, 5 )
		self.m_pnlInput.SetSizer( fgSizer )
		self.m_pnlInput.Layout()

		self.m_pnlVarOpt = pnlDict( self)
	
		self.m_pnlOKCancel = wx.Panel( self)
		self.m_BtnOK = wx.Button( self.m_pnlOKCancel, label = "OK")	
		self.m_BtnCancel = wx.Button( self.m_pnlOKCancel, label = "Cancel")
	
		sizerOKCancel = wx.BoxSizer( wx.HORIZONTAL )
		sizerOKCancel.Add( self.m_BtnOK, 0, wx.ALL, 5 )
		sizerOKCancel.Add( self.m_BtnCancel, 0, wx.ALL, 5 )
		self.m_pnlOKCancel.Layout()
		self.m_pnlOKCancel.SetSizerAndFit( sizerOKCancel )
		
		self.szrMain = wx.BoxSizer( wx.VERTICAL )
		self.szrMain.Add( self.m_pnlInput, 1, wx.EXPAND |wx.ALL, 5 )
		self.szrMain.Add( self.m_pnlVarOpt, 0, wx.EXPAND |wx.ALL, 5 )
		self.szrMain.Add( self.m_pnlOKCancel, 0, wx.EXPAND|wx.ALL, 5 )
		self.Layout()
		self.SetSizerAndFit( self.szrMain )
		
		self.Centre( wx.BOTH )

		self.m_chType.Bind( wx.EVT_CHOICE, self.__OnChoiceType )
		self.m_BtnOK.Bind( wx.EVT_BUTTON, self.__OnOK )
		self.m_BtnCancel.Bind( wx.EVT_BUTTON, self.__OnCancel )


	def __OnChoiceType( self, event ):
		self.szrMain.Detach(self.m_pnlVarOpt)
		self.szrMain.Detach(self.m_pnlOKCancel)

		self.m_pnlVarOpt.Destroy()

		SelPanel = self.m_Panels[event.GetSelection()][1]
		self.m_pnlVarOpt = SelPanel(self)
		self.m_pnlVarOpt.Layout()
		self.m_pnlVarOpt.Refresh()

		self.szrMain.Add(self.m_pnlVarOpt, 0, wx.EXPAND |wx.ALL, 5 )
		self.szrMain.Add(self.m_pnlOKCancel, 0, wx.EXPAND |wx.ALL, 5 )
		self.Layout()
		self.Fit()

		event.Skip()


	def __OnOK( self, event ):
		try:
			Name = str(self.m_txtName.GetValue())
			assert Name.isidentifier(), Name + " is not a valid identifier."

			Value = self.m_pnlVarOpt.get(self.m_Range)
			assert type(Value) != type(None), "Could not create the variable"

			CommandWindowDict[Name] = Value

		except Exception as e:
			wxmessagebox(str(e), "Error")
			return #dont close
		
		self.Close()


	def __OnCancel( self, event ):
		self.Close()
		event.Skip()


if __name__=="__main__":
	try:
		frm = frmCreateVar(None)
		frm.Show()
	except Exception as e:
		wxmessagebox(str(e), "Error")