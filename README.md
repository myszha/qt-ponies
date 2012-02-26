qt-ponies
=========

Qt-ponies (pronounced 'cute-ponies') is an implementation of desktop-ponies (http://www.desktopponies.com/) in Qt.

Inspired by qponies project by svenstaro (https://github.com/svenstaro/qponies).

Compilation
-----------
A compiler supporting C++11 is required, as are Qt libraries.

\# qmake  
\# make  
\# ./ponies


Running
-------
Run 'ponies'. If you have not added any ponies, the configuration window
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
Pony behavior following/moving to a point on the screen is not currently
supported.


Screenshots of the configuration window
---------------------------------------

* Add pony screen
 
![](http://i.imgur.com/7fpXG.png)


* Active ponies screen

![](http://i.imgur.com/rLhjM.png)
