
examNumber = 42
maxSize = examNumber*64 # kb

# fileBlock[(Content, size, location)]
fileBlocks = [(True, maxSize, 0)] # Make sure free blocks also exist
for i in range(maxSize-1):
    fileBlocks.append((None, 0, i+1))

def malloc(allocSize):
    # Loop through all elements in fileblocks
    for block in fileBlocks:
        # When block of big enough size is found and is also free
        if allocSize < block[1] & block[0] == None:
            # Check if block is big enough for splitting
            currentBlockSize
            halfBlock = block[1]+1 / 2
            currentLocation = block[2]
            if allocSize > halfBlock:
                # find block in fileBlocks and replace with one block, and add another block just after that block
                fileBlocks[currentLocation] = (True, halfBlock, block[2])
                fileBlocks[currentLocation+halfBlock] = (True, halfBlock, block[2]+halfBlock)

            # If block can be splitted, check if possible to split again ... again

def split():
    print '..'

malloc(1344)

print fileBlocks
print len(fileBlocks)

