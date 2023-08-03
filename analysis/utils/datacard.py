class Datacard:
    def __init__(self, obs, sig, bkg, systs):
        self.obs = obs
        self.n_obs = len(obs)
        
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
                
        self.syst_labels = []
        self.systs = []
        n_samples = self.n_sig + self.n_bkg
        cw = self.column_width
        for sample_label, labeled_systs in systs.items():
            for syst_label, syst_values in labeled_systs.items():
                # Register syst
                if syst_label not in self.syst_labels:
                    self.syst_labels.append(syst_label)
                    self.systs.append([f"{'-':>{cw}}" for _ in range(self.n_obs*(n_samples))])
                
                # Get index of syst label
                label_i = self.syst_labels.index(syst_label)
                
                # Assign syst values
                if sample_label in self.sig_labels:
                    for value_i, syst_value in enumerate(syst_values):
                        syst_i = self.n_obs+value_i
                        self.systs[label_i][syst_i] = f"{syst_value:{cw}.4f}" 
                elif sample_label in self.bkg_labels:
                    for value_i, syst_value in enumerate(syst_values):
                        syst_i = value_i
                        self.systs[label_i][syst_i] = f"{syst_value:{cw}.4f}"  
                else:
                    raise Exception(f"{sample_label} not found")
                    
        self.header_width = max([len(l) for l in self.syst_labels])+2
        self.header_width = max(self.header_width, 12)
        
        self.content = None
        self.fill()
        
    def fill(self):
        cw = self.column_width
        hw = self.header_width
        hline = "-"*(hw+5 + cw*(self.n_sig + self.n_bkg)*self.n_obs) + "\n"
        content = ""
        content += f"imax {self.n_obs} number of channels\n"
        content += f"jmax {self.n_bkg} number of backgrounds\n"
        content += f"kmax {len(self.systs)} number of nuisance parameters\n"
        content += hline
        content += f"{'bin':<{hw+5}}"
        content +=  "".join([f"{'bin'+str(i+1):>{cw}}" for i in range(self.n_obs)])
        content +=  "\n"
        content += f"{'observation':<{hw+5}}"
        content +=  "".join([f"{n:{cw}d}" for n in self.obs])
        content +=  "\n"
        content += hline
        content += f"{'bin':<{hw+5}}"
        content += "".join([f"{'bin'+str(i+1):>{cw}}" for i in range(self.n_obs) for _ in range(self.n_sig + self.n_bkg)])
        content +=  "\n"
        content += f"{'process':<{hw+5}}"
        content +=  "".join([f"{l:>{cw}}" for _ in range(self.n_obs) for l in self.sig_labels + self.bkg_labels])
        content +=  "\n"
        content += f"{'process':<{hw+5}}"
        content += "".join([f"{i:>{cw}}" for _ in range(self.n_obs) for i in range(self.n_sig + self.n_bkg)])
        content +=  "\n"
        content += f"{'rate':<{hw+5}}"
        content +=  "".join([f"{y[i]:{cw}.2f}" for y in self.sig_yields for i in range(self.n_sig)])
        content +=  "".join([f"{y[i]:{cw}.2f}" for y in self.bkg_yields for i in range(self.n_bkg)])
        content +=  "\n"
        content += hline
        for syst_i, syst_values in enumerate(self.systs):
            content += f"{self.syst_labels[syst_i]:<{hw}}"
            content +=  " lnN "
            content +=  "".join(syst_values)
            content += "\n"
            
        self.content = content
        
    def write(self, output_dat):
        with open(output_dat, "w") as dat_out:
            dat_out.write(self.content)

class DatacardABCD(Datacard):
    def fill(self):
        bin_labels = ("A", "B", "C", "D")
        cw = self.column_width
        hw = self.header_width
        hline = "-"*(hw+5 + cw*(self.n_sig + self.n_bkg)*self.n_obs) + "\n"
        content = ""
        # Header
        content += f"imax {self.n_obs} number of channels\n"
        content += f"jmax {self.n_bkg} number of backgrounds\n"
        content += f"kmax * number of nuisance parameters\n"
        content += hline
        # Observed
        content += f"{'bin':<{hw+5}}"
        content +=  "".join([f"{bin_labels[i]:>{cw}}" for i in range(self.n_obs)])
        content +=  "\n"
        content += f"{'observation':<{hw+5}}"
        content +=  "".join([f"{n:{cw}d}" for n in self.obs])
        content +=  "\n"
        content += hline
        # Expected
        content += f"{'bin':<{hw+5}}"
        content += "".join([f"{bin_labels[i]:>{cw}}" for i in range(self.n_obs)])
        content += "".join([f"{bin_labels[i]:>{cw}}" for i in range(self.n_obs)])
        content +=  "\n"
        content += f"{'process':<{hw+5}}"
        content +=  "".join([f"{l:>{cw}}" for _ in range(self.n_obs) for l in self.bkg_labels])
        content +=  "".join([f"{l:>{cw}}" for _ in range(self.n_obs) for l in self.sig_labels])
        content +=  "\n"
        content += f"{'process':<{hw+5}}"
        content += "".join([f"{1:>{cw}}" for _ in range(self.n_obs)])
        content += "".join([f"{0:>{cw}}" for _ in range(self.n_obs)])
        content +=  "\n"
        content += f"{'rate':<{hw+5}}"
        content +=  "".join([f"{y[i]:{cw}.2f}" for y in self.bkg_yields for i in range(self.n_bkg)])
        content +=  "".join([f"{y[i]:{cw}.2f}" for y in self.sig_yields for i in range(self.n_sig)])
        content +=  "\n"
        content += hline
        # Systematics
        for syst_i, syst_values in enumerate(self.systs):
            content += f"{self.syst_labels[syst_i]:<{hw}}"
            content +=  " lnN "
            content +=  "".join(syst_values)
            content += "\n"
        content += hline
        # Rateparams
        content += f"{'a rateParam':<{hw}}  A  {self.bkg_labels[0]:>{cw}} (@0*@1/@2) b,c,d\n"
        content += f"{'b rateParam':<{hw}}  B  {self.bkg_labels[0]:>{cw}} {self.obs[1]}\n"
        content += f"{'c rateParam':<{hw}}  C  {self.bkg_labels[0]:>{cw}} {self.obs[2]}\n"
        content += f"{'d rateParam':<{hw}}  D  {self.bkg_labels[0]:>{cw}} {self.obs[3]}"
            
        self.content = content
