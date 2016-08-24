# MLFQ Algorithm


# Check for new input

def JobAdd(name, arrivaltime, execTime):
    aJobDict = {'name' : name, 'arrivalTime': arrivaltime, 'execTimeLeft': execTime, 'execTimeUsed': 0, 'queue': 1}
    Queue1.add(aJobDict)

def QueueDec(name, queue):
    if name.queue < 5 and name.queue > 0 :

        exec("%s = %d" % ('Queue' + str(name.queue), listelement))

        name.queue += 1
    else:
        print 'Error: Queue out of bounds'

# Big While loop which runs everything
def RunJobs():
    ticks = 0
    while ticks < 1000:
        RunQueue1(Queue1)
        RunQueue2(Queue2)
        RunQueue1(Queue1)
        RunQueue2(Queue2)
        RunQueue3(Queue3)
        RunQueue2(Queue2)
        RunQueue1(Queue1)
        RunQueue2(Queue2)
        RunQueue3(Queue3)
        RunQueue4(Queue4)
        RunQueue3(Queue3)
        RunQueue2(Queue2)
        RunQueue1(Queue1)
        RunQueue2(Queue2)
        RunQueue3(Queue3)
        RunQueue4(Queue4)
        RunQueue5(Queue5)
        RunQueue4(Queue4)
        RunQueue3(Queue3)
        RunQueue2(Queue2)
    QueueReset()

# Moves all Jobs to Queue 1 and thereby sets it to top priority
def QueueReset():
    for i in Queue2:
        Queue1.add(Queue2[i])
    Queue2 = []

    for i in Queue3:
        Queue1.add(Queue3[i])
    Queue3 = []

    for i in Queue4:
        Queue1.add(Queue4[i])
    Queue4 = []

    for i in Queue5:
        Queue1.add(Queue5[i])
    Queue5 = []


def RunQueue1(list):        # Round Robin is not supported
    i = len(list)
    while i > 0:
        list[i.execTimeLeft-1]
        list[i.execTimeUsed+1]
        # A Check for if TimeLeft = 0 should be made and run remove from queue if = 0
        ticks += 1
        i -= 1


def RunQueue2(list):        # Round Robin is not supported
    i = len(list)
    while i > 0:
        # A Check for number of runs left should be made
        list[i.execTimeLeft-2]
        list[i.execTimeUsed+2]
        ticks += 2
        i -= 1

def RunQueue3(list):        # Round Robin is not supported
    i = len(list)
    while i > 0:
        # A Check for number of runs left should be made
        list[i.execTimeLeft-4]
        list[i.execTimeUsed+4]
        ticks += 4
        i -= 1

def RunQueue4(list):        # Round Robin is not supported
    i = len(list)
    while i > 0:
        # A Check for number of runs left should be made
        list[i.execTimeLeft-8]
        list[i.execTimeUsed+8]
        ticks += 2
        i -= 1

def RunQueue5(list):        # Round Robin is not supported
    i = len(list)
    while i > 0:
        # A Check for number of runs left should be made
        list[i.execTimeLeft-16]
        list[i.execTimeUsed+16]
        ticks += 2
        i -= 1


if __name__ == '__main__':
    Queue1 = []
    Queue2 = []
    Queue3 = []
    Queue4 = []
    Queue5 = []
    #JobAdd('a',1,2)


'''
    while True:
        RunJobs()
'''



