import os
import sys
import flags
import bindatastore as bin

# python3 export_player.py --landingX 300 --landingY 100 --platformX --ram ram/flurrie.raw [--interferenceX1 0 --interference X2 50 --interferenceY -100]

FLAGS = flags.Flags()
FLAGS.DefineString("ram", "ram.raw")        # Filepath to Dolphin RAM dump
FLAGS.DefineFloat("landingX")               # Specify the X boundary of the plane panel (when physics change)
FLAGS.DefineFloat("landingY")
FLAGS.DefineFloat("platformX")
FLAGS.DefineFloat("interferenceX1")
FLAGS.DefineFloat("interferenceX2")
FLAGS.DefineFloat("interferenceY")
#FLAGS.DefineString("region", "U")   # Game version (U or J)

getMarioPtr = { "U": 0x8041e900}
posOffset = 0x8C
baseSpeedOffset = 0x180
motOffset = 0x294

def main(argc, argv):
    dat = bin.BDStore(big_endian=True)
    dat.RegisterFile(FLAGS.GetFlag("ram"), offset=0x80000000)
    getMarioPtrStore = dat.at(getMarioPtr["U"])
    marioPtr = getMarioPtrStore.rptr()
    marioPtrStore = dat.at(marioPtr)


    xPos = marioPtrStore.rfloat(posOffset)
    yPos = marioPtrStore.rfloat(posOffset + 4)
    zPos = marioPtrStore.rfloat(posOffset + 8)
    
    baseSpeed = marioPtrStore.rfloat(baseSpeedOffset)
    
    motPtr = marioPtrStore.rptr(motOffset)
    motPtrStore = dat.at(motPtr)

    motFlags = motPtrStore.rbytes(4, 0)
    motFlags = int.from_bytes(motFlags, "big")

    motXRot   = motPtrStore.rfloat(0x4)
    motYRot   = motPtrStore.rfloat(0x8)
    motZRot   = motPtrStore.rfloat(0xC)
    motIndex4 = motPtrStore.rfloat(0x10)
    motIndex5 = motPtrStore.rfloat(0x14)
    ySpeed    = motPtrStore.rfloat(0x18)
    motIndex7 = motPtrStore.rfloat(0x1C)
    # timeout field ommitted
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
    #landingX=515.0
    #landingY=25.0    
    landingX = FLAGS.GetFlag("landingX")
    landingY = FLAGS.GetFlag("landingY")
    if landingX is None or landingY is None:
        print("A proper landing position was not provided.")

    platformX = FLAGS.GetFlag("platformX")
    if platformX is None:
        print("A proper x-axis boundary for the plane panel was not provided.")

    # Export to stdout

    sys.stdout.write("%.9g " % xPos)
    sys.stdout.write("%.9g " % yPos)
    sys.stdout.write("%.9g " % zPos)
    sys.stdout.write("%.9g " % baseSpeed)
    sys.stdout.write("%d " % motFlags)
    sys.stdout.write("%.9g " % motXRot)
    sys.stdout.write("%.9g " % motYRot)
    sys.stdout.write("%.9g " % motZRot)
    sys.stdout.write("%.9g " % motIndex4)
    sys.stdout.write("%.9g " % motIndex5)
    sys.stdout.write("%.9g " % ySpeed)
    sys.stdout.write("%.9g " % motIndex7)
    sys.stdout.write("%.9g " % motIndex9)
    sys.stdout.write("%.9g " % landingX)
    sys.stdout.write("%.9g " % landingY)
    sys.stdout.write("%.9g " % platformX)

    interferenceX1 = FLAGS.GetFlag("interferenceX1")
    interferenceX2 = FLAGS.GetFlag("interferenceX2")
    interferenceY = FLAGS.GetFlag("interferenceY")

    if not None in [interferenceX1, interferenceX2, interferenceY]:
        # Assert x1 < x2
        if interferenceX1 < interferenceX2:
            sys.stdout.write("%.9g " % interferenceX1)
            sys.stdout.write("%.9g " % interferenceX2)
        else:
            sys.stdout.write("%.9g " % interferenceX2)
            sys.stdout.write("%.9g " % interferenceX1)
        sys.stdout.write("%.9g " % interferenceY)


if __name__ == "__main__":
    (argc, argv) = FLAGS.ParseFlags(sys.argv[1:])
    main(argc, argv)
