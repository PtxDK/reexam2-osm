# MLFQ


# All Queues
Queues = [[],[],[],[],[]]

# Start and ending of linked lists
Start = [None, None, None, None, None]
End = [None, None, None, None, None]

# 
TimeSlice = 32

# Simulation Job Holder
PreRunList = []


def AddJob(name, arrivalTime, execTime):
    aJobDict = {'name': name, 'arrivalTime': arrivalTime, 'execTimeLeft': execTime, 'execTimeUsed': 0, 'next': None}
    Queues[0].append(aJobDict)

def RunJob(dictElement, timeSlice):
    TimeLeft = int(dictElement['execTimeLeft'])
    if TimeLeft > timeSlice:
        newTimeLeft = TimeLeft-TimeSlice
        dictElement['execTimeLeft': newTimeLeft ]
    elif (TimeLeft < timeSlice & TimeLeft > 0):
        print 'Reached else if'
    else:
        print 'RunJob Error'

        # DO STUFF WITH BREAKING!

def RunQueues():
    # Do Queue1
    for i in range(len(Queues[0])):
        RunJob(Queues[0][i], 1*TimeSlice)

def FileImport():
    with open("mlfq_input.txt", "r") as f:
        for item in f:
            PreRunList.append(item.split())

def ImportJobs():
    for i in range(len(PreRunList)):
        AddJob(PreRunList[i][0], PreRunList[i][1], PreRunList[i][2])


if __name__ == "__main__":

    FileImport()
    ImportJobs()
    RunQueues()



