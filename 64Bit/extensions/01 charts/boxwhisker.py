import scisuit.plot as plt
from _sci import Workbook, Framework



if __name__ == "__main__":
	try:
		ws = Workbook().activeworksheet()
		rng = ws.selection()
		if(rng == None):
			raise RuntimeError("A selection must be made.")

		DataList:list[list] = []
		for lst in rng.tolist(axis=0):
			data = [j for j in lst if isinstance(j, int|float)]
			if len(data)>=3:
				DataList.append(data)
		
		assert len(DataList) >0, "At least 1 column of valid data is expected."	
					
		for i, d in enumerate(DataList):
			plt.boxplot(data=d, label=f"col({i+1})")	
		plt.show()			
				
	except Exception as e:
		Framework().messagebox(str(e), "Box-Whisker Error!")