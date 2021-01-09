# Version v2.0.0
The first major version change of LC3Tools since launch!  This change breaks
backward compatibility for graders (now called unit tests).  Please read the
[upgrade guide](UPGRADE1.md) to transition existing graders to the new API.

* Complete rewrite of the simulator backend to an event-driven simulation
* Fully deterministic, including keyboard interrupts
* Explicit state changes to ensure consistency
* Significantly improved command line debugging printouts
* Several bugfixes and stability improvements
* New backend API
* Improved unit testing API that includes quality-of-life updates after a year
  of experience in the real world

# Copyright Notice
Copyright 2020 &copy; McGraw-Hill Education. All rights reserved. No
reproduction or distribution without the prior written consent of McGraw-Hill
Education.
