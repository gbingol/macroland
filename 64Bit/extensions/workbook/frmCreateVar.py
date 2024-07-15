import importlib
import wx

from _sci import activeworksheet, Frame, parent_path, makeicon




class pnlRange (wx.Panel ):
	def __init__( self, parent):
		wx.Panel.__init__ ( self, parent)
	
	def get(self, Rng):
		return Rng



class pnlDict ( wx.Panel ):
	def __init__( self, parent):
		wx.Panel.__init__ ( self, parent)

		self.m_chkHasHeader = wx.CheckBox( self, wx.ID_ANY, u"First Row is Header")
		
		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( self.m_chkHasHeader, 0, wx.ALL|wx.EXPAND, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()

	
	def get(self, Rng):
		return Rng.todict(self.m_chkHasHeader.GetValue())
	



class pnlList ( wx.Panel ):
	def __init__( self, parent):
		wx.Panel.__init__ ( self, parent)

		self.m_staticText = wx.StaticText( self, label = "Axis:")
		self.m_staticText.Wrap( -1 )
		self.m_rdAxisNone = wx.RadioButton( self, label = "None")
		self.m_rdAxisNone.SetValue( True )
		self.m_rdAxis0 = wx.RadioButton( self, label = "0")
		self.m_rdAxis1 = wx.RadioButton( self, label = "1")
		
		szrOpts = wx.BoxSizer( wx.HORIZONTAL )
		szrOpts.Add( self.m_staticText, 0, wx.ALL, 5 )
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
		Frame.__init__ ( self, 
			parent,
			title = u"Create Variable", 
			style=wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP)

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		ParentPath = parent_path(__file__)
		IconPath = ParentPath / "icons" / "py_logo32.png"
		self.SetIcon(makeicon(IconPath))

		self.m_Panels = [
			["dict", pnlDict], 
			["list", pnlList],
			["Range", pnlRange]]

		"""
		The frame is shown when user makes a selection on a worksheet and then
		right-clicks to see the context-menu. Context-menu will offer 
		"Create Variable" menu item. Therefore self.m_Range cannot be None. 
		
		Note that this frame is non-blocking (modeless) to ScienceSuit framework. 
		Therefore user might change the selection any time after the frame is shown; 
		however we work with the initial one.
		"""
		WS = activeworksheet()
		self.m_Range = WS.selection()
		self.m_pnlInput = wx.Panel( self)
		self.m_staticName = wx.StaticText( self.m_pnlInput, label = "Variable Name:")
		self.m_staticName.Wrap( -1 )
		self.m_txtName = wx.TextCtrl( self.m_pnlInput)
		self.m_staticType = wx.StaticText( self.m_pnlInput, label = "Type:")
		self.m_staticType.Wrap( -1 )
		self.m_choiceType = wx.Choice( self.m_pnlInput, choices = [e[0] for e in self.m_Panels])
		self.m_choiceType.SetSelection( 0 )

		fgSizer = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSizer.AddGrowableCol( 1 )
		fgSizer.SetFlexibleDirection( wx.BOTH )
		fgSizer.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSizer.Add( self.m_staticName, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtName, 1, wx.ALL, 5 )
		fgSizer.Add( self.m_staticType, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_choiceType, 0, wx.ALL, 5 )
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

		self.m_choiceType.Bind( wx.EVT_CHOICE, self.OnChoiceType )
		self.m_BtnOK.Bind( wx.EVT_BUTTON, self.OnOK )
		self.m_BtnCancel.Bind( wx.EVT_BUTTON, self.OnCancel )


	def OnChoiceType( self, event ):
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


	def OnOK( self, event ):
		try:
			Name = str(self.m_txtName.GetValue())
			assert Name.isidentifier(), Name + " is not a valid identifier."

			Value = self.m_pnlVarOpt.get(self.m_Range)
			assert type(Value) != type(None), "Could not create the variable"

			Modul = importlib.import_module("_sci.vars")
			Modul.__dict__[Name] = Value
			

		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return #dont close
		
		self.Close()


	def OnCancel( self, event ):
		self.Close()
		event.Skip()


if __name__=="__main__":
	try:
		frm = frmCreateVar(None)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e), "Error")