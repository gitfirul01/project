import pandas as pd
import matplotlib.pyplot as plt

file_name = 'export_dataframe'
read_path = './{}.csv'.format(file_name)

df = pd.read_csv(read_path)

print(df)

y = df.iloc[1:, 1].values
x = list(range(1, len(y)+1))
print(y)
print(x)

plt.plot(x,y)
plt.show()