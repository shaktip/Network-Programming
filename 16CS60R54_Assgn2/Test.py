import matplotlib.pyplot as plt
import numpy as np

column_labels = list(range(0,24))
row_labels = ["Lundi",
              "Mardi",
              "Mercredi",
              "Jeudi",
              "Vendredi",
              "Samedi",
              "Dimanche"]
data = np.array([
                [0,0,0,0,0,0,0,0,0,0,434,560,650,340,980,880,434,434,0,0,0,0,0,0],
                [0,0,0,0,0,0,0,434,560,0,650,0,0,0,0,340,980,0,0,0,880,0,434,343],
                [0,0,0,0,0,0,0,0,0,0,434,560,650,340,980,880,434,434,0,0,0,0,0,0],
                [0,0,0,0,0,0,0,434,560,0,650,0,0,0,0,340,980,0,0,0,880,0,434,343],
                [0,0,0,0,0,0,0,0,0,0,434,560,650,340,980,880,434,434,0,0,0,0,0,0],
                [0,0,0,0,0,0,0,434,560,0,650,0,0,0,0,340,980,0,0,0,880,0,434,343],
                [0,0,0,0,0,0,0,0,0,0,434,560,650,340,980,880,434,434,0,0,0,0,0,0]
                ])
fig, axis = plt.subplots() # il me semble que c'est une bonne habitude de faire supbplots
heatmap = axis.pcolor(data, cmap=plt.cm.Blues) # heatmap contient les valeurs

axis.set_yticks(np.arange(data.shape[0])+0.5, minor=False)
axis.set_xticks(np.arange(data.shape[1])+0.5, minor=False)

axis.invert_yaxis()

axis.set_yticklabels(row_labels, minor=False)
axis.set_xticklabels(column_labels, minor=False)

fig.set_size_inches(11.03, 3.5)

plt.savefig('test.png', dpi=100)
