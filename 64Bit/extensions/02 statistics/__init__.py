import pathlib

import _sci.extension as ext
from _sci.framework import Framework


def run(x):
	Framework().RunPyFile(x)

if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	btnRandNum = ext.Button("Number Gen", 
						 CurFolder / "icons/randomnumgener.jpg",
						 CurFile,
						 run, CurFolder / "frmrandnumgener.py")
		
		
	btnSampling = ext.Button("Sampling",
						  CurFolder / "icons/sampling.png",
						  CurFile,
						  run, CurFolder / "frmSampling.py")


	btnTestNormality = ext.Button("Test Norm",
							   CurFolder / "icons/testnorm.jpg",
							   CurFile,
							   run, CurFolder / "frmtestnormality.py")


	btnZTest = ext.Button("Z-test", 
					   CurFolder / "icons/test_z.png",
					   CurFile,
					   run, CurFolder / "frmtest_z.py")


	btnFTest = ext.Button("F-test", 
					   CurFolder / "icons/test_f.png",
					   CurFile,
					   run, CurFolder / "frmtest_f.py")

	btnSignTest = ext.Button("Sign Test", 
						  CurFolder / "icons/test_sign.png",
						  CurFile,
						  run, CurFolder / "frmtest_sign.py")

	btnttest1 = ext.Button("1 sample t-test", 
						CurFolder / "icons/t_test1sample.png",
						CurFile,
						run, CurFolder / "frmtestt_1sample.py")

	btnttest2 = ext.Button("2 sample t-test", 
						CurFolder / "icons/t_test2sample.png",
						CurFile, 
						run, CurFolder / "frmtestt_2sample.py")

	btnttest_paired = ext.Button("Paired t-test", 
							  CurFolder / "icons/t_testpaired.png",
							  CurFile,
							  run, CurFolder / "frmtestt_paired.py")


	btnAOV1_Unstacked = ext.Button("ANOVA 1 (Unstacked)", 
								CurFolder / "icons/anovasinglefactor.png",
								CurFile,
								run, CurFolder / "frmaov1_unstk.py")


	btnAOV1_Stacked = ext.Button("ANOVA 1 (Stacked)", 
							  CurFolder / "icons/anovasinglefactor.png",
							  CurFile,
							  run, CurFolder / "frmaov1_stk.py")


	btnANOVA2 = ext.Button("2-way ANOVA", 
						CurFolder / "icons/anova2factor.png",
						CurFile,
						run, CurFolder / "frmaov2.py")

	btnLinearRegres = ext.Button("Linear Regression", 
							  CurFolder / "icons/regression.png",
							  CurFile,
							  run, CurFolder / "frmregression_linear.py")



	btntest_t = ext.HybridButton(btnttest2)
	btntest_t.add(btnttest1)
	btntest_t.add(btnttest_paired)


	btnAOV1 = ext.HybridButton(btnAOV1_Unstacked)
	btnAOV1.add(btnAOV1_Stacked)


	page = ext.Page("Statistics") 
	page.add(btnRandNum)
	page.add(btnSampling)
	page.add(btnTestNormality)
	page.add(btnFTest)
	page.add(btnZTest)
	page.add(btntest_t)
	page.add(btnSignTest)
	page.add(btnAOV1)
	page.add(btnANOVA2)
	page.add(btnLinearRegres)

	Framework().ToolBar_AddPage(page)