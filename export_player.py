import os
import sys
import flags
import bindatastore as bin

FLAGS = flags.Flags()
FLAGS.DefineString("in", "ram.raw")        # Filepath to Dolphin RAM dump
#FLAGS.DefineString("region", "U")   # Game version (U or J)

getMarioPtr = { "U": 0x8041e900}
posOffset = 0x8C
motOffset = 0x294

def main(argc, argv):
    dat = bin.BDStore(big_endian=True)
    dat.RegisterFile(FLAGS.GetFlag("in"), offset=0x80000000)
    getMarioPtrStore = dat.at(getMarioPtr["U"])
    marioPtr = getMarioPtrStore.rptr()
    marioPtrStore = dat.at(marioPtr)


    xPos = marioPtrStore.rfloat(posOffset)
    yPos = marioPtrStore.rfloat(posOffset + 4)
    zPos = marioPtrStore.rfloat(posOffset + 8)
    
    motPtr = marioPtrStore.rptr(motOffset)
    motPtrStore = dat.at(motPtr)

    motFlags = motPtrStore.rbytes(4, 0)
    motFlags = int.from_bytes(motFlags, "big")

    motXRot   = motPtrStore.rfloat(0x4)
    motYRot   = motPtrStore.rfloat(0x8)
    motZRot   = motPtrStore.rfloat(0xC)
    motIndex4 = motPtrStore.rfloat(0x10)
    motIndex5 = motPtrStore.rfloat(0x14)
    motIndex6 = motPtrStore.rfloat(0x18)
    motIndex7 = motPtrStore.rfloat(0x1C)
    motIndex8 = motPtrStore.rfloat(0x20)
    motIndex9 = motPtrStore.rfloat(0x24)

    ######################################
    #         LANDING PARAMETERS         #
    ######################################
    # Specify the X and Y position bound #
    # required in order to define what's #
    # considered a valid plane flight.   #
    #Values for the first plane panel, east of the thousand year door room. 
    #landingX = -467.7
    #landingY = 110.0
    #Flurrie's plane panel.
    #landingX = 482.000
    #landingY = 100.000
    #Gloomtail's door - the ultimate challenge! 
    #landingX = 681.000
    #landingY = -15
    #Grodus platform
    #landingX = 595
    #landingY = -390
    #Room heading to blooper.
    landingX=515.0
    landingY=25.0    

    # Export to stdout

    sys.stdout.write("%.9g " % xPos)
    sys.stdout.write("%.9g " % yPos)
    sys.stdout.write("%.9g " % zPos)
    sys.stdout.write("%d " % motFlags)
    sys.stdout.write("%.9g " % motXRot)
    sys.stdout.write("%.9g " % motYRot)
    sys.stdout.write("%.9g " % motZRot)
    sys.stdout.write("%.9g " % motIndex4)
    sys.stdout.write("%.9g " % motIndex5)
    sys.stdout.write("%.9g " % motIndex6)
    sys.stdout.write("%.9g " % motIndex7)
    sys.stdout.write("%.9g " % motIndex8)
    sys.stdout.write("%.9g " % motIndex9)
    sys.stdout.write("%.9g " % landingX)
    sys.stdout.write("%.9g " % landingY)


if __name__ == "__main__":
    (argc, argv) = FLAGS.ParseFlags(sys.argv[1:])
    main(argc, argv)
