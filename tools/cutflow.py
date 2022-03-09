class Cut:
    def __init__(self, name, parent=None, left=None, right=None, 
                 n_pass=0, n_pass_weighted=0., n_fail=0, n_fail_weighted=0.):
        self.name = name
        self.n_pass = n_pass
        self.n_pass_weighted = n_pass_weighted
        self.n_fail = n_fail
        self.n_fail_weighted = n_fail_weighted
        self.parent = parent
        self.left = left
        self.right = right

    def __eq__(self, other_cut):
        if self.parent:
            same_parent = (self.parent.name == other_cut.parent.name)
        else:
            same_parent = (not self.parent and not other_cut.parent)
        if self.left:
            same_left = (self.left.name == other_cut.left.name)
        else:
            same_left = (not self.left and not other_cut.left)
        if self.right:
            same_right = (self.right.name == other_cut.right.name)
        else:
            same_right = (not self.right and not other_cut.right)
        same_lineage = (same_parent and same_left and same_right)
        same_name = (self.name == other_cut.name)
        return same_name and same_lineage

    def __add__(self, other_cut):
        if self == other_cut:
            cut_sum = Cut(
                self.name,
                n_pass=(self.n_pass + other_cut.n_pass),
                n_pass_weighted=(self.n_pass_weighted + other_cut.n_pass_weighted)
            )
            return cut_sum
        else:
            raise ValueError("can only add equivalent cuts")

class Cutflow:
    def __init__(self):
        self.__cuts = {}
        self.__root_cut_name = None
        self.__terminal_cut_names = []
        self.__cut_network = {}

    def __getitem__(self, name):
        return self.__cuts[name]

    def __eq__(self, other_cutflow):
        return self.__cut_network == other_cutflow.__cut_network

    def __add__(self, other_cutflow):
        if self != other_cutflow:
            raise ValueError("can only add equivalent cutflows")
        else:
            summed_cuts = {}
            for name, cut in self.items():
                other_cut = other_cutflow[name]
                summed_cuts[name] = cut + other_cut
            return Cutflow.from_network(cuts=summed_cuts, cut_network=self.__cut_network)

    def __recursive_print(self, cut, tabs=""):
        if cut is self.root_cut():
            prefix = ""
        elif cut is cut.parent.left:
            if not cut.parent.right:
                prefix = f"{tabs}\u2514\u2612\u2500"
                tabs += "    "
            else:
                prefix = f"{tabs}\u251C\u2612\u2500"
                tabs += "\u2502   "
        elif cut is cut.parent.right:
            prefix = f"{tabs}\u2514\u2611\u2500"
            tabs += "    "

        print(f"{prefix}{cut.name}")
        print(f"{tabs}pass: {cut.n_pass} (raw) {cut.n_pass_weighted} (wgt)")
        print(f"{tabs}fail: {cut.n_fail} (raw) {cut.n_fail_weighted} (wgt)")

        if cut.left:
            self.__recursive_print(cut.left, tabs=tabs)
        if cut.right:
            self.__recursive_print(cut.right, tabs=tabs)

    def print(self):
        self.__recursive_print(self.root_cut())

    def items(self):
        return self.__cuts.items()

    def cut_names(self):
        return self.__cuts.keys()

    def cuts(self):
        return self.__cuts.values()

    def terminal_cuts(self):
        return [self.__cuts[name] for name in self.__terminal_cut_names]

    def root_cut(self):
        return self.__cuts[self.__root_cut_name]

    @staticmethod
    def from_network(cuts, cut_network):
        cutflow = Cutflow()
        # Check inputs
        if not cuts and not cut_network:
            raise ValueError("list of cuts and cut network both empty")
        elif not cuts:
            raise ValueError("list of cuts empty")
        elif not cut_network:
            raise ValueError("cut network empty")
        elif not set(cuts) == set(cut_network.keys()):
            raise ValueError("list of cuts does not match cut network")
        # Build cutflow
        cutflow.__cuts = cuts
        cutflow.__cut_network = cut_network
        for name, (parent_name, left_name, right_name) in cut_network.items():
            if parent_name:
                cutflow.__cuts[name].parent = cutflow.__cuts[parent_name]
            else:
                if cutflow.__root_cut_name:
                    raise Exception("invalid cutflow - multiple root cuts")
                else:
                    cutflow.__root_cut_name = name
            if left_name:
                cutflow.__cuts[name].left = cutflow.__cuts[left_name]
            if right_name:
                cutflow.__cuts[name].right = cutflow.__cuts[right_name]
            if not left_name and not right_name:
                cutflow.__terminal_cut_names.append(name)
        # Check cutflow and return it if it is healthy
        if len(cutflow.__terminal_cut_names) == 0:
            raise Exception("invalid cutflow - no terminal cuts")
        elif not cutflow.__root_cut_name:
            raise Exception("invalid cutflow - no root cut")
        else:
            return cutflow

    @staticmethod
    def from_file(cutflow_file, delimiter=","):
        cuts = {}
        cut_network = {}
        with open(cutflow_file, "r") as f_in:
            for line in f_in:
                # Read cut attributes
                line = line.replace("\n", "")
                cut_attr = line.split(delimiter)
                # Extract basic info
                name = cut_attr[0]
                n_pass, n_pass_weighted, n_fail, n_fail_weighted = cut_attr[1:5]
                # Cast string counts into numbers
                n_pass = int(n_pass)
                n_fail = int(n_fail)
                n_pass_weighted = float(n_pass_weighted)
                n_fail_weighted = float(n_fail_weighted)
                # Extract lineage
                parent_name, left_name, right_name = cut_attr[5:]
                # Cast 'null' values into NoneType
                if parent_name == "null":
                    parent_name = None
                if left_name == "null":
                    left_name = None
                if right_name == "null":
                    right_name = None
                # Create new cut and update cut network
                cuts[name] = Cut(
                    name, 
                    n_pass=n_pass, n_pass_weighted=n_pass_weighted, 
                    n_fail=n_fail, n_fail_weighted=n_fail_weighted
                )
                cut_network[name] = (parent_name, left_name, right_name)

        return Cutflow.from_network(cuts=cuts, cut_network=cut_network)

if __name__ == "__main__":
    zzz_cutflow = Cutflow.from_file("studies/pilot/output/2018/ZZZ_Cutflow.cflow")
    wwz_cutflow = Cutflow.from_file("studies/pilot/output/2018/WWZ_4F_Cutflow.cflow")
    (zzz_cutflow + wwz_cutflow).print()
