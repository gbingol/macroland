import scisuit.plot as plt
import numpy as np
from _sci import Workbook, Framework



if __name__ == "__main__":
	
	try:
		ws = Workbook().activeworksheet()
		rng = ws.selection()
		if(rng == None):
			raise RuntimeError("A selection must be made.")
		
		SelData = rng.tolist(axis=0)		
		assert len(SelData)>0, "At least 1 column of data is expected."

		
		#If the top cell is None, then that column is the labels column for levels 
		HasLevLbls = SelData[0][0] == None
		LevLbls = [i for i in SelData[0] if isinstance(i, str)]
		
		HasLevLbls = HasLevLbls or len(LevLbls) == rng.nrows()
						
		CategRng = rng.subrange(row=0, col=1) if HasLevLbls else rng
		CategData = CategRng.tolist(axis=1)

		CatLbls = [i for i in CategData[0] if isinstance(i, str)]
		HasCategLabels = len(CatLbls) == CategRng.ncols()

		DataRng = CategRng.subrange(row=1, col=0) if HasCategLabels else CategRng
		Data = DataRng.tolist(axis=0)

		Levels = np.arange(DataRng.nrows()) 

		WIDTH = 0.25 #width of each bar
		GAP = 0.0 # gap between each category bar at a certain level
		mult = 0
		for i, hgt in enumerate(Data):
			offset = (WIDTH + GAP)*mult
			label = CatLbls[i] if HasCategLabels else f"Col {i+1}"
			plt.bar(x=Levels + offset, height=hgt, width=WIDTH, label=label)
			mult += 1

		plt.set_xticks(Levels+(WIDTH if DataRng.ncols()>1 else WIDTH/2),
						 LevLbls if HasLevLbls else Levels+1)

			
		plt.show()
				
				
	except Exception as e:
		Framework().messagebox(str(e), "Column-Clustered Error!")