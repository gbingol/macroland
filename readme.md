## MacroLand

An extensible and configurable computing environment. 



&nbsp;



## Framework

MacroLand is comprised of 3 major parts:

1. **Workbook:** Adding new Worksheets, formatting cells, saving/opening project file.

2. **Command Window:** Running single or multi-line Python commands, navigating through
history.

3. **Extension Manager:** Installing/uninstalling or enabling/disabling extensions.

&nbsp;

### Extensibility

MacroLand has extension mechanism to further its capabilities. For example, the image below shows:

- A *Toolbar* with 3 pages namely Home, *Data Tools* and *Statistics*,
- A *context menu* with Copy, Cut, Delete and *Create Variable* and *Export* menu items,
- Statistics on *selection* (max, min, sum, mean)


![Workbook](workbook.png)


The pages *Data Tools* and *Statistics*, the menu items *Create Variable* and *Export* 
and the displayed statistics on selection have all been added via extensions. Therefore, 
through extensions MacroLand environment allows you to add more pages to toolbar, new 
menu items to context menu or simply to change the displayed statistical measures. 

Although not shown in the image, it should be noted that it is also possible to 
modify other context-menus, i.e., statusbar.



&nbsp;



## Objectives 

Python has many libraries for different purposes. For example, a simple t-test can be 
performed in the following way:

```Python
import pprint
from scisuit.stats import test_t

treat = [24, 43, 58, 71, 43, 49, 61, 44, 67, 49]
cont = [42, 43, 55, 54, 20, 85, 33, 41, 19, 60, 53, 42]

pval, tbl = test_t(x=treat, y=cont, varequal=False)

print("p-value=" + str(pval))
pprint.pprint(tbl) 
```

Once a script like above-shown one is successfully implemented, imagine how
easy it would be if variables `treat` and `cont` were columns of data in 
a Worksheet and one would be selecting them 
as one would in a spreadsheet software (Excel, LibreOffice ...).

Below app simply demonstrates this purpose.


![2-sample t-test app](ttest_2sample.png)


In the above-shown app, which has been designed using **wxPython**,
*Variable #1 & #2* are data coming from two differnet selections and 
the *output* of computation can be displayed either on 
the same Worksheet or on a different one. 
With the embedded **_sci** Python library, essential tools are provided to 
integrate Python scripts to different parts of MacroLand framework.

It is the goal of this project to establish collaboration to 
be able to add more features and more apps to the framework.




&nbsp;




## Compilation

Although Visual Studio solution and project files are provided,
extra steps are needed:

- Install Python with debug libraries.
- Download and compile Lua, wxWidgets and Boost (linking will be required).
- C++20 or higher  



&nbsp;



## Contact

Feel free to drop me an email: gbingol@hotmail.com.