import wx
from _sci import GridTextCtrl, Workbook, Worksheet, Range, pnlOutputOptions, Frame
from _sci.util import assert_pkg

class frmAccumulator ( Frame ):

    def __init__( self, parent ):
        super().__init__ ( parent,)

        szrMain = wx.BoxSizer( wx.VERTICAL )

        szrInput = wx.BoxSizer( wx.HORIZONTAL )

        stTxt = wx.StaticText( self, wx.ID_ANY, "Data")

        szrInput.Add( stTxt, 0, wx.ALL, 5 )

        self.m_txtData = GridTextCtrl( self)
        szrInput.Add( self.m_txtData, 1, wx.ALL, 5 )


        sdbSzr = wx.StdDialogButtonSizer()
        self.m_OK = wx.Button( self, wx.ID_OK )
        sdbSzr.AddButton( self.m_OK )
        self.m_Cancel = wx.Button( self, wx.ID_CANCEL )
        sdbSzr.AddButton( self.m_Cancel )
        sdbSzr.Realize()

        self.m_pnlOutput = pnlOutputOptions(self)

        szrMain.Add( szrInput, 0, wx.EXPAND, 5 )
        szrMain.Add( self.m_pnlOutput, 0, wx.EXPAND, 5 )
        szrMain.Add( sdbSzr, 0, wx.EXPAND, 5 )

        self.SetSizer( szrMain )
        self.Layout()

        self.Centre( wx.BOTH )

        self.m_OK.Bind(wx.EVT_BUTTON, self.__OnOK)
    

    def __OnOK(self, event:wx.CommandEvent):
        #from _sci.util
        assert_pkg(name="pandas", pip="pandas")

        #_sci.Range
        data = Range(self.m_txtData.GetValue()).tolist(axis=0)

        Output = []
        for e in data:
            dt = [i for i in e if isinstance(i, int|float) ]
            Output.append(sum(dt))
        
        #ws (_sci.Worksheet)
        ws, Row, Col = self.m_pnlOutput.Get()
        assert ws != None, "Ouput Options: Selected range is invalid."	
        prtfy = self.m_pnlOutput.Prettify()

        ws.writelist(Output, Row, Col, rowmajor=False, pretty=prtfy)


if __name__ == "__main__":
    frm = frmAccumulator(None)
    frm.Show()