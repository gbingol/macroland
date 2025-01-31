import scisuit.plot as plt
from _sci import Workbook, Framework



if __name__ == "__main__":
	try:
		ws = Workbook().activeworksheet()

		rng = ws.selection()
		assert rng != None, "A selection must be made."
		
		mainList = rng.tolist(axis=0)
		N = len(mainList)
				
		assert N <= 2, "At least 1 or at most 2 columns of data is expected."
		
		
		if N == 1:
			data = [j for j in mainList[0] if isinstance(j, int|float)]
			assert len(data)>=3, "Selection must have at least 3 numeric entries."
			
			plt.qqnorm(data = data)
		
		elif N == 2:
			dataX = [j for j in mainList[0] if isinstance(j, int|float)]
			assert len(dataX)>=3, "First selected column does not have at least 3 numeric entries."
			
			dataY = [j for j in mainList[1] if isinstance(j, int|float)]
			assert len(dataY)>=3, "Second selected column does not have at least 3 numeric entries."
			
			plt.qqplot(x=dataX, y=dataY)
			
		plt.show()		
				
	except Exception as e:
		Framework().messagebox(str(e), "Q-Q Chart Error!")