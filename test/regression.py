#!/usr/bin/env python

import os
import subprocess
import concurrent.futures

def runTest(test_dir, src_name, bin_path):
    src_path = os.path.join(test_dir, src_name)
    bless_path = os.path.join(test_dir, 'BLESSING')

    if not os.path.isfile(src_path) or not os.path.isfile(bless_path):
        return False

    with open(bless_path) as bless_file:
        bless_lines = bless_file.readlines()
        seed = int(bless_lines[0])
        expected = int(bless_lines[1])

    command = '%s %s --seed=%d' % (bin_path, src_path, seed)
    output = subprocess.Popen(command.split(' '), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, _ = output.communicate()
    stdout = stdout.decode('ISO-8859-1').replace(r'\n', '\r\n')

    stdout_lines = [x.rstrip() for x in stdout.split('\n')]
    if len(stdout_lines) < 2: actual = 0
    elif not 'successful' in stdout_lines[1]: actual = 0
    else: actual = int(float(stdout_lines[-2].split(' ')[3].split('/')[0]))

    return stdout, actual, expected

def runTests(test_root, src_name, bin_path):
    failures = list()
    hist = {'PASS': 0, 'FAIL': 0, 'FATAL': 0}

    print('========== %s ==========' % test_root)
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = {
            executor.submit(runTest, os.path.join(test_root, test_dir), src_name, bin_path): test_dir
            for test_dir in os.listdir(test_root)
        }

        for future in concurrent.futures.as_completed(futures):
            test_dir = futures[future]
            try:
                stdout, actual, expected = future.result()
            except Exception as e:
                status = 'FATAL'
                failures.append((test_dir, str(e), 0, 0))
            else:
                if actual == expected:
                    status = 'PASS'
                else:
                    status = 'FAIL'
                    failures.append((test_dir, stdout, actual, expected))
            print('[%s] %s' % (status, test_dir))
            hist[status] += 1

    if len(failures) > 0:
        print('----- FAILURES -----')
        for failure in failures:
            test_dir, stdout, actual, expected = failure
            print('----- %s -----' % test_dir)
            print('Actual (%d) != Expected (%d)' % (actual, expected))
            print(stdout)

    return hist

def main():
    tests = [{
        'test_root': 'dataset/2',
        'src_name': 'sort.asm',
        'bin_path': '../build/bin/sort'
    }, {
        'test_root': 'dataset/3',
        'src_name': 'intersection.asm',
        'bin_path': '../build/bin/intersection'
    }, {
        'test_root': 'dataset/4',
        'src_name': 'nim.asm',
        'bin_path': '../build/bin/nim'
    }, {
        'test_root': 'dataset/5',
        'src_name': 'interrupt.asm',
        'bin_path': '../build/bin/interrupt2'
    }, {
        'test_root': 'dataset/6',
        'src_name': 'binsearch.asm',
        'bin_path': '../build/bin/binsearch'
    }, {
        'test_root': 'dataset/7',
        'src_name': 'interrupt1.asm',
        'bin_path': '../build/bin/interrupt1'
    }, {
        'test_root': 'dataset/8',
        'src_name': 'polyroot.asm',
        'bin_path': '../build/bin/polyroot'
    }, {
        'test_root': 'dataset/9',
        'src_name': 'pow2.bin',
        'bin_path': '../build/bin/pow2'
    }]

    hist = {'PASS': 0, 'FAIL': 0, 'FATAL': 0}
    for test in tests:
        if not os.path.isdir(test['test_root']) or not os.path.isfile(test['bin_path']):
            print('Skipping %s ... did you extract the dataset and build in ../build?' % test['test_root'])
            continue
        local_hist = runTests(**test)
        for key, value in local_hist.items():
            hist[key] += value
        print(local_hist)
    print(hist)

if __name__ == '__main__':
    main()
