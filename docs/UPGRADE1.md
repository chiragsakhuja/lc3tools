# Upgrading unit tests and frontends from API_VER 1 to 2
Release v2.0.0 of LC3Tools introduces a similar, but new, API version that is no
longer backward compatible with version 1 of the API.  Since version 1 is now
deprecated, this guide helps you transition to version 2.  The biggest changes
in the documented API have just been in some renamed functions, but behavior is
mostly the same, so the transition shouldn't be too difficult.

## Who is this for?
I'm gearing this guide toward instructors who intend on carrying over old
graders (now referred to as unit tests) to future semesters.  For example, this
may be because you are using the same assignment between semesters.  LC3Tools
does provide a compatibility layer for API_VER 1, which supports the
[documented API](API1.md), but API_VER 2 is the recommended API now. Also, note
that undocumented code may have changed and is not supported by the
compatibility layer.

This change may also impact anyone who has developed their own frontend.
Release v2.0.0 introduces a completely new simulator backend as well as some
minor bugfixes that are not in v1.0.6.  The new backend is functionality
identical to the previous backend, but it more stable and is fully
deterministic.  If these changes are important, please transition to API_VER 2.

## New Directories
The `backend` and `frontend` categorization no longer applies, and everything is
in a single `src` directory now.  Unit tests (formerly referred to as graders)
are now in `src/test/tests`.  Otherwise, behavior is the same---you run CMake to
detect new unit test sources files that are built alongside the other command
line utilities.

## API_VER Define
**To use API_VER 2, you must define it in your `.cpp` file before including any
LC3Tools header files. Add the following line to the top of your unit test.**

```
#define API_VER 2
#include "framework.h"
...
```
