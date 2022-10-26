import xml.etree.ElementTree as ET
import argparse
import json
import glob
import sys
import numpy as np
import tqdm
import uproot

class LesHouche:
    """
    Reads LHE (Les Houche) file and streams relevant XML elements. The schema that this 
    function obeys is described below.

    Follows this schema for reading the User Process (UP) Run common block:
    ```
    <init>
    IDBMUP_1 IDBMUP_2 EBMUP_1 EBMUP_2 PDFGUP_1 PDFGUP_2 PDFSUP_1 PDFSUP_2 IDWTUP NPRUP 
      XSECUP_1 XERRUP_1 XMAXUP_1 LPRUP_1 
      XSECUP_2 XERRUP_2 XMAXUP_2 LPRUP_2
      ...
      XSECUP_J XERRUP_J XMAXUP_J LPRUP_J 
    </init>
    ```
    - Line items (left to right):
        - IDBMUP: (int) ID of beam particle according to the Particle Data Group (PDG)
                  convention
        - EBMUP: (float) energy in GeV of beam particle
        - PDFGUP: (int) the author group for the beam according to the Cernlib PDFlib
        - PDFSUP: (int) the PDF set for the beam according to the Cernlib PDFlib
        - IDWTUP: (int) master switch dictating how the event weights (XWGTUP) are 
                  interpreted; values are described in the "gory details" link
        - NPRUP: (int) the number of different user subprocesses
        - XSECUP: (float) the XSEC for the process in picobarns (for NPRUP processes)
        - XERRUP: (float) the statistical error associated with XSECUP
        - XMAXUP: (float) the maximum XWGTUP for the process
        - LPRUP: (int) a listing of all user process IDs that can appear in IDPRUP of 
                 HEPEUP for this run

    Follows this schema for reading the UP Event common block:
    ```
    <event>
    COMMON LINE
    PARTICLE LINE
    PARTICLE LINE
    ...
    PARTICLE LINE
    </event>
    ```
    - Common line items (left to right):
        - NUP: (int) number of particle entries in this event
        - IDPRUP: (int) ID of the process for this event
        - XWGUP: (float) event weight
        - SCALUP: (float) scale of the event in GeV, as used for calculation of PDFs
        - AQEDUP: (float) the QED coupling α_QED used for this event (e.g. 1/128)
        - AQCDUP: (float) the QCD coupling α_QCD used for this event
    - Particle line items (left to right):
        - IDUP: (int) particle ID according to PDG convention
        - ISTUP: (int) status code (e.g. incoming = +1, outgoing = -1)
        - MOTHUP1: (int) index of first mother
        - MOTHUP2: (int) index of last mother
        - ICOLUP1: (int) integer tag for the color flow line passing through the color 
                   of the particle
        - ICOLUP2: (int) integer tag for the color flow line passing through the anti-
                  color of the particle
        - P_X: (float) x-component of lab frame 3-momentum of the particle
        - P_Y: (float) y-component of lab frame 3-momentum of the particle
        - P_Z: (float) z-component of lab frame 3-momentum of the particle
        - E: (float) Energy-component of lab frame 4-momentum of the particle
        - M: (float) the 'generated mass' of the particle
        - VTIMUP: (float) invariant lifetime cτ in mm (distance from production to 
                  decay)
        - SPINUP: (float) cosine of the angle between the spin-vector of particle and 
                  the 3-momentum of the decaying particle, specified in the lab frame
    
    Official documentation of this schema: 
        - File structure: https://arxiv.org/pdf/hep-ph/0609017.pdf
        - Gory details: https://arxiv.org/pdf/hep-ph/0109068.pdf
    """
    def __init__(self, lhe_file):
        major, minor, patch = uproot.__version__.split(".")
        if int(major) < 4 or int(minor) < 2:
            raise Exception(
                f"uproot version 4.2.x or higher required; {uproot.__version__} installed"
            )

        self.lhe_file = lhe_file
        self.xml_root = None
        self.xml_context = None
        self.beam = {}
        self.processes = {}
        self.run_schema = {
            "beam": [
                ("IDBMUP_1", int), ("IDBMUP_2", int), ("EBMUP_1", float), ("EBMUP_2", float),
                ("PDFGUP_1", int), ("PDFGUP_2", int), ("PDFSUP_1", int), ("PDFSUP_2", int),
                ("IDWTUP", int), ("NPRUP", int)
            ],
            "processes": [
                ("XSECUP", float), ("XERRUP", float), ("XMAXUP", float), ("LPRUP", int)
            ]
        }
        self.event_schema = {
            "common": [
                ("NUP", int), ("IDPRUP", int), ("XWGUP", float),
                ("SCALUP", float), ("AQEDUP", float), ("AQCDUP", float)
            ],
            "particles": [
                ("IDUP", int), ("ISTUP", int),
                ("MOTHUP1", int), ("MOTHUP2", int),
                ("ICOLUP1", int), ("ICOLUP2", int),
                ("P_X", float), ("P_Y", float), ("P_Z", float), 
                ("E", float), ("M", float),
                ("VTIMUP", float), ("SPINUP", float)
            ],
            "weights": ["rwgt"]
        }

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.close()

    def __get_lines(self, text):
        return list(filter(None, text.split("\n")))

    def __get_items(self, line):
        return list(filter(None, line.split()))

    def open(self):
        self.xml_context = ET.iterparse(self.lhe_file)
        xml_evt, self.xml_root = next(self.xml_context)
        for xml_evt, xml_elem in self.xml_context:
            if xml_elem.tag == "init":
                beam_line, *process_lines = self.__get_lines(xml_elem.text)
                # Extract beam information
                for item_i, item in enumerate(self.__get_items(beam_line)):
                    key, typ = self.run_schema["beam"][item_i]
                    self.beam[key] = [typ(item)]
                # Extract process information
                for process_line in process_lines:
                    for item_i, item in enumerate(self.__get_items(process_line)):
                        key, typ = self.run_schema["processes"][item_i]
                        if key not in self.processes:
                            self.processes[key] = []
                        self.processes[key].append(typ(item))

                xml_elem.clear()
                break

    def close(self):
        del self.xml_context
        del self.xml_root

    @property
    def events(self):
        for xml_evt, xml_elem in self.xml_context:
            if xml_elem.tag == "event":
                common_line, *particle_lines = self.__get_lines(xml_elem.text)
                event = {}
                # Extract event information
                for item_i, item in enumerate(self.__get_items(common_line)):
                    key, typ = self.event_schema["common"][item_i]
                    event[key] = typ(item)
                for particle_line in particle_lines:
                    for item_i, item in enumerate(self.__get_items(particle_line)):
                        key, typ = self.event_schema["particles"][item_i]
                        if key not in event:
                            event[key] = []
                        event[key].append(typ(item))
                # Extract event weights
                wgt_elems = xml_elem.find("rwgt") or []
                for wgt_elem in wgt_elems:
                    for wgt_name in self.event_schema["weights"]:
                        if wgt_name in wgt_elem.attrib["id"]:
                            if wgt_name not in event:
                                event[wgt_name] = []
                            event[wgt_name].append(float(wgt_elem.text))
                xml_elem.clear()
                yield event

def lhe_to_json(lhe_file):
    with LesHouche(lhe_file) as lhe:
        events = [event for event in lhe.events]
    with open(lhe_file.replace(".lhe", ".json")) as json_file:
        json.dump(events, json_file)

def lhe_to_root(lhe_file, basket_nbytes=100000):
    with uproot.recreate(lhe_file.replace('.lhe', '.root')) as root_file:
        with LesHouche(lhe_file) as lhe:
            particle_branches = [key for key, typ in lhe.event_schema["particles"]]
            events = {}
            events_nbytes = 0
            for event in tqdm.tqdm(lhe.events, desc="Parsing LHE events"):
                if not events:
                    events = {branch: [] for branch in event}
                # Update events buffer
                for branch, leaf in event.items():
                    if type(leaf) == list:
                        leaf = np.array(leaf)
                        events_nbytes += leaf.nbytes
                    else:
                        events_nbytes += sys.getsizeof(leaf)
                    events[branch].append(leaf)
                # Write out events buffer
                if events_nbytes > basket_nbytes:
                    events = __lhe_to_uproot_events(events, jagged_branches=particle_branches)
                    if "Events" not in root_file:
                        root_file["Events"] = events
                    else:
                        root_file["Events"].extend(events)
                    # Flush events buffer
                    events = {branch: [] for branch in events}
                    events_nbytes = 0
            # Pick up remaining events
            events = __lhe_to_uproot_events(events, jagged_branches=particle_branches)
            root_file["Events"].extend(events)
            # Write metadata TTrees
            root_file["Beam"] = lhe.beam
            root_file["Processes"] = lhe.processes

def __lhe_to_uproot_events(events, jagged_branches=[]):
    for branch, leaves in events.items():
        if branch in jagged_branches:
            events[branch] = np.array(leaves, dtype=object)
        else:
            events[branch] = np.array(leaves)
    return events

if __name__ == "__main__":
    for lhe_file in sys.argv[1:]:
        print(f"Converting {lhe_file} to {lhe_file.replace('.lhe', '.root')}")
        lhe_to_root(lhe_file)
