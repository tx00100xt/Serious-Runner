## Serious Runner
[![Build status](https://github.com/tx00100xt/Serious-Runner/actions/workflows/cibuild.yml/badge.svg)](https://github.com/tx00100xt/Serious-Runner/actions/)
[![License: GPL v2](https://img.shields.io/badge/License-GPL_v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/tx00100xt/Serious-Runner)](https://github.com/tx00100xt/Serious-Runner/releases/tag/v1.0)
[![Github downloads](https://img.shields.io/github/downloads/tx00100xt/Serious-Runner/total.svg?logo=github&logoColor=white&style=flat-square&color=E75776)](https://github.com/tx00100xt/Serious-Runner/releases/)

Serious Runner а small program that loads and runs content created by users for the game Serious Sam Classic.

History of creation
-------------------
As you know, over the years since the game's release, users have created mods, additional maps, and map packs for Serious Sam. The game interface allows you to launch additional maps and modifications, but this is not implemented very conveniently. Maps and mods may contain Cyrillic names, which leads to incorrect display of characters, unless support for the corresponding language is installed in the game. In addition, users do not know where to get additional content for the game. I decided to write a program to simplify this process. More than 600 maps created by enthusiasts for single play were reviewed. About half of them were selected and posted on the Web Archive, and all information about them was entered into the SQL database. The goal was to select the best cards with beautiful design and interesting gameplay.

Serious Runner allows you to download and run additional maps and mods created by enthusiasts. You can also add your own or other maps using the release form on github. After verification, the cards will be added to the SQL database.

How the program works
---------------------
When launched for the first time, the program checks for the presence of the database along the path:
```
~/.local/share/Serious-Runner/DB/seriousrunner.db
```
If the database is not installed, it will be searched in the home directory (or /usr/share/seriousrunner/DB depending on the installation type). Once the SQL database is found, it will be placed in the above path.

Next, the program will notify you about the need to install the program and install the game data, and the main program window will open.

The first thing to do is install the game. After clicking the Install Games button, the program will first download the latest release of the game from GitHub, unpack it into the /tmp directory and copy the contents along the path:
```
~/.local/share/Serious-Runner
```
Then, the program will download the settings and configuration files and place them there.
As a result, we will have a local copy of the games located at the following paths:
```
~/.local/share/Serious-Runner/SamTFE
~/.local/share/Serious-Runner/SamTSE
```
The second stage of installation is installing the game data to the above local paths.
To do this, the game data must be unpacked in your home directory. If you purchased the game on Steam, then there is no need to place game data.
After clicking the button Search and install game data, the program will find it and copy it to its local copy of the game, according to the paths indicated above. This completes the installation procedure.

Dependencies
------------

* `libarchive, liblzma, libQt5Gui, libQt5Core, libQt5Widgets, libQt5Sql, libQt5Network`

Building (Linux only)
---------------------
Type this in your terminal:
```
git clone https://github.com/tx00100xt/Serious-Runner.git
cd Serious-Runner
mkdir build && cd build
cmake .
make -j4
make install
```

Building and System install (Linux only)
----------------------------------------
```
git clone https://github.com/tx00100xt/Serious-Runner.git
cd Serious-Runner
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/usr
make -j4
su
make install
exit
```

Screenshots of the program
--------------------------
![Serious Runner png1](https://raw.githubusercontent.com/tx00100xt/Serious-Runner/main/Picturies/1.png)

![Serious Runner png2](https://raw.githubusercontent.com/tx00100xt/Serious-Runner/main/Picturies/2.png)

![Serious Runner png3](https://raw.githubusercontent.com/tx00100xt/Serious-Runner/main/Picturies/3.png)

Supported Architectures
----------------------
* `x86-64`

Supported OS
-----------
* `Linux (glibc)`
* `Alpine Linux (musl)`
* `FreeBSD`

Plans
-----
* `There are no plans.`

### Build status
|CI|Platform|Compiler|Configurations|Platforms|Status|
|---|---|---|---|---|---|
|GitHub Actions|Ubuntu, Alpine, FreeBSD|GCC, Clang|Release| x64|![GitHub Actions Build Status](https://github.com/tx00100xt/Serious-Runner/actions/workflows/cibuild.yml/badge.svg)

You can download a the automatically build based on the latest commit.  
To do this, go to the [Actions tab], select the top workflows, and then Artifacts.

License
-------
* `Serious Runner licensed under the GNU GPL v2 (see LICENSE file).`

[Actions tab]: https://github.com/tx00100xt/Serious-Runner/actions "Download Artifacts"

