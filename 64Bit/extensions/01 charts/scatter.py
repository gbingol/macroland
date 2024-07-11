import scisuit.plot as plt
from _sci import activeworksheet, messagebox



if __name__ == "__main__":
	try:
		ws = activeworksheet()
		rng = ws.selection()
		if(rng == None):
			raise RuntimeError("A selection must be made.")
		
		mainList = rng.tolist(axis=0)
				
		assert len(mainList)>0, "At least 1 column of data is expected."
		
		if len(mainList) == 1:
			x = [i+1 for i in range(len(mainList[0])) if isinstance(i, int|float)]
			y = mainList[0]
			
			assert len(x)>=2, "Selection must have at least 2 numeric entries."
			plt.scatter(x=x, y=y)
		
		else:
			x = [i for i in mainList[0] if isinstance(i, int|float)]
			assert len(x)>=2, "Selection must have at least 2 numeric entries."
			
			for i in range(1, len(mainList)):
				y = [j for j in mainList[i] if isinstance(j, int|float)]
				assert len(x)==len(y), "Selection must have at least 2 numeric entries."
				
				plt.scatter(x=x, y=y)
			
		plt.show()
				
				
	except Exception as e:
		messagebox(str(e), "Scatter Error!")