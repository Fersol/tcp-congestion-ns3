import pandas as pd
import matplotlib.pyplot as plt

filename = "cwnd.tr"
df = pd.read_csv(filename, sep=" ", header=None)
plt.plot(df[0], df[1])
plt.savefig('cwnd.png')


