#!/usr/bin/env zsh

export_pd () {
    python3 ./export_player.py \
            --ram ram/$1.raw \
            --landingX $2 \
            --landingY $3 \
            --platformX $4 \
            >playerdats/$1.dat
}

export_pd_int () {
    python3 ./export_player.py \
        --ram ram/$1.raw \
        --landingX $2 \
        --landingY $3 \
        --platformX $4 \
        --interferenceX1 $5 \
        --interferenceX2 $6 \
        --interferenceY $7 \
        >playerdats/$1.dat
} 

# Prologue
export_pd tyd_east_room_no_yellow_block 475 110 28.9
export_pd tyd_east_room -375 85 28.9
export_pd tyd_east_room_tas -475 110 125 # Prevents above-panel physics from occurring
export_pd blooper 525 10 296.15

# Chapter 1
#export_pd outside_hooktail # We skip this in the tas... should we include?
#export_pd inside_hooktail # We skip this in the tas... should we include?

# Chapter 2
export_pd flurrie_unraised 490 110 -253.89
export_pd flurrie_raised 490 110 -253.89

# Chapter 3
export_pd glitzville -345 100 563.2

# Chapter 8
export_pd gloomtail 685 -15 -503.8
export_pd grodus 600 -415 -650 # Prevents above-panel physics from occurring
export_pd gloomtail_after_bug 685 -15 -503.8

export_pd_int plane_game 1700 1000 -1302.9 -1400 1800 0
export_pd plane_game_left -1302.9 1000 -1302.9
