This is the raytracer I wrote for University of Waterloo's Introduction to Computer Graphics course (CS488). I built on the provided starter code to make it work as part of a normal assignment. Later in the term chose to extend the raytracer with various new features in order to complete the project work component of the course.

### Links

[Some demo images](https://dlem.github.io/raytracer/demo.html)

[A user manual](https://dlem.github.io/raytracer/manual.pdf)

[Some implementation notes](https://dlem.github.io/raytracer/implementation.pdf)

### Prerequisites

GCC, make, lua 5.1 development libraries, libpng development libraries. imagemagick is used for regression testing.

Installation on Ubuntu 14.04: `sudo apt-get install build-essential imagemagick libpng-dev lua5.1-dev`

### Building the Raytacer

```bash
cd src && make
```

### Running the Raytracer

```bash
cd demos
source .setup
../src/rt --help
../src/rt lab.lua
../src/rt lab.lua lab.png
```

### Regression Testing

Do this before making any changes in order to populate the reference directory:

```bash
cd src && make
cd ../test && make reference
make clean
cd ..
```

Then, after making changes:

```bash
cd src && make
cd ../test && make
cd results
```

The results folder will contain 1) a side-by-side comparison of the generated demo images from before and after your changes, with differences highlighted in a third frame, and 2) a diff of the rendering runtimes for each demo. Hopefully, your changes will not have introduced any flaws in the images and will not have introduced any performance regressions.
