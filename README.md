# A Computing Environment for Engineers

*MacroLand* is an extensible and customizable computing environment designed with engineers 
in mind. It is mostly written in C++ and uses Python for extending and customizing the framework. 

&nbsp;

The framework is essentially comprised of 3 parts:
1. Workbook
1. Command window
1. Apps



## Workbook
Among the several functions of the workbook, most notable ones are:
1. Saving and loading data (.sproj file format)
1. Formatting and manipulating the data,
1. Creating a Python variable from a selection,
1. Exporting/importing data from CSV or text files.


![workbook](workbook.png)


&nbsp;



## Command-window 
The command-window (integrated with workbook) serves:  
1. To work with a variable created via workbook,
1. To run multiple or single line commands,
1. Code highlighting and code completion.

![Command Window](command_wnd.png)



&nbsp;


## Extensions
An extension is essentially a collection of programs written in Python using a GUI library 
(currently wxPython is used). Once a collection is prepared, then it is very easy to 
integrate to the framework: Simply follow the below steps:

1. Under extensions folder, create a folder.
1. Create *__init__.py* file.
1. Import builtin *_sci* library.

```Python
import pathlib

import _sci.extension as ext
from _sci import Framework, ToolBar
```

4. Create a button
``` Python
def run(x):
	Framework().RunPyFile(x)

if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	btnFoodDB = ext.Button("Food DB", 
                    CurFolder/"icons/fooddatabase.jpg", 
                    run, CurFolder/"frmFoodDatabase.py")
```

5. Create a page and add the buttons
```Python
    #continuing from the previous code
    page = ext.Page("Process Eng")
	page.add(btnFoodDB)
```

6. Add the page to the toolbar
```Python
     #continuing from the previous code
    ToolBar().AddPage(page)
```



Currently there are 4 toolbar pages:  

- Home (Default first page)  
![Home toolbar](toolbar_home.png)

- Charts  
![Charts toolbar](toolbar_charts.png)

- Statistics  
![Statistics toolbar](toolbar_stats.png)

- Process Engineering  
![Process Engineering toolbar](toolbar_proceng.png)


**Note*:* Except the first part of the *Home* page, the rest are added dynamically by extensions.  



&nbsp;


## Apps

Apps for different purposes can be developed; i.e. an app with merely computational purposes or 
another app which directly works with the data from the workbook.  

For example, based on a selection (range), such as *response* and *factor(s)* 
shown in the linear regression app, the selected data can be analyzed conveniently.
Selection can be modified by editing the text (Sheet 1:H3:H22) or simply by making a new select.  

![Apps](apps.png)


&nbsp;

---

The framework relies on the following libraries:
- Numpy 
- scisuit
- wxWidgets
- wxPython