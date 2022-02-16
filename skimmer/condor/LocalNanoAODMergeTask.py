import os

from metis.Task import Task
from metis.File import File, MutableFile
import metis.Utils as Utils

import ROOT as r
import time

class LocalNanoAODMergeTask(Task):
    def __init__(self, **kwargs):
        """
        Takes a list of input file paths and a single full absolute output filename
        and performs a single merge operation
        """
        self.input_filenames = kwargs.get("input_filenames", [])
        self.output_filename = kwargs.get("output_filename", [])
        self.io_mapping = kwargs.get("io_mapping", [])
        self.ignore_bad = kwargs.get("ignore_bad", False)
        self.show_progress = kwargs.get("show_progress", True)
        self.update_mapping()
        super(self.__class__, self).__init__(**kwargs)

    def get_inputs(self):
        return sum([x[0] for x in self.io_mapping], [])

    def get_outputs(self):
        return sum([x[1] for x in self.io_mapping], [])

    def update_mapping(self):
        if self.io_mapping: return
        self.io_mapping = [
            [map(File,self.input_filenames), [File(self.output_filename),]]
        ]

    def process(self):
        done = all(map(lambda x: x.exists(), self.get_outputs()))
        self.logger.info("Begin processing")
        if not done:
            self.merge_function(self.get_inputs(), self.get_outputs()[0])
        self.logger.info("End processing")

    def merge_function(self, inputs, output):
        # make the directory hosting the output if it doesn't exist
        fdir = output.get_basepath()
        if not os.path.exists(fdir):
            result = Utils.do_cmd("mkdir -p {}".format(fdir))

        ntotal = len(inputs)
        ngood = 0
        goodinputs = []
        for inp in inputs:
            if self.ignore_bad and not inp.exists(): 
                continue
            if not os.path.exists(inp.get_name()): 
                continue
            goodinputs.append(inp.get_name())
            ngood += 1

        self.logger.info("Adding {} files to be merged. {} were found to be good".format(ntotal, ngood))

        out = Utils.do_cmd("python haddnano.py {output} {inputfiles}".format(output=output.get_name(), inputfiles=" ".join(goodinputs)))

        logfile = output.get_name().replace(".root", ".log")

        f = open(logfile, "w")
        f.write(out)
        f.close()

        self.logger.info("Done merging files into {}".format(output.get_name()))

if __name__ == "__main__":
    pass
