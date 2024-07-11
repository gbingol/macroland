import scisuit.plot as plt
from _sci import messagebox

if __name__ == "__main__":
	try:
		plt.psychrometry()
		plt.show()			
				
	except Exception as e:
		messagebox(str(e), "Psychrometry Error!")