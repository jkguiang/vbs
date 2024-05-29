#ifndef ABCDNet_h
#define ABCDNet_h

#include "ABCDNetWeights.h"

namespace ABCDNet
{
    const float WP_SR = 0.89; // Working point for all-hadronic signal region

    float run(float hbbfatjet_pt, float hbbfatjet_eta, float hbbfatjet_phi, float hbbfatjet_mass,
              float ld_vqqfatjet_pt, float ld_vqqfatjet_eta, float ld_vqqfatjet_phi, float ld_vqqfatjet_mass,
              float tr_vqqfatjet_pt, float tr_vqqfatjet_eta, float tr_vqqfatjet_phi, float tr_vqqfatjet_mass,
              float M_jj)
    {

        // Build DNN input vector
        float x[13] = {
            log(hbbfatjet_pt),
            (hbbfatjet_eta + 2.5f)/(2.5f + 2.5f),
            (hbbfatjet_phi + 3.f)/(3.f + 3.f),
            hbbfatjet_mass/200.f,
            log(ld_vqqfatjet_pt),
            (ld_vqqfatjet_eta + 2.5f)/(2.5f + 2.5f),
            (ld_vqqfatjet_phi + 3.f)/(3.f + 3.f),
            ld_vqqfatjet_mass/200.f,
            log(tr_vqqfatjet_pt),
            (tr_vqqfatjet_eta + 2.5f)/(2.5f + 2.5f),
            (tr_vqqfatjet_phi + 3.f)/(3.f + 3.f),
            tr_vqqfatjet_mass/200.f,
            M_jj/3000.f,
        };

        // (0): Linear(in_features=13, out_features=64, bias=True) => x = x*W_T + b
        float x_0[64];
        for (unsigned int col = 0; col < 64; ++col)
        {
            x_0[col] = 0;
            for (unsigned int inner = 0; inner < 13; ++inner)
            {
                x_0[col] += x[inner]*ABCDNet::wgtT_0[inner][col];
            }
            x_0[col] += ABCDNet::bias_0[col];
        }
        
        // (1): LeakyReLU(negative_slope=0.01)
        float x_1[64];
        for (unsigned int col = 0; col < 64; ++col)
        {
            x_1[col] = (x_0[col] > 0.f) ? x_0[col] : 0.01f*x_0[col];
        }
        
        // (2): Linear(in_features=64, out_features=64, bias=True) => x = x*W_T + b
        float x_2[64];
        for (unsigned int col = 0; col < 64; ++col)
        {
            x_2[col] = 0;
            for (unsigned int inner = 0; inner < 64; ++inner)
            {
                x_2[col] += x_1[inner]*ABCDNet::wgtT_2[inner][col];
            }
            x_2[col] += ABCDNet::bias_2[col];
        }
        
        // (3): LeakyReLU(negative_slope=0.01)
        float x_3[64];
        for (unsigned int col = 0; col < 64; ++col)
        {
            x_3[col] = (x_2[col] > 0.f) ? x_2[col] : 0.01f*x_2[col];
        }
        
        // (4): Linear(in_features=64, out_features=64, bias=True) => x = x*W_T + b
        float x_4[64];
        for (unsigned int col = 0; col < 64; ++col)
        {
            x_4[col] = 0;
            for (unsigned int inner = 0; inner < 64; ++inner)
            {
                x_4[col] += x_3[inner]*ABCDNet::wgtT_4[inner][col];
            }
            x_4[col] += ABCDNet::bias_4[col];
        }
        
        // (5): LeakyReLU(negative_slope=0.01)
        float x_5[64];
        for (unsigned int col = 0; col < 64; ++col)
        {
            x_5[col] = (x_4[col] > 0.f) ? x_4[col] : 0.01f*x_4[col];
        }
        
        // (6): Linear(in_features=64, out_features=1, bias=True) => x = x*W_T + b
        float x_6[1];
        for (unsigned int col = 0; col < 1; ++col)
        {
            x_6[col] = 0;
            for (unsigned int inner = 0; inner < 64; ++inner)
            {
                x_6[col] += x_5[inner]*ABCDNet::wgtT_6[inner][col];
            }
            x_6[col] += ABCDNet::bias_6[col];
        }
        
        // (7): Sigmoid()
        float x_7[1];
        for (unsigned int col = 0; col < 1; ++col)
        {
            x_7[col] = exp(x_6[col])/(exp(x_6[col]) + 1);
        }
        
        return x_7[0];
    };
}

#endif
