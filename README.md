lc3sim
lc3sim is a complete overhaul of the current LC-3 simulator used in EE 306 at The University of Texas at Austin.
With lc3sim, we strive to achieve the following goals:
* Create a truly cross platform assembler and simulator
* Design an interface that unifies editing, assembling, and simulating code
* Develop a formal testing infrastructure that can be used with equal ease by both students and graders
* Integrate a submission system with the interface to encourage frequent submissions and/or version control
* Maintain an organized, easily extendable, and open source code base

### Cross platform
The two major problems with the existing simulator are that the code base has fragmented on Windows and Linux such that different functionality is supported, and OS X has no official support. The goal of lc3sim to be truly cross platform implies that not only will complete functionality be supported on all major platforms, but also that the code should be compileable on these platforms.
