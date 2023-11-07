class Datacard:
    def __init__(self, obs, sig, bkg, systs):
        self.obs = obs
        self.n_obs = len(obs)

        self.bin_labels = [f"bin{i+1}" for i in range(self.n_obs)]
        
        self.sig_labels = []
        self.sig_yields = [[] for _ in range(self.n_obs)]
        for sig_label, sig_yields in sig.items():
            self.sig_labels.append(sig_label)
            for bin_i, sig_yield in enumerate(sig_yields):
                self.sig_yields[bin_i].append(sig_yield)
                
        self.bkg_labels = []
        self.bkg_yields = [[] for _ in range(self.n_obs)]
        for bkg_label, bkg_yields in bkg.items():
            self.bkg_labels.append(bkg_label)
            for bin_i, bkg_yield in enumerate(bkg_yields):
                self.bkg_yields[bin_i].append(bkg_yield)

        self.n_sig = len(self.sig_labels)
        self.n_bkg = len(self.bkg_labels)
        
        self.column_width = max([len(l) for l in self.sig_labels + self.bkg_labels])+2
        self.column_width = max(self.column_width, 12)

        syst_dists = {}
        sample_systs = {}
        for sample_label, labeled_systs in systs.items():
            sample_systs[sample_label] = {}
            for syst_label, syst_values in labeled_systs.items():
                if type(syst_label) != tuple:
                    syst_dists[syst_label] = "lnN"
                    sample_systs[sample_label][syst_label] = syst_values
                elif len(syst_label) == 2:
                    syst_dists[syst_label[0]] = syst_label[1]
                    sample_systs[sample_label][syst_label[0]] = syst_values
                else:
                    raise ValueError(
                        "systs must be labled with a string (assume lnN) or (name, dist) pairs"
                    )
                
        self.syst_labels = []
        self.systs = []
        n_samples = self.n_sig + self.n_bkg
        cw = self.column_width
        for sample_label, labeled_systs in sample_systs.items():
            for syst_label, systs in labeled_systs.items():
                # Register syst
                if syst_label not in self.syst_labels:
                    self.syst_labels.append(syst_label)
                    self.systs.append([f"{'-':>{cw}}" for _ in range(self.n_obs*(n_samples))])
                
                # Get index of syst label
                label_i = self.syst_labels.index(syst_label)
                
                # Assign syst values
                if sample_label in self.sig_labels:
                    for value_i, syst_value in enumerate(systs):
                        syst_i = value_i + self.sig_labels.index(sample_label)
                        self.systs[label_i][syst_i] = f"{syst_value:{cw}.4f}" 
                elif sample_label in self.bkg_labels:
                    for value_i, syst_value in enumerate(systs):
                        if syst_value >= 0:
                            syst_i = self.n_obs*self.n_sig + value_i + self.bkg_labels.index(sample_label)
                            self.systs[label_i][syst_i] = f"{syst_value:{cw}.4f}"  
                    
        hw = max([len(l) for l in self.syst_labels])+2
        hw = max(hw, 12)
        for syst_label, syst_dist in syst_dists.items():
            label_i = self.syst_labels.index(syst_label)
            self.syst_labels[label_i] = f"{syst_label:<{hw}} {syst_dist} "

        self.header_width = max([len(l) for l in self.syst_labels])+2
        self.header_width = max(self.header_width, hw)
        
        self.content = None
        self.fill()

    def fill(self):
        cw = self.column_width
        hw = self.header_width
        hline = "-"*(hw + cw*(self.n_sig + self.n_bkg)*self.n_obs) + "\n"
        content = ""
        # Header
        content += f"imax {self.n_obs} number of channels\n"
        content += f"jmax {self.n_bkg} number of backgrounds\n"
        content += f"kmax * number of nuisance parameters\n"
        content += hline
        # Observed
        content += f"{'bin':<{hw}}"
        content +=  "".join([f"{self.bin_labels[i]:>{cw}}" for i in range(self.n_obs)])
        content +=  "\n"
        content += f"{'observation':<{hw}}"
        content +=  "".join([f"{n:{cw}d}" for n in self.obs])
        content +=  "\n"
        content += hline
        # Expected
        content += f"{'bin':<{hw}}"
        content += "".join([f"{self.bin_labels[i]:>{cw}}" for i in range(self.n_obs) for _ in range(self.n_sig)])
        content += "".join([f"{self.bin_labels[i]:>{cw}}" for i in range(self.n_obs) for _ in range(self.n_bkg)])
        content +=  "\n"
        content += f"{'process':<{hw}}"
        content +=  "".join([f"{l:>{cw}}" for _ in range(self.n_obs) for l in self.sig_labels])
        content +=  "".join([f"{l:>{cw}}" for _ in range(self.n_obs) for l in self.bkg_labels])
        content +=  "\n"
        content += f"{'process':<{hw}}"
        content += "".join([f"{i+1:>{cw}}" for _ in range(self.n_obs) for i in range(-self.n_sig, 0)])
        content += "".join([f"{i+1:>{cw}}" for _ in range(self.n_obs) for i in range(self.n_bkg)])
        content +=  "\n"
        content += f"{'rate':<{hw}}"
        content +=  "".join([f"{y[i]:{cw}.2f}" for y in self.sig_yields for i in range(self.n_sig)])
        content +=  "".join([f"{y[i]:{cw}.2f}" for y in self.bkg_yields for i in range(self.n_bkg)])
        content +=  "\n"
        content += hline
        # Systematics
        for syst_i, syst_values in enumerate(self.systs):
            content += f"{self.syst_labels[syst_i]:<{hw}}"
            content +=  "".join(syst_values)
            content += "\n"
        content += hline
            
        self.content = content
        
    def write(self, output_dat):
        with open(output_dat, "w") as dat_out:
            dat_out.write(self.content)

class DatacardABCD(Datacard):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.bin_labels = ["A", "B", "C", "D"]

    def fill(self):
        super().fill()
        cw = self.column_width
        hw = self.header_width
        # Rateparams
        self.content += f"{'a rateParam':<{hw}}  A  {self.bkg_labels[0]:>{cw}} (@0*@1/@2) b,c,d\n"
        self.content += f"{'b rateParam':<{hw}}  B  {self.bkg_labels[0]:>{cw}} {self.obs[1]}\n"
        self.content += f"{'c rateParam':<{hw}}  C  {self.bkg_labels[0]:>{cw}} {self.obs[2]}\n"
        self.content += f"{'d rateParam':<{hw}}  D  {self.bkg_labels[0]:>{cw}} {self.obs[3]}"
