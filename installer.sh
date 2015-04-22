#! /bin/bash

## Install script to set up enviroment and libraries on BBB 
## Run: sh installer.sh
## Approx Install Time ~ 10-20min

PWD="temppwd"

clear

echo "\nSETTING UP AB-BBB ENVIROMENT\n"
echo "\nBEGINNING INSTALLATION\n"


echo "\n------------------------SYSTEM CLEAN----------------------" 
(
	echo $PWD | sudo -S apt-get --assume-yes clean 
) && echo 'System Clean OK' || (echo 'System Clean FAILED' && exit)

echo "\n------------------------SYSTEM UPDATE---------------------" 
(
	echo $PWD | sudo -S apt-get --assume-yes update
) && echo 'System Update OK' || (echo 'System Update  FAILED' && exit)

echo "\n------------------------INSTALL OPENCV--------------------" 
(
	echo $PWD | sudo -S apt-get --assume-yes install libopencv-dev
) && echo 'OpenCV Install OK' || (echo 'OpenCV Install FAILED' && exit)

echo "\n------------------------INSTALL LIBAV---------------------" 
(
	echo $PWD | sudo -S apt-get --assume-yes install libav-tools
) && echo 'OpenCV Install OK' || (echo 'OpenCV Install FAILED' && exit)

echo "\n------------------------CLONE REPO------------------------" 
(
	git clone https://github.com/angelaCTO/AngryBirds-2.0.git
) && echo 'Repo Clone OK' || echo 'Repo Clone FAILED';

mv AngryBirds-2.0 AngryBirds

echo "\nDONE!\n"

exit 0

