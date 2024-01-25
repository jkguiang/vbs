import pandas as pd

class Systematic:
    def __init__(self, name, signal_regions):
        self.name = name
        self.signal_regions = signal_regions
        self.systs = {signal_region: [] for signal_region in signal_regions}
        
    def copy(self, name):
        new_systs = Systematic(name, signal_regions=self.signal_regions)
        new_systs.systs = self.systs
        return new_systs

    def add_syst(self, syst, signal_region):
        self.systs[signal_region].append(syst)
        
    def add_systs(self, systs, signal_region=None):
        if signal_region:
            self.systs[signal_region] += systs
        else:
            for syst_i, syst in enumerate(systs):
                self.systs[self.signal_regions[syst_i]].append(syst)
    
    def get_systs(self):
        return self.systs
    
    def get_systs_str(self, signal_region=None):
        if signal_region:
            systs = self.systs[signal_region]
            if len(systs) == 1:
                return f"{systs[0]:0.1%}"
            else:
                return f"{min(systs):0.1%} - {max(systs):0.1%}"
        else:
            return {SR: self.get_systs_str(signal_region=SR) for SR in self.signal_regions}
        
class SystematicsTable:
    def __init__(self, systs=None, samples=None):
        self.systs = systs or []
        self.samples = samples or []
        
    def add_row(self, syst):
        self.systs.append(syst)
        
    def to_dataframe(self, columns=None):
        rows = []
        for syst in self.systs:
            row = {"Systematic": syst.name}
            row.update(syst.get_systs_str())
            rows.append(row)
        df = pd.DataFrame(rows)
        if columns:
            columns.insert(0, "Systematic")
            return df[columns]
        else:
            return df
        
    def to_csv(self, columns=None, output_csv=None):
        df = self.to_dataframe(columns=columns)
        csv = df.to_csv(index=False)
        if output_csv:
            with open(output_csv, "w") as csv_out:
                csv.write(csv)
        else:
            return csv
        
    def to_latex(self, columns=None, output_tex=None):
        # Convert to Pandas DataFrame
        df = self.to_dataframe(columns=columns)
        
        # Convert to LaTeX
        latex = (df.style
                   .hide(axis="index")
                   .to_latex(column_format="lcc", position="H")
                   .replace("%", "\%"))
        # Insert hlines and centering
        latex = latex.split("\n")
        latex.insert(3, "\\hline")
        latex.insert(2, "\\hline\n\\hline")
        latex.insert(1, "\\begin{center}")
        latex.insert(-3, "\\hline\n\\hline")
        latex.insert(-2, "\\end{center}")
        latex = "\n".join(latex)
        
        if output_tex:
            with open(output_tex, "w") as tex_out:
                tex_out.write(latex)
        else:
            return latex
    
    def to_datacard_json(self, signal_regions=None, output_json=None):
        datacard_systs = {}
        for syst in self.systs:
            labeled_systs = syst.get_systs()
            datacard_systs[syst.name] = [1 + max(systs[SR]) for SR in signal_regions]
            
        if output_json:
            with open(output_json, "w") as json_out:
                json.dump(datacard_systs, json_out)
                
        return datacard_systs
