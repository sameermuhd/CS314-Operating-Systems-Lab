import random
import csv

asize = 1024
psize = 16 * 1024
uList = [0] * psize

for i in range(100):
    random.seed(i)
    for j in range(psize):
        limit = j
        vaddr = int(asize * random.random())
        if (vaddr < limit):
            uList[j] += 1
            
uList = [x / 100 for x in uList]
rows = list()
for i in range(psize):
	newList = list()
	newList.append(i)
	newList.append(uList[i])
	rows.append(newList)

fields = ["Limit", "Fraction of Valid Addresses"]

with open('plot_output.csv', 'w') as f:
      
    write = csv.writer(f)
    write.writerow(fields)
    write.writerows(rows)
