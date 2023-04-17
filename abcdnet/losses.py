import torch
import torch.nn as nn
import torch.nn.functional as F

class DisCoLoss(nn.Module):
    def __init__(self, disco_lambda, device=None):
        super().__init__()
        self.disco_lambda = disco_lambda

    @classmethod
    def from_config(cls, config):
        return cls(config.train.get("disco_lambda", 0))

    def dCorr(self, var_1, var_2, normed_weight, power=1):
        """
        Stolen from https://github.com/gkasieczka/DisCo/blob/master/Disco.py
        var_1: First variable to decorrelate (eg mass)
        var_2: Second variable to decorrelate (eg classifier output)
        normed_weight: Per-example weight. Sum of weights should add up to N (where N is the number of examples)
        power: Exponent used in calculating the distance correlation
        
        var_1, var_2 and normed_weight should all be 1D torch tensors with the same number of entries
        """
        
        xx = var_1.view(-1, 1).repeat(1, len(var_1)).view(len(var_1),len(var_1))
        yy = var_1.repeat(len(var_1),1).view(len(var_1),len(var_1))
        amat = (xx-yy).abs()

        xx = var_2.view(-1, 1).repeat(1, len(var_2)).view(len(var_2),len(var_2))
        yy = var_2.repeat(len(var_2),1).view(len(var_2),len(var_2))
        bmat = (xx-yy).abs()

        amatavg = torch.mean(amat*normed_weight,dim=1)
        Amat=amat-amatavg.repeat(len(var_1),1).view(len(var_1),len(var_1))\
            -amatavg.view(-1, 1).repeat(1, len(var_1)).view(len(var_1),len(var_1))\
            +torch.mean(amatavg*normed_weight)

        bmatavg = torch.mean(bmat*normed_weight,dim=1)
        Bmat=bmat-bmatavg.repeat(len(var_2),1).view(len(var_2),len(var_2))\
            -bmatavg.view(-1, 1).repeat(1, len(var_2)).view(len(var_2),len(var_2))\
            +torch.mean(bmatavg*normed_weight)

        ABavg = torch.mean(Amat*Bmat*normed_weight,dim=1)
        AAavg = torch.mean(Amat*Amat*normed_weight,dim=1)
        BBavg = torch.mean(Bmat*Bmat*normed_weight,dim=1)

        if power == 1:
            dCorr=(torch.mean(ABavg*normed_weight))/torch.sqrt((torch.mean(AAavg*normed_weight)*torch.mean(BBavg*normed_weight)))
        elif power == 2:
            dCorr=(torch.mean(ABavg*normed_weight))**2/(torch.mean(AAavg*normed_weight)*torch.mean(BBavg*normed_weight))
        else:
            dCorr=((torch.mean(ABavg*normed_weight))/torch.sqrt((torch.mean(AAavg*normed_weight)*torch.mean(BBavg*normed_weight))))**power
        
        return dCorr

    def forward(self, inferences, labels, disco_target, weights):
        BCE = F.binary_cross_entropy(inferences, labels, reduction="mean", weight=weights)
        dCorr2 = self.dCorr(inferences[labels == 0], disco_target[labels == 0], weights[labels == 0], power=2)
        return BCE + self.disco_lambda*dCorr2
