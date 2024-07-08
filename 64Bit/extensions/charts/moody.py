from wx import MessageBox
import scisuit.plot as plt


if __name__ == "__main__":
	try:
		plt.moody()
		plt.show()			
				
	except Exception as e:
		MessageBox(str(e), "Moody Diagram Error!")