qt-ponies
=========

Qt-ponies (pronounced 'cute-ponies') is an implementation of DesktopPonies (http://www.desktopponies.com/) in Qt.
All pony data (images, behaviors, sounds) is taken from DesktopPonies, and is compatible with it.

Inspired by qponies project by svenstaro (https://github.com/svenstaro/qponies).

Compilation
-----------
A compiler supporting C++11 is required, as are Qt libraries (and X11 developement libraries on X11 sytems).

Under Debian/Ubuntu you can install the dependencies by invoking:

    \# sudo apt-get install build-essential libx11-dev libqtcore4 libqtgui4 libqt4-dev qt4-qmake

Then build qt-ponies by invoking:

    \# qmake  
    \# make  

Running under Windows
-------
A testing version of qt-ponies is avialable at [https://github.com/downloads/myszha/qt-ponies/qt-ponies-v0.5-win32.zip](https://github.com/downloads/myszha/qt-ponies/qt-ponies-v0.5-win32.zip).

Running
-------
Run 'qt-ponies'. If you have not added any ponies, the configuration window
will be shown (as is the case at first startup). Configuration is saved
in config.ini in the same directory as the executable.

To open the configuration window, you can double-click on the system tray 
icon or right-click it and choose 'Open configuration'. You can also close
the appliacation from this menu.

Right clicking on a pony brings up its context menu, where you can put that
pony to sleep, remove it, or remove all instances of that pony.


Other information
-----------------
This is a work in progress.
Tested on Linux amd64 3.2, Qt 4.7, g++ 4.6.2.

Due to case-sensitivity of filenames under Unix, all 'pony.ini' files 
must be lower case. The case of .gif files in pony.ini must also be 
correct.

Not all ponies were tested.

Effects, interactions and sounds are not currently supported.

On Unix systems an X server supporting ARGB visuals and and a compositing 
window manager are required for transparency of pony windows. Something like
Compiz, Unity, Gnome Shell, KWin and most newer window managers support it.


Screenshots of the configuration window
---------------------------------------

* Add pony screen
 
![](http://i.imgur.com/7fpXG.png)


* Active ponies screen

![](http://i.imgur.com/rLhjM.png)
