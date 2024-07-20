from _sci import activeworksheet, messagebox



if __name__=="__main__":
	try:
		ws = activeworksheet()
		rng = ws.selection()

		assert rng != None, "A single column selection must be made"
		assert rng.ncols()==1, "Only a single column can be selected."

		lst = rng.tolist()
		lst = [e for e in lst if isinstance(e, str|float|int)]

		N0 = len(lst)
		assert N0>=2, "At least 2 valid entries expected."

		TL, BR = rng.coords()
		R, C = TL

		dct = dict.fromkeys(lst)
		N1 = len(dct)

		rng.clear()

		i=0
		for k, _ in dct.items():
			ws[R+i, C] = str(k)
			i += 1

		messagebox(f"Removed {N0-N1} items.", "Success")
		
	except Exception as e:
		messagebox(str(e), "Duplicate Removal Error!")