#!/bin/bash
cardName=$(sudo fdisk -l 2>/dev/null | grep "63.9 GB" | awk '{print substr($2, 0, length($2))}')
cardPartition=$(sudo fdisk -l 2>/dev/null | grep "$Device Boot" -A 1 | grep "${cardName}" | awk '{print $1}')

sudo mkdir -p /home/ubuntu/AngryBirds/SDCard/
sudo mount "${cardPartition}" /home/ubuntu/AngryBirds/SDCard/ -t auto
