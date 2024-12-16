import numpy as np
from matplotlib import pyplot as plt

textfile = open("input.txt", 'r',encoding='cp1251')
text=textfile.read()
text = text.split('\n')
data = []
for i in range(len(text)):
    str = text[i].split()
    data.append(int(str[3]))

plt.hist(data, bins=20)
plt.title("Гистограмма времени реакции на событие при нагрузке сети")
plt.xlabel("Время, нс")
plt.ylabel("Количество")
plt.show()

print(f"Mean = {np.mean(data)}\nMax = {max(data)}")
