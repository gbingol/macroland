import pandas as pd
from _sci import Worksheet

def _getHeadersIndexesValues(df:pd.DataFrame)->tuple:
	"""
	returns headers, indexes and values, each as list
	"""
	Headers = list(df)
	Values = df.values.tolist()
	Indexes = []
	for row in df.index:
		Indexes.append(row)
	
	return Headers, Indexes, Values



def print_to_ws(df:pd.DataFrame, WS:Worksheet, Row:int, Col:int, headers=True, indexes=True):
	"""
	prints a DataFrame to a given worksheet starting from Row and Col \n
	headers = True, prints headers \n
	indexes = True, prints indexes
	"""
	Headers, Indexes, Values = _getHeadersIndexesValues(df)

	r, c= Row, Col
	i, j = 0, 0 #row, col index

	if indexes: 
		i = 1 if headers else 0
		for index in Indexes:
			WS[r + i, c] = {"value":index, "style":"italic"}
			i += 1

	if headers:
		j = 1 if indexes else 0 #if indexes printed then start from 1st col, otherwise 0th
		for header in Headers:
			WS[r, c + j] = {"value":header, "weight":"bold"}
			j += 1
		
	
	i, j = 0, 0 #reset index counters
	if headers:
		r += 1 #1 row below (0th row has headers)
	
	if indexes:
		c += 1 #1 col right (0th col has indexes)

	for lst in Values:
		for v in lst:
			WS[r + i, c + j] = v
			j += 1
		
		i += 1
		j = 0
