# Opengl Tutorials

A repo for storing code as I work through the tutorials at [learnopengl.com](https://learnopengl.com). The only interesting aspect is that almost the whole thing is written in plain C.

## Installation

Only a few dependencies are required which are not packaged with this project.
- GLFW: This is most easily obtained using a package manager (eg. `yum install glfw-devel`)
- glad: Visit [the official site](https://glad.dav1d.de/) to generate appropriate source files. Download the zip file, extract, and edit Makefiles as appropriate with the location of your glad install.
- assimp: visit [the official dowload site](https://www.assimp.org/index.php/downloads) to find compressed source files. Installation on Linux is straight forward. Requires cmake and g++ to build, but exposes a C interface. Required for the model loading chapter and beyond (introduction and lighting chapters exclude this requirement). Edit Makefiles in those chapters as appropriate with the header and library locations for assimp.

## Credit to Other Projects

stb.h and linmath.h are included as submodules with this repo. The relevant project pages are [here](https://github.com/nothings/stb) and [here](https://github.com/datenwolf/linmath.h.git).
