import scisuit.plot as plt
from _sci import Workbook, Framework



if __name__ == "__main__":
	try:
		ws = Workbook().activeworksheet()
		rng = ws.selection()
		if(rng == None):
			raise RuntimeError("A selection must be made.")
		
		ncols = rng.ncols()
		
		HistData = None
		YesNo = 0
		if ncols>1:
			msg = ("Selection contains multiple columns.\n"
					"Should a single chart be produced?\n \n"
					f"If Yes, only a single chart combining the valid data of {ncols} columns will be displayed.\n"
					f"If No, possibly {ncols} separate charts will be displayed.")
			YesNo = Framework().messagebox(msg, "Multiple Columns?", yesno=True)

			if YesNo == 1:
				HistData = rng.tolist()
			else:
				HistData = rng.tolist(axis=0) #2D list containing ncols of list
		
		#ncols == 1 and only 1D list produced
		else:
			HistData = rng.tolist()
		

		if ncols>1 and YesNo == 0:	
			Data = []	
			for lst in HistData:
				dt = [j for j in lst if isinstance(j, int|float)]
				if len(dt)>=3:
					Data.append(dt)

			for i, lst in enumerate(Data):
				plt.hist(data=lst, density=True)
				plt.title("Histogram col("+str(i+1)+")")
				
				if i<(len(Data)-1):
					plt.figure()

		else:
			data = [j for j in HistData if isinstance(j, int|float)]
			assert len(data)>=3, "Selection must have at least 3 numeric entries."
			
			plt.hist(data=data, density=True)
			
		plt.show()			
				
	except Exception as e:
		Framework().messagebox(str(e), "Histogram Error!")