qt-ponies
=========

Qt-ponies (pronounced 'cute-ponies') is an implementation of DesktopPonies (http://desktopponies.bugs3.com/) in Qt.
All pony data (images, behaviors, sounds) is taken from DesktopPonies, and is compatible with it.

Inspired by qponies project by svenstaro (https://github.com/svenstaro/qponies).

Installation
------------
Compile the program or download a precompiled binary from [https://github.com/myszha/qt-ponies/downloads](https://github.com/myszha/qt-ponies/downloads).

Under Debian-based, you must download and install qt-ponies\_$version\_$arch.deb and qt-ponies-data\_$version\_all.deb .

Under Windows, download and unpack qt-ponies-$version-win32.zip .

Running
-------
Run 'qt-ponies'. If you have not added any ponies, the configuration window
will be shown (as is the case at first startup).

To open the configuration window, you can double-click on the system tray 
icon or right-click it and choose 'Open configuration'. You can also close
the appliacation from this menu.

Right clicking on a pony brings up its context menu, where you can put that
pony to sleep, remove it, or remove all instances of that pony.

Running under Windows
-------
A precompiled binary of qt-ponies is avialable in the downloads section.

Compilation
-----------
A compiler supporting C++11 (g++ >= 4.6, clang >= 3.1) is required, as are Qt (version = 4.8) libraries (and X11 developement libraries on X11 sytems).

Under Debian/Ubuntu you can install the dependencies by invoking:

    # sudo apt-get install build-essential libx11-dev libxfixes-dev libqtcore4 libqtgui4 libqt4-dev qt4-qmake

Then build qt-ponies by invoking:

    # git clone https://github.com/myszha/qt-ponies/
    # cd qt-ponies
    # qmake  
    # make  

**Or** you can use a precompiled Debian/Ubuntu package for i386 and amd64, available in downloads.

Other information
-----------------
This is a work in progress.
Tested on Linux amd64 3.2, Qt 4.7, g++ 4.6.2.

Due to case-sensitivity of filenames under Unix, all 'pony.ini' files 
must be lower case. The case of .gif files in pony.ini must also be 
correct.

On Unix systems an X server supporting ARGB visuals and and a compositing 
window manager are required for transparency of pony windows. Something like
Compiz, Unity, Gnome Shell, KWin and most newer window managers support it.

**Configuration**

The configuration file is kept in:

On Unix:

    $HOME/.config/qt-ponies/qt-ponies.ini

On Windows:

    %APPDATA%\qt-ponies\qt-ponies.ini


Screenshots of the configuration window
---------------------------------------

* Add pony screen
 
![](http://i.imgur.com/cObuc.png)


* Active ponies screen

![](http://i.imgur.com/rLhjM.png)
