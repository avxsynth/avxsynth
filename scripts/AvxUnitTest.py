#!/usr/bin/python
#
# Automated testing framework for AvxSynth.
# Individual unit tests and analysis.
#
# The MD5 hash algorithm will be used for data comparison
# For a description of the test suite format, see README.TXT
#
# Required test suite format: 1

import os
import sys

from AvxFrameServer import *
from AvxTestCommon import *


class AvxUnitTest(object):
    '''AvxUnitTest:

    Controls evaluation of a test script and compares output to reference
    '''
    def __init__(self, avs_name, main_hash, extra_hash_table={}):
        '''Parameters:

        avs_name: pathname of script to open
        main_hash: expected hashsum of main output
        extra_hash_table: 
        '''
        if not isinstance(avs_name, str):
            raise ValueError('avs_name must be a str')
        elif not isinstance(main_hash, str):
            raise ValueError('main_hash must be a str')
        elif not isinstance(extra_hash_table, dict):
            raise ValueError('extra_hash_table must be a dict')
        
        self.avs_name = avs_name
        self.main_hash = main_hash
        self.extra_hash_table = extra_hash_table

        self.frameserver = AvxFrameServer(self.avs_name,
                                          self.extra_hash_table.keys())
        self.results = [None, {}]
        self.complete = False

    def run(self, dumpav=False, source_chdir='.', output_chdir='.'):
        '''Evalute script.  Check the status of self.complete first.'''
        self.frameserver.run(dumpav, source_chdir, output_chdir)
        self.results = self.frameserver.query_results()
        self.complete = True

    def check_results(self):
        '''Checks results against main_hash and extra_hash_table.  Returns a
        list of all files with non-matching hashes.  Check the status of
        self.complete first.'''        
        if not self.complete:
            raise AvxNotReadyError('unit test has not yet executed')
        not_valid_files = []
        # Results format: [main_hash, {extra_file_1:extra_hash_1, ...}]
        if self.main_hash != self.results[0]:
            not_valid_files.append('AVX_FRAMESERVER_OUTPUT')
        for i in self.extra_hash_table:
            if self.extra_hash_table[i] != self.results[1][i]:
                not_valid_files.append(i)
        return not_valid_files

    def test_file_format(self):
        '''Returns a string suitable for writing to a test suite file.  Check
        the status of self.complete first.'''
        if not self.complete:
            raise AvxNotReadyError('unit test has not yet executed')
        # Format: SCRIPT,avs_name,hash OR extra_file_name,hash
        file_str = ['SCRIPT,"{0}",{1}'.format(self.avs_name, self.results[0])]
        for i in self.extra_hash_table:
            file_str.append('"{0}",{1}'.format(i, self.results[1][i]))
        return '\n'.join(file_str)

    def cleanup(self, output_chdir='.'):
        '''Delete generated files listed in self.extra_hash_table and reset
        self.complete'''
        self.frameserver.cleanup(output_chdir)
        self.results = [None, {}]
        self.complete = False


def parse_line(line):
    '''Parse a line from a test suite file.  Returns a list of comma-separated
    values, escaped by regular quotes.  A value of None is returned for comment
    lines and empty lines'''
    if line.strip().startswith('#') or line.strip() == '':
        return None
    line = line.strip('\n')
    parsed = [[]]
    within_quotes = False
    # Write each comma-separated-value as its own list element.  Quotes escape.
    for i in line:
        if i == '"':
            within_quotes = not within_quotes
            continue
        elif within_quotes:
            parsed[-1].append(i)
            continue
        elif i == ',':
            parsed.append([])
            continue
        parsed[-1].append(i)
    for i in range(len(parsed)):
        parsed[i] = ''.join(parsed[i])
    return parsed


def parse_file(infile):
    '''Parse the test suite file named infile.  Returns a list of AvsUnitTest
    objects for each test listed in the file.'''
    file = open(infile)
    test_units = []
    last_test_unit = [None, '', {}]
    for i in file:
        orig_line = i
        line = parse_line(i)
        if line == None:
            continue
        if (line[0] == 'SCRIPT'):
            if last_test_unit[0] != None:
                test_units.append(AvxUnitTest(*last_test_unit))
                # The last parameter is a dict so a new object is required.
                last_test_unit = [None, '', {}]
            # Line format: SCRIPT,avs_name,hash
            try:
                last_test_unit[0] = line[1]
                last_test_unit[1] = line[2]
            except IndexError:
                errmsg = 'malformed line: {0}\n'
                sys.stderr.write(errmsg.format(orig_line))
        else:
            # Line format: extra_file_name,hash
            try:
                last_test_unit[2][line[0]] = line[1]
            except IndexError:
                errmsg = 'malformed line: {0}\n'
                sys.stderr.write(errmsg.format(orig_line))
    # Flush the last entry
    if last_test_unit[0] != None:
        test_units.append(AvxUnitTest(*last_test_unit))
    return test_units


def main(infile, logfile='', dumpav=False, savefiles=False,
         source_chdir=".", output_chdir='.'):
    '''AvxSynth automated testing framework:  Execute the test suite file
    infile and write results to outfile in test suite format.'''
    unit_tests = parse_file(infile)
    if logfile:
        out = open(logfile, mode='w')
    else:
        out = open(os.devnull, mode='w')
    test_has_failed = []
    orig_dir = os.getcwd()
    for i in unit_tests:
        os.chdir(orig_dir)
        test_name = i.avs_name
        try:
            # Some filters will append instead of overwriting.
            i.cleanup(output_chdir)
            i.run(dumpav, source_chdir, output_chdir)
        except (AvxScriptError, AvxRuntimeError, AvxFileError) as err:
            test_has_failed.append(test_name)
            errmsg = 'Test {0} failed.  Error: {1}\n'
            sys.stderr.write(errmsg.format(test_name, err))
            out.write('SCRIPT,"{0}",{1}\n'.format(test_name, 'ERROR'))
            for j in i.extra_hash_table.keys():
                out.write('"{0}",{1}\n'.format(j, 'ERROR'))
            continue
        # check_results() returns a list of failed files.
        test_result = i.check_results()
        if test_result:
            errmsg = 'Test {0} failed.  Output mismatch in {1}.\n'
            sys.stderr.write(errmsg.format(test_name, test_result))
            test_has_failed.append(test_name)
        else:
            sys.stderr.write('Test {0} pass...\n'.format(test_name))
        out.write(i.test_file_format() + '\n')
        if not savefiles:
            i.cleanup(output_chdir)
    if test_has_failed:
        errmsg = 'The following tests have failed: {0}\n'
        sys.stderr.write(errmsg.format(test_has_failed))
        return 2
    return 0

if __name__ == '__main__':
    try:
        infile = ''
        _main_logfile = ''
        _main_dumpav = False
        _main_savefiles = False
        _main_chdir = '.'
        idx = 1
        while not infile:
            arg = sys.argv[idx]
            if arg == '--log':
                _main_logfile = sys.argv[idx + 1]
                idx += 2
                continue                
            elif arg == '--dump-av':
                _main_dumpav = True
                idx += 1
                continue
            elif arg == '--save-files':
                _main_savefiles = True
                idx += 1
                continue
            elif arg == '--source-chdir':
                _main_source_chdir = sys.argv[idx + 1]
                idx += 2
                continue
            elif arg == '--output-chdir':
                _main_output_chdir = sys.argv[idx + 1]
                idx += 2
                continue
            else:
                infile = arg
                idx += 1
                continue
        if len(sys.argv) > (idx):
            warn = 'WARNING: Unused arguments {}\n'
            sys.stderr.write(warn.format(sys.argv[idx + 1:]))
    except:
        script_name = os.path.basename(sys.argv[0])
        usage = '''
Usage: {} [--dump-av] [--save-files] [--source-chdir DIR] [--output-chdir DIR] [--log LOG] infile
'''
        sys.stderr.write(usage.format(script_name))
        sys.exit(2)
    sys.exit(main(infile, _main_logfile, _main_dumpav, _main_savefiles,
                  _main_source_chdir, _main_output_chdir))
