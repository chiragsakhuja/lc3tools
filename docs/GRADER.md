# Table of Contents

* [Setup](GRADER.md#setup)
* [Running](GRADER.md#running)
  * [Prepare Mode](GRADER.md#prepare-mode)
  * [Grade Mode](GRADER.md#grade-mode)
  * [Upload Mode](GRADER.md#upload-mode)

# Grader Script
The grader script, `tools/grader.py`, simplifies the grading process by: (1)
organizing the students' submissions and downloading student information for
convenience, (2) running a [unit test](TEST.md) in batch for all students or for
an individual student and optionally generating a grade report, and (3)
uploading grades with the test report attached as a comment on each
student's submission.  Additionally, the script provides several options to
facilitate debugging.

The University of Texas at Austin uses the Canvas learning management system, so
the grader script leverages the Canvas API.  To the best of my knowledge, this
script should work with Canvas at other institutes and can be adapted to use
other learning management systems.

# Setup
1. Get a Canvas API key by going to your account settings page on Canvas (i.e.
   `https://<INSTITUTE>.instructure.com/profile/settings`) and then clicking the
   'New Access Token' button.  The API key, which is a large string of
   characters, must remain private, so ensure that it's not accidentally pushed
   to Github or accessible by others elsewhere!
2. Determine the Course ID number by navigating to the course home page on
   Canvas.  The Course ID number will be in the URL (i.e.
   `https://<INSTITUTE>.instructure.com/courses/<COURSE_ID>`).
3. Create a `.env` file in the `tools` directory with the following format:
```
INSTITUTE='<INSTITUTE>'
CANVAS_KEY='<API_KEY>'
COURSE_ID='<COURSE_ID>'
```
4. Install the `python-dotenv` and `canvasapi` Python packages through `pip`.

# Running
The script, located at `tools/grader.py` provides three modes:
1. `prepare`: Extract the submission zip file (usually `submissions.zip` when
   downloaded directly from Canvas), organize the files into directories for
   each student, and create an `INFO.json` file, which contains student
   information such as EID,  in each student's directory.
2. `grade`: Grade one or all of the submissions and optionally generate a grade
   report file.
3. `upload`: Upload grades to Canvas and attach the test report as a comment on
   each student's submission.

The script must be run from the same directory that the `.env` file is saved,
which is recommended to be the `tools` directory.

## Prepare Mode
The purpose of Prepare Mode is to organize the submission zip file, which can be
downloaded directly from the assignment page on Canvas and contains each
student's submissions, into a file structure that is more conducive to grading.

The submission zip file contains a flat structure of `*.asm` or `*.bin` files
with a Canvas-specific naming convention for the files.  Running this mode: (1)
creates a directory for each student based on their Canvas ID number, (2)
renames the submission file to the name provided to the script, and (3) creates
an `INFO.json` file in each student's directory.

Full operation of Prepare Mode is as follows:
```
usage: python grader.py prepare [OPTIONS]

  --file=FILE      [Required] File name of assignment
  --zip=ZIP        [Required] Path to zip file downloaded from Canvas
  --root=DIR       [Default="submissions"] Path to output directory
```

**Important Note:** Canvas marks late submissions by appending `LATE` to the
submission file name.  Prepare Mode does not retain this information, so the
grader script will grade submissions irrespective of their submission time.  It
is up to the instructor to manually enter a 0 in the grade report or on Canvas
if a submission is deemed late.

### Example
Assume that 2 students, Jane Doe and Chirag Sakhuja, have submitted an
assignment to Canvas corresponding to the [sample
assignment](TEST.md#assignment-description) in the [unit test
document](TEST.md).  First, the instructor should go to the assignment page and
download submissions.  We will assume the downloaded file is called
`submissions.zip` and is also stored in the `tools` directory.  Prepare Mode may
be run as follows from the `tools` directory:
```
python grader.py --file=tutorial_sol.asm --zip=submissions.zip --root=tutorial_submissions
```

Extracting `submissions.zip` may result in the following two files:
```
doejane_4768371_64927485_tutorial_sol.asm
sakhujachirag_4769143_64927613_tutorial_sol.asm
```

After running Prepare Mode, the following directory structure will be produced:
```
tutorial_submissions
|- 4768371
   |- INFO.json
   |- tutorial_sol.asm
|- 4769143
   |- INFO.json
   |- tutorial_sol.asm
```
And `INFO.json` for Chirag Sakhuja will contain something similar to:
```
{"name": "Sakhuja, Chirag", "eid": "cs84292", "id": "4769143"}
```

## Grade Mode
**Ensure that the submission directory has been prepared by [Prepare
Mode](GRADER.md#prepare-mode).**

Grade Mode performs grading and is the most commonly used mode in the grader
script.  At a high level, Grade Mode simply automates invocations to the
specified unit test.  However, this mode provides several useful features in the
context of an instructor grading assignments for a class.

Full operation of Grade Mode is as follows:
```
usage: python grader.py grade [OPTIONS]

  --file=FILE      [Required] File name of assignment
  --tester=TESTER  [Required] Path to unit test binary
  --root=DIR       [Default="submissions"] Path to prepared directory
  --eid=EID        [Optional] EID (when grading a single student's assignment)
  --passargs=ARGS  [Optional] Space-delimited string of arguments to pass into unit test
  --dryrun         [Optional] Do not update the grade report file
```

### Grading
Grading can either be performed in batch mode or solo mode (e.g. when looking
into a single student's submission).  In either case, simply provide the
`--file` and `--tester` arguments.  To grade a single student's assignment,
additionally provide the `--eid` argument with the student's EID.

Additionally, Grade Mode supports the `--passargs` argument, which can be used
to pass arguments, which are outlined in the [CLI document](CLI.md#unit-tests),
directly into the unit test.  This may be useful in batch mode for debugging
purposes or to display explicit grader messages (i.e. `--tester-verbose`) for
all students.  This may also be useful in solo mode, such as to get detailed
runtime information with the `--sim-print-level` argument, which would otherwise
require a manual invocation of the unit test.

All output from the unit test is redirected into files in the student's
submission directory.  Standard output is redirected to `<FILE>.out.txt`, and
standard error is redirected to `<FILE>.err.txt`.

### Grade Report
By default, Grade Mode produces tab-separated grade report file called
`REPORT.tsv`, which is stored in the root directory and contains four columns:
full name, EID, Canvas ID, and grade.  When running in batch mode, the entirety
of `REPORT.tsv` is replaced with the results of the run.  When running in solo
mode, only that student's grade is updated in the report.  In either case, to
avoid the grade report from being updated, supply the `--dryrun` argument.

### Example
Continuing from the [example from the previous section](GRADER.md#example), we
now wish to grade the batch of submissions.  To do so, simply run the following
command from `tools`:
```
python grade.py grade --file=tutorial_sol.asm --tester=../build/bin/tutorial --root=tutorial_submissions
```

`REPORT.tsv` may look like the following:
```
Doe, Jane	jd57293	4768371	100.0
Sakhuja, Chirag	cs84292	4769143	0.0
```

Hold on, Chirag wasn't expected to get a 0!  This warrants some closer
examination, so re-run just his submission to get more detailed information.
Since this is just for debugging purposes, there's no need to update the grade
report.  Run the following command:
```
python grade.py grade --file=tutorial_sol.asm --tester=../build/bin/tutorial --root=tutorial_submissions --eid=cs84292 --dryrun --passargs='--sim-print-level=9 --asm-print-level=9'
```

The updated output files,
`tutorial_submissions/4769143/tutorial_sol.asm.out.txt` and
`tutorial_submissions/4769143/tutorial_sol.asm.err.txt` now contain a wealth of
useful information to help with debugging.  See the [debugging
document](DEBUG.md) for tips on how to debug.

Assume that Chirag made a silly mistake and has resubmitted his assignment.
Manually replace `tutorial_sol.asm` with the updated file and then run the
grader script again without the `--dryrun` and `--passargs` arguments.

## Upload Mode
**Ensure that [Grade Mode](GRADER.md#prepare-mode) has completed for the entire
submission directory and that `REPORT.tsv` exists and is accurate.**

The purpose of Upload Mode is to upload grades and test reports (i.e.
`<FILE>.out.txt`) to Canvas.  It is highly recommended that the assignment
grades are muted on Canvas before running Upload Mode.  Similar to Grade Mode,
Upload Mode can update all students' grades or just a single student's grade on
Canvas.  This mode requires an additional argument: the case-sensitive,
whitespace-sensitive name of the assignment on Canvas.

Full operation of Upload Mode is as follows:
```
usage: python grader.py grade [OPTIONS]

  --file=FILE              [Required] File name of assignment
  --assignment=ASSIGNMENT  [Required] Exact name of assignment on Canvas
  --root=DIR               [Default="submissions"] Path to prepared directory
  --eid=EID                [Optional] EID (when updataing a single student's grade)
```

### Example
The final step in the example from [the previous sections](GRADER.md#example-1)
is to upload grades and reports to Canvas.  To do so, run the following command:
```
python grade.py upload --file=tutorial_sol.asm --assignment='Tutorial Assignment' --root=tutorial_submissions
```

If a submission needs to be regraded, then run Grade Mode without `--dryrun` and
then run Upload Mode with `--eid` argument.

# Copyright Notice
Copyright 2020 &copy; McGraw-Hill Education. All rights reserved. No
reproduction or distribution without the prior written consent of McGraw-Hill
Education.
