import scisuit.plot as plt
from _sci import Workbook, Framework


def plot(Categ, )
Categ = ("Control", "MW", "IR")
Levels = {
   'A': (18, 18, 14),
   'B': (38, 48, 47),
   'C': (189, 195, 217)}

x = np.arange(len(Categ))  # the label locations
width = 0.25  # the width of the bars
mult = 0

for _, measure in Levels.items():
   offset = width * mult
   rects = plt.bar(x=x + offset, height=measure, width=width)
   mult += 1

plt.set_xticks(x+width, Categ)

plt.show()


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

		plt.bar()
					
		for i, d in enumerate(DataList):
			plt.boxplot(data=d, label=f"col({i+1})")	
		plt.show()			
				
	except Exception as e:
		Framework().messagebox(str(e), "Box-Whisker Error!")