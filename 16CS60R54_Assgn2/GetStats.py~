import matplotlib.pyplot as plt; plt.rcdefaults()
import numpy as np
import matplotlib.pyplot as plt1
from matplotlib.colors import ListedColormap

import csv

def WriteIntoCSV(ClientIdsX , NoOfMsgs) :
    f = open("Output.csv", "wb")
    c = csv.writer(f)
    r = []
    r.append("")
    for i in range(0, len(ClientIdsX)) :
        r.append(ClientIdsX[i])
        
    c.writerow(r)
    for i in range(0, len(ClientIdsX)) :
        r = []
        r.append(ClientIdsX[i])
        for j in range(0, len(NoOfMsgs[i])) :
            r.append(NoOfMsgs[i][j])
        c.writerow(r)
        
    f.close()
            
def PlotHeatMap1(ClientIdsX, ClientIdsY, NoOfMsgs) :
    column_labels = ClientIdsX
    row_labels = ClientIdsY
    data = np.array(NoOfMsgs)
    
    fig = plt.figure()
    axis = fig.add_subplot(111)
    
    #fig, axis = plt.subplots() 
    heatmap = axis.pcolor(data, cmap=plt.cm.Blues)

    axis.set_yticks(np.arange(data.shape[0])+0.5, minor=False)
    axis.set_xticks(np.arange(data.shape[1])+0.5, minor=False)

    axis.invert_yaxis()

    axis.set_yticklabels(row_labels, minor=False)
    axis.set_xticklabels(column_labels, minor=False)
    plt.colorbar(heatmap)
    fig.set_size_inches(11.03, 3.5)

    plt.savefig('HeatMap.png', dpi=100)    
    plt.close('all')
    
 
def PlotHistogram(Objects , Performance):
    y_pos = np.arange(len(Objects))
    plt1.bar(y_pos, Performance, align='center', alpha=0.5)
    plt1.xticks(y_pos, Objects)
    plt1.ylabel('Number Of Characters')
    plt1.title('Load of Characters sent on Network')
    #plt.show()
    plt1.savefig("Histogram.png", dpi=100)
    plt1.close('all')
    
#Data.csv has columns{FromId, FromName , ToId, TName, Msg , Time}
with open('Data.csv') as csvfile:
    readCSV = csv.reader(csvfile, delimiter=',')
    ClientIds = []
    
    for row in readCSV:
        client1 = row[0].strip()
        client2 = row[2].strip() 
        if  client1 not in ClientIds:
            ClientIds.append(client1);
            
        if client2 not in ClientIds:
            ClientIds.append(client2);
           
    ClientIds.sort();
    MsgCount = []
    NoOfMsg = []
    for i in range(0 ,len(ClientIds)):
        MsgCount.append(0)
        NoOfMsg.append([])
    for i in range(0, len(ClientIds)):
        for j in range(0, len(ClientIds)):
            NoOfMsg[i].append(0)
                
    csvfile.seek(0);
    for row in readCSV:
        indx = ClientIds.index(row[0].strip())
        MsgCount[indx] = MsgCount[indx] +  len(row[4].strip())
        col = ClientIds.index(row[2].strip())
        NoOfMsg[indx][col] = NoOfMsg[indx][col] + 1
               
    print len(ClientIds)     
    print ClientIds
    print MsgCount
    print NoOfMsg
    
    PlotHistogram(ClientIds, MsgCount)
    PlotHeatMap1(ClientIds, ClientIds, NoOfMsg)
    WriteIntoCSV(ClientIds, NoOfMsg)
       # print(row[0])
       # print(row[0],row[1],row[2],)
