import xml.etree.ElementTree as ET
import argparse
import json
import sys
import numpy as np
import tqdm
import uproot

class LesHouche:
    """
    Reads LHE (Les Houche) file and streams relevant XML elements.

    Follows this schema for reading header information:
    <header>
    ...
    <MGGenerationInfo>
    #  Number of Events        :  500
    #  Integrated weight (pb)  :  XSEC
    </MGGenerationInfo>
    </header>

    Follows this schema for reading event-level information:
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
        NUP: (int) number of particle entries in this event
        IDPRUP: (int) ID of the process for this event
        XWGUP: (float) event weight
        SCALUP: (float) scale of the event in GeV, as used for calculation of PDFs
        AQEDUP: (float) the QED coupling α_QED used for this event (e.g. 1/128)
        AQCDUP: (float) the QCD coupling α_QCD used for this event
    - Particle line items (left to right):
        IDUP: (int) particle ID according to PDG convention
        ISTUP: (int) status code (e.g. incoming = +1, outgoing = -1)
        MOTHUP1: (int) index of first mother
        MOTHUP2: (int) index of last mother
        ICOLUP1: (int) integer tag for the color flow line passing through the color 
            of the particle
        ICOLUP2: (int) integer tag for the color flow line passing through the anti-
            color of the particle
        P_X: (float) x-component of lab frame 3-momentum of the particle
        P_Y: (float) y-component of lab frame 3-momentum of the particle
        P_Z: (float) z-component of lab frame 3-momentum of the particle
        E: (float) Energy-component of lab frame 4-momentum of the particle
        M: (float) the 'generated mass' of the particle
        VTIMUP: (float) invariant lifetime cτ in mm (distance from production to decay)
        SPINUP: (float) cosine of the angle between the spin-vector of particle and the 
            3-momentum of the decaying particle, specified in the lab frame
    """
    def __init__(self, lhe_file):
        self.lhe_file = lhe_file
        self.xml_root = None
        self.xml_context = None
        self.header = {}
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

    def open(self):
        self.xml_context = ET.iterparse(self.lhe_file)
        xml_evt, self.xml_root = next(self.xml_context)
        for xml_evt, xml_elem in self.xml_context:
            if xml_elem.tag == "header":
                # Extract cross section (xsec)
                madgraph_info = xml_elem.find("MGGenerationInfo")
                lines = self.__get_lines(madgraph_info.text)
                if "Integrated weight (pb)" in lines[-1]:
                    self.header["XSEC"] = float(lines[-1].split(":")[-1])
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
                for item_i, item in enumerate(common_line.split()):
                    key, typ = self.event_schema["common"][item_i]
                    event[key] = typ(item)
                for particle_line in particle_lines:
                    for item_i, item in enumerate(particle_line.split()):
                        key, typ = self.event_schema["particles"][item_i]
                        if key not in event:
                            event[key] = []
                        event[key].append(typ(item))
                # Extract event weights
                for wgt_elem in xml_elem.find("rwgt"):
                    for wgt_name in self.event_schema["weights"]:
                        if wgt_name in wgt_elem.attrib["id"]:
                            if wgt_name not in event:
                                event[wgt_name] = []
                            event[wgt_name].append(float(wgt_elem.text))
                xml_elem.clear()
                yield event

    def __get_lines(self, text):
        return list(filter(None, text.split("\n")))

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
            for event in tqdm.tqdm(lhe.events):
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
            # Convert header dict into a dict of lists
            header = {
                k: [v] if type(v) != list else v for k, v in lhe.header.items()
            }
            root_file["Header"] = header

def __lhe_to_uproot_events(events, jagged_branches=[]):
    for branch, leaves in events.items():
        if branch in jagged_branches:
            events[branch] = np.array(leaves, dtype=object)
        else:
            events[branch] = np.array(leaves)
    return events

if __name__ == "__main__":
    lhe_to_root("/ceph/cms/store/user/jguiang/VBSVVHSignalGeneration/lhe/VBSWWH_Inclusive_4f_LO_10k.lhe")
    input()
