
import numpy as np

def correct(_data: list, tolerance: float = 0.25):
	
	data = _data.copy()

	if len(data) == 0:
		return []
	if tolerance < 0:
		raise Exception("a")

	mean = sum(data) / len(data)
	max_absolute_difference = tolerance * mean

	data.sort(reverse=False)
	while data[len(data) - 1]  >= mean and abs(data[len(data) - 1] - mean) > max_absolute_difference:
		data.pop(len(data) - 1)

	data.reverse()
	while len(data) != 0 and data[len(data) - 1]  <= mean and abs(data[len(data) - 1] - mean) > max_absolute_difference:
		data.pop(len(data) - 1)

	return data

x1 = [99, 98, 95, 60, 101, 102, 100, 98, 0]
print(correct(x1))
