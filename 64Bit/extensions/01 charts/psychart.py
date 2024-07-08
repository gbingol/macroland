from wx import MessageBox
import scisuit.plot as plt


if __name__ == "__main__":
	try:
		plt.psychrometry()
		plt.show()			
				
	except Exception as e:
		MessageBox(str(e), "Psychrometry Error!")