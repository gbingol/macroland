from wx import MessageBox
import scisuit.plot as plt
from _sci import activeworksheet



if __name__ == "__main__":
	try:
		ws = activeworksheet()
		rng = ws.selection()
		if(rng == None):
			raise RuntimeError("A selection must be made.")
		
		mainList = rng.tolist(axis=0)
		
		N = len(mainList) 
		assert N>0, "At least 1 column of data is expected."
		
					
		for i in range(N):
			data = [j for j in mainList[i] if isinstance(j, int|float)]
			assert len(data)>=3, "Selection must have at least 3 numeric entries."
		
			plt.hist(data=data, density=True)
			
			if N>1:
				plt.title("Histogram col("+str(i+1)+")")
			
			if i<(N-1):
				plt.figure()
			
		plt.show()			
				
	except Exception as e:
		MessageBox(str(e), "Histogram Error!")