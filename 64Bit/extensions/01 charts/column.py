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

		#Single category, only levels selected
		if(len(SelData) == 1):
			height = [i for i in SelData[0] if isinstance(i, int|float)]
			ticks = np.arange(len(height))
			plt.bar(x=ticks, height=height)
			plt.set_xticks(ticks + 0.4, ticks)
		
		else:
			Levellabels = [i for i in SelData[0] if isinstance(i, str)]
			LevelsLabeled = len(Levellabels) > 0
			
			RawRng = rng.subrange(row=0, col=1) if LevelsLabeled else rng
			RawData = RawRng.tolist(axis=1)

			CatLbls = [i for i in RawData[0] if isinstance(i, str)]
			CatLabeled = len(CatLbls)>0

			DataRng = RawRng.subrange(row=1, col=0) if CatLabeled else RawRng
			Data = DataRng.tolist(axis=0)

			Levels = np.arange(DataRng.nrows()) 

			width = 0.25 
			mult = 0

			for i, hgt in enumerate(Data):
				offset = width * mult
				label = CatLbls[i] if CatLabeled else f"Col {i+1}"
				plt.bar(x=Levels + offset, height=hgt, width=width, label=label)
				mult += 1

			plt.set_xticks(Levels+width, Levellabels if LevelsLabeled else Levels+1)

			
		plt.show()
				
				
	except Exception as e:
		Framework().messagebox(str(e), "Scatter Error!")