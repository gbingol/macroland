import scisuit.plot as plt
from _sci import Framework

if __name__ == "__main__":
	try:
		plt.moody()
		plt.show()			
				
	except Exception as e:
		Framework().messagebox(str(e), "Moody Diagram Error!")