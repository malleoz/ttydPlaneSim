#!/usr/bin/env zsh

export_pd () {
    python3 ./export_player.py \
            --ram rams/$1.ram \
            --landingX $2 \
            --landingY $3 \
            >playerdats/$1.dat
}

export_pd tyd_east_room -468 110
export_pd flurrie 482 100
export_pd gloomtail 681 -15
export_pd grodus 595 -390
export_pd blooper 515 25
