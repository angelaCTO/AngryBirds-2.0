Angry Birds
=========
For more information, please visit our [wiki](https://github.com/UCSD-E4E/AngryBirds/wiki) page! 

Core Contributors
=================
  - Angela To
  - Dustin Mendoza
  - Traci Takasugi
  - Luke DeLuccia

Objectives
==========

Angry Birds is a project which aims to do the following:
  - Detect if a bird has flown into a window
  - Record video footage of the crash
  - Record other data such as speed and/or force
  - Upload data to a database for further examination

Version
-------

1.0 - Starting version of project

Detailed Specifications
-----------------------

Tech
-----------

Angry Birds uses a number of open source projects and tech to work properly:

* [Dillinger] - awesome online markdown editor
* [SublimeText2] - awesome editor with code folding and other good stuff
* [Beagle Bone Black] - Awesome microcontroller
* [Ubuntu] - V14.04 
* [OpenCV] - V2.4 Computer Vision used to store the images

Installation
--------------

On a development computer with Ubuntu installed run:

```sh
sudo apt-get update
git clone https://github.com/UCSD-E4E/AngryBirds.git angrybirds
cd angrybirds
```

If you have not installed openCV run:

```sh
sudo apt-get install libopencv-dev
```

TODO: sample to test if camera and setup working

TODO
====
- [x] Update installation instructions
- [x] Add in images for project
- [x] Purchase piezo electric sensors
- [ ] Add sample 

License
----

MIT


**Free Software, Hell Yeah!**

[Beagle Bone Black]:http://beagleboard.org/Products/BeagleBone+Black
[Dillinger]:http://dillinger.io/
[SublimeText2]:http://www.sublimetext.com/
[OpenCV]:http://opencv.org/
