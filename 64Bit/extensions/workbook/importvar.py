import numpy as np
from wx import GetTextFromUser 

from _sci import (Worksheet, Range, activeworksheet, 
                  CommandWindowDict, messagebox)


class GotoLabel(Exception): 
    """Serves as a label"""
    pass

if __name__ == '__main__':
    try:
        ws = activeworksheet()

        msg = """
        Enter a valid expression that returns list | dict | str | int | float.

        Any variables used in the expression must already be defined in the 
        command window.
        """
        expression = GetTextFromUser(msg, "Enter an expression")
        if expression == "":
            raise GotoLabel("")
        
        result = eval(expression, CommandWindowDict)
        assert isinstance(result, list|dict|str|int|float), "expected list|dict|str|int|float"

        rng = ws.selection()
        ncols, nrows = 0, 0
        if rng!=None:
            ncols, nrows = rng.ncols(), rng.nrows()
        
        CurRow, CurCol = ws.cursor()
        ws.writelist(result, CurRow, CurCol)
        
    except GotoLabel:
        pass
    except Exception as e:
        messagebox(str(e), "Import Error!")
