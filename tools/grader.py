# 
# Copyright 2020 McGraw-Hill Education. All rights reserved. No reproduction or distribution without the prior written consent of McGraw-Hill Education.
# 
import argparse
import json
import os
import subprocess
import sys
import zipfile

from dotenv import load_dotenv
from canvasapi import Canvas

# Download student profile from Canvas.
def getUserInfoFromID(id):
    profile = course.get_user(id).get_profile()
    return { 'name': profile['sortable_name'], 'eid': profile['login_id'], 'id': str(id) }

# Get assignment ID based on the assignment's name on Canvas.
def getAssignmentFromName(assignment_name):
    for assignment in course.get_assignments():
        if assignment_name in str(assignment):
            return assignment
    return None

# Fuzzy match with filenames to avoid some common submission typos.
def doesNameFuzzyMatch(real, expected):
    expected_parts = expected.lower().split('.')
    real_parts = real.lower().split('.')
    return expected_parts[0] in real_parts[0] and expected_parts[-1] == real_parts[-1]

# Extract the raw grade from the grade report.
def getGradeFromGraderOutput(grader_output):
    lines = [x.rstrip() for x in grader_output.split('\n')]
    if len(lines) < 2: return 0
    if not 'successful' in lines[1]: return 0
    return float(lines[-2].split(' ')[3].split('/')[0])

def prepare(submission_zip, submission_root, file_name):
    # Create the destination directory if it doesn't exist.
    if not os.path.isdir(submission_root):
        os.mkdir(submission_root)

    # Extract the zip file
    with zipfile.ZipFile(submission_zip, 'r') as file:
        file.extractall(submission_root)

    # Iterate over the students.
    for submission in sorted(os.listdir(submission_root)):
        submission_path = os.path.join(submission_root, submission)
        if not os.path.isfile(submission_path): continue

        print('Preparing', submission)
        # Ignore if the submission was marked as late on Canvas.  We provide an
        # anonymous amount of grace time after the submission deadline, so we
        # must manually confirm the submission time on Canvas and put a 0 if it
        # exceeded even the grace period.
        submission_parts = submission.split('_')
        user_id = submission_parts[1]
        if user_id == 'LATE':
            user_id = submission_parts[2]
        user_info = getUserInfoFromID(user_id)

        # Create the directory for the student, based on the student's Canvas
        # ID.
        user_id_path = os.path.join(submission_root, user_id)
        if not os.path.isdir(user_id_path):
            os.mkdir(user_id_path)

        # Save the student's profile in INFO.json for easy reference.
        with open(os.path.join(user_id_path, 'INFO.json'), 'w') as file:
            file.write(json.dumps(user_info))

        # Students sometimes add capitalization or submit with the incorrect
        # file name.  This catches a couple of common cases, but it's best to
        # inspect files manually if the grade is unexpected.
        dest_file_name = submission
        if doesNameFuzzyMatch(submission, file_name):
            dest_file_name = file_name
        os.rename(submission_path, os.path.join(user_id_path, dest_file_name))

def grade(submission_root, file_name, grader_path, eid, dryrun, passargs):
    # Build up the grade report in memory before writing it to a file.
    report_lines = list()

    # By default, iterate over all the submissions in the prepared directory.
    for submission in sorted(os.listdir(submission_root)):
        submission_path = os.path.join(submission_root, submission)
        if not os.path.isdir(submission_path): continue

        with open(os.path.join(submission_path, 'INFO.json')) as file:
            user_info = json.loads(file.read())

        # If the grader is running for a single student, ignore all other EIDs.
        if eid and user_info['eid'] != eid: continue

        print('Grading %s (%s)' % (user_info['name'], user_info['id']))
        file_path = os.path.join(submission_path, file_name)
        if os.path.isfile(file_path):
            # Build up and run the command string to invoke the grader.
            # Generally for debugging purposes, you can pass arguments directly
            # from this script into the grader binary.
            if passargs:
                command = '%s %s %s' % (grader_path, passargs, file_path)
            else:
                command = '%s %s' % (grader_path, file_path)
            output = subprocess.Popen(command.split(' '), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            stdout, stderr = output.communicate()
            stdout = stdout.decode('utf8').replace(r'\n', '\r\n')
            stderr = stderr.decode('utf8').replace(r'\n', '\r\n')
            total_score = getGradeFromGraderOutput(stdout)
        else:
            stdout = 'Could not open %s' % (file_name)
            stderr = ''
            total_score = 0

        # If this is a dryrun, just print out the results from the grader.
        # Otherwise, save stdout and stderr into files in the student's
        # submission directory and update the report file.
        if dryrun:
            print(stdout)
        else:
            with open(os.path.join(submission_path, file_name + '.out.txt'), 'w') as file:
                file.write(stdout)
            with open(os.path.join(submission_path, file_name + '.err.txt'), 'w') as file:
                file.write(stderr)

            report_lines.append([user_info['name'], user_info['eid'], user_info['id'], str(total_score)])

    if dryrun:
        return

    report_path = os.path.join(submission_root, 'REPORT.tsv')

    if eid and os.path.isfile(report_path):
        # If this is not a dryrun and it's just for a specific student, update
        # just that student's entry in the report.
        with open(report_path, 'r') as file:
            old_report_lines = [x.rstrip().split('\t') for x in file.readlines()]
        modified = False
        for idx, old_report_line in enumerate(old_report_lines):
            if old_report_line[1] == eid:
                old_report_lines[idx] = report_lines[0]
                modified = True
                break
        if modified:
            report_lines = old_report_lines
        else:
            report_lines += old_report_lines

    # Sort the report by last name and write it out.  If this is running for all
    # students, then the report gets overwritten.  If this is running for a
    # single student, then the report just gets updated (see above).
    report_lines = sorted(report_lines, key=lambda x: x[0])
    with open(report_path, 'w') as file:
        file.writelines(['\t'.join(x) + '\n' for x in report_lines])

def upload(submission_root, assignment_name, file_name, eid=None):
    # Get the assignment ID.
    assignment = getAssignmentFromName(assignment_name)

    if not assignment:
        raise Exception('Could not find assignment %s' % (assignment_name))

    report_path = os.path.join(submission_root, 'REPORT.tsv')
    if not os.path.exists(report_path):
        raise Exception('Could not find ' + report_path)

    # Strip whitespace from the report.
    with open(report_path, 'r') as file:
        report_lines = [x.strip() for x in file.readlines()]

    # By default, iterate over all the students in the report.
    for row in report_lines:
        row_parts = row.split('\t')

        # If the grader is running for a single student, ignore all other EIDs.
        if eid and eid != row_parts[1]: continue

        user_id = row_parts[2]
        # Truncate the grade to an int.
        grade = int(float(row_parts[3]))

        # Upload stdout from the grader (i.e. the individual student's test
        # case report) as a comment on the submission on Canvas.
        comment_path = os.path.join(submission_root, user_id, file_name) + '.out.txt'

        print('Uploading grades for', row_parts[0])
        submission = assignment.get_submission(user_id)
        submission.edit(submission={'posted_grade': grade})
        if os.path.exists(comment_path):
            submission.upload_comment(comment_path)

class ArgDispatcher(object):
    def __init__(S):
        parser = argparse.ArgumentParser(description='Utility to manage EE 306 grading')
        parser.add_argument('command', help='Subcommand to run')
        args = parser.parse_args(sys.argv[1:2])
        if not hasattr(S, args.command):
            print('Unrecognized command', args.command)
            parser.print_help()
            exit(1)
        getattr(S, args.command)()

    # There are three sub-commands to the grader script:
    # 1. prepare: Extract submissions.zip, organize into directories, and pull
    #    student information from Canvas.
    # 2. grade: Grade one or all of the assignments and optionally build a
    #    grade report (necessary to upload to Canvas).
    # 3. upload: Upload grades to Canvas and attach grade report as a comment
    #    on the students' submissions.
    def prepare(S):
        parser = argparse.ArgumentParser(description='Prepare downloaded zip of submissions')
        parser.add_argument('--file', type=str, required=True, help='File that was graded')
        parser.add_argument('--zip', type=str, required=True, help='Downloaded zip path')
        parser.add_argument('--root', type=str, default='submissions', help='Submission root')
        args = parser.parse_args(sys.argv[2:])
        prepare(args.zip, args.root, args.file)

    def grade(S):
        parser = argparse.ArgumentParser(description='Grade submissions')
        parser.add_argument('--file', type=str, required=True, help='File to be graded')
        parser.add_argument('--tester', type=str, required=True, help='Unit test binary to use')
        parser.add_argument('--eid', type=str, default=None, help='EID if grading single student')
        parser.add_argument('--root', type=str, default='submissions', help='Submission root')
        parser.add_argument('--dryrun', action='store_true', help='Don\'t modify reports and output to stdout instead of file')
        parser.add_argument('--passargs', type=str, default='', help='Argument string to pass to grader binary')
        args = parser.parse_args(sys.argv[2:])
        grade(args.root, args.file, args.grader, args.eid, args.dryrun, args.passargs)

    def upload(S):
        parser = argparse.ArgumentParser(description='Upload grades and comments')
        parser.add_argument('--assignment', type=str, required=True, help='Name of assignment')
        parser.add_argument('--file', type=str, required=True, help='File that was graded')
        parser.add_argument('--eid', type=str, default=None, help='EID if grading single student')
        parser.add_argument('--root', type=str, default='submissions', help='Submission root')
        args = parser.parse_args(sys.argv[2:])
        upload(args.root, args.assignment, args.file, args.eid)

def main():
    global canvas
    global course
    global users

    # Load environment variables from .env file.
    load_dotenv()
    # Set up Canvas API.
    canvas_url = 'https://{}.instructure.com'.format(os.getenv('INSTITUTE'))
    canvas = Canvas(canvas_url, os.getenv('CANVAS_KEY'))
    course = canvas.get_course(os.getenv('COURSE_ID'))
    users = course.get_users()

    # Parse arguments.
    ArgDispatcher()

if __name__ == '__main__':
    main()
