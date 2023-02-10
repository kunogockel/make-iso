# make-iso

Creates an ISO image from CD

Syntax:

	make-iso filename [path]

`filename` is the desired name of the iso file without extension, 
".iso" is automatically appended

`path` is an optional path to store the iso file into 
(I wrote the original version of the program some years ago an I do not know anymore, why I
designed the path specification as a separate parameter)

`make-iso` is a simple console program for Windows written in C which creates an iso image
from a CD. It was intended to archive physical CD ROM disks as a file to make them available
also on Computers without CDROM drive.

The program has no dependencies and only uses the Windows API to create the ISO image.

Basically the program works similar to the `dd` utility on Linux. It copies the sectors from the
CD and saves them into a file. Windows can access devices by specifying the drive letter of
the device in the form `\\.\d:`, where `d:` is the drive specifier. The device can be accessed
by the `CreateFile` and `ReadFile` functions. 

The program uses the `GetDriveType` function to detect the first available CDROM drive, opens it
and reads the data from the device in blocks of 64KB.


