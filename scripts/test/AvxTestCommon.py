# Automated testing framework for AvxSynth.
# Common data types and error classes.

class AvxNotReadyError(Exception):
    '''Indicates that results have not yet been obtained for a test'''
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class AvxFileError(Exception):
    '''Indicates that one or more output files could not be opened'''
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class AvxScriptError(Exception):
    '''Indicates an error occurred when opening the script'''
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class AvxRuntimeError(Exception):
    '''Indicates an error was encountered while running the script'''
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)
