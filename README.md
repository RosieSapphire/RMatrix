# RMatrix
This is simply my own version of the classic [cmatrix](https://github.com/abishekvashok/cmatrix) program. It is written from scratch using C89 and my custom [Rose Petal API](https://github.com/RosieSapphire/RosePetal) to test it out.
## Installation
First of all, make sure you have `git`, `gcc` and `make` installed on your computer. This will be easiest to do with Linux, but you can use `WSL` to compile it, and, if you want, make your own build system using `Visual Studio`.

Once you've got all the prerequisites, clone the repository by going into the folder you'd like to clone it into and typing

```git clone --recurse-submodules https://github.com/RosieSapphire/RMatrix.git rmatrix```

Once it's in, `cd` into the directory and run `make -j`. This will build the executable, and the Rose Petal library included as a submodule.

From here, just run `./rmatrix` and you'll be greeted to the raining terminal text familiar to all cmatrix-like applications. :D
