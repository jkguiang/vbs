#include "vbsvvhjets/collections.h"
// RAPIDO


#include "arbol.h"
#include "hepcli.h"
#include "looper.h"
#include "cutflow.h"
// ROOT
#include "TString.h"
#include "Math/VectorUtil.h"
#include "TH2.h"
#include "TH3.h"
// NanoCORE
#include "Nano.h"
#include "Config.h"
#include "tqdm.h"

#include "MCTools.h"






int main(int argc, char** argv)
{
    // CLI
    HEPCLI cli = HEPCLI(argc, argv);

    // Initialize Looper
    Looper looper = Looper(cli);

    // Initialize Arbol
    Arbol arbol = Arbol(cli);
    arbol.newBranch<double>("reweight_c2v_eq_3", -999);

    // Initialize Cutflow
    Cutflow cutflow = Cutflow(cli.output_name + "_Cutflow");

    // Pack above into VBSVVHJets struct (also adds branches)
    VBSVVHJets::Analysis analysis = VBSVVHJets::Analysis(arbol, nt, cli, cutflow);
    analysis.initBranches();
    // analysis.initCorrections();
    analysis.initCutflow();


    // gen level VVH
    arbol.newBranch<double>("gen_ld_v_ld_fermion_pt",-999);
    arbol.newBranch<double>("gen_ld_v_ld_fermion_eta",-999);
    arbol.newBranch<double>("gen_ld_v_ld_fermion_phi",-999);
    arbol.newBranch<double>("gen_ld_v_ld_fermion_mass",-999);
    arbol.newBranch<double>("gen_ld_v_tr_fermion_pt",-999);
    arbol.newBranch<double>("gen_ld_v_tr_fermion_eta",-999);
    arbol.newBranch<double>("gen_ld_v_tr_fermion_phi",-999);
    arbol.newBranch<double>("gen_ld_v_tr_fermion_mass",-999);
    arbol.newBranch<double>("gen_tr_v_ld_fermion_pt",-999);
    arbol.newBranch<double>("gen_tr_v_ld_fermion_eta",-999);
    arbol.newBranch<double>("gen_tr_v_ld_fermion_phi",-999);
    arbol.newBranch<double>("gen_tr_v_ld_fermion_mass",-999);
    arbol.newBranch<double>("gen_tr_v_tr_fermion_pt",-999);
    arbol.newBranch<double>("gen_tr_v_tr_fermion_eta",-999);
    arbol.newBranch<double>("gen_tr_v_tr_fermion_phi",-999);
    arbol.newBranch<double>("gen_tr_v_tr_fermion_mass",-999);

    arbol.newBranch<double>("gen_ld_v_ld_fermion_pdgId",-999);
    arbol.newBranch<double>("gen_ld_v_tr_fermion_pdgId",-999);
    arbol.newBranch<double>("gen_tr_v_ld_fermion_pdgId",-999);
    arbol.newBranch<double>("gen_tr_v_tr_fermion_pdgId",-999);

    // Initialize branches for leading and trailing h_bb
    arbol.newBranch<double>("gen_ld_h_bb_pt", -999);
    arbol.newBranch<double>("gen_ld_h_bb_eta", -999);
    arbol.newBranch<double>("gen_ld_h_bb_phi", -999);
    arbol.newBranch<double>("gen_ld_h_bb_mass", -999);
    arbol.newBranch<double>("gen_tr_h_bb_pt", -999);
    arbol.newBranch<double>("gen_tr_h_bb_eta", -999);
    arbol.newBranch<double>("gen_tr_h_bb_phi", -999);
    arbol.newBranch<double>("gen_tr_h_bb_mass", -999);

    // Initialize branches for leading and trailing h_non_bb
    arbol.newBranch<double>("gen_ld_h_non_bb_pt", -999);
    arbol.newBranch<double>("gen_ld_h_non_bb_eta", -999);
    arbol.newBranch<double>("gen_ld_h_non_bb_phi", -999);
    arbol.newBranch<double>("gen_ld_h_non_bb_mass", -999);
    arbol.newBranch<double>("gen_tr_h_non_bb_pt", -999);
    arbol.newBranch<double>("gen_tr_h_non_bb_eta", -999);
    arbol.newBranch<double>("gen_tr_h_non_bb_phi", -999);
    arbol.newBranch<double>("gen_tr_h_non_bb_mass", -999);

    // Initialize branches for pdgId
    arbol.newBranch<double>("gen_ld_h_bb_pdgId", -999);
    arbol.newBranch<double>("gen_tr_h_bb_pdgId", -999);
    arbol.newBranch<double>("gen_ld_h_non_bb_pdgId", -999);
    arbol.newBranch<double>("gen_tr_h_non_bb_pdgId", -999);



    // Initialize branches for vbf_quarks
    arbol.newBranch<double>("gen_ld_vbf_quarks_pt", -999);
    arbol.newBranch<double>("gen_ld_vbf_quarks_eta", -999);
    arbol.newBranch<double>("gen_ld_vbf_quarks_phi", -999);
    arbol.newBranch<double>("gen_ld_vbf_quarks_mass", -999);

    arbol.newBranch<double>("gen_tr_vbf_quarks_pt", -999);
    arbol.newBranch<double>("gen_tr_vbf_quarks_eta", -999);
    arbol.newBranch<double>("gen_tr_vbf_quarks_phi", -999);
    arbol.newBranch<double>("gen_tr_vbf_quarks_mass", -999);


    arbol.newBranch<double>("gen_ld_v_mass",-999);
    arbol.newBranch<double>("gen_ld_v_pt",-999);
    arbol.newBranch<double>("gen_ld_v_eta",-999);
    arbol.newBranch<double>("gen_ld_v_phi",-999);
    arbol.newBranch<double>("gen_tr_v_mass",-999);
    arbol.newBranch<double>("gen_tr_v_pt",-999);
    arbol.newBranch<double>("gen_tr_v_eta",-999);
    arbol.newBranch<double>("gen_tr_v_phi",-999);

    arbol.newBranch<double>("gen_h_bb_mass",-999);
    arbol.newBranch<double>("gen_h_bb_pt",-999);
    arbol.newBranch<double>("gen_h_bb_eta",-999);
    arbol.newBranch<double>("gen_h_bb_phi",-999);
    arbol.newBranch<double>("gen_h_non_bb_mass",-999);
    arbol.newBranch<double>("gen_h_non_bb_pt",-999);
    arbol.newBranch<double>("gen_h_non_bb_eta",-999);
    arbol.newBranch<double>("gen_h_non_bb_phi",-999);

    arbol.newBranch<double>("gen_tr_v_dR_fermmions",-999);
    arbol.newBranch<double>("gen_ld_v_dR_fermmions",-999);
    arbol.newBranch<double>("gen_h_bb_dR",-999);
    arbol.newBranch<double>("gen_h_non_bb_dR",-999);

    arbol.newBranch<double>("gen_vbf_quarks_deltaphi",-999);
    arbol.newBranch<double>("gen_vbf_quarks_deltaeta",-999);
    arbol.newBranch<double>("gen_vbf_quarks_dijetmass",-999);

    arbol.newBranch<int>("v1_pdgid",-999);
    arbol.newBranch<int>("v2_pdgid",-999);


    // We know that on gen level: element 0,1 are the incoming quarks and 2,3,4 are the VVH, and 5,6 are VBF quarks. we are gonna use that knowledge to get the VVH decay products and collect them using some if then statements
    //
    Cut* allVVH_products_identification_genlevel = new LambdaCut(
    	"AllVVH_products_identification_genlevel",
    	[&]()
    	{

    		std::vector<LorentzVector> vv_p4;
    		std::vector<LorentzVector> h_p4;
    		std::vector<LorentzVector> vbf_quarks_p4;
    		std::vector<int> vv_idx;
    		std::vector<int> vbf_quarks_idx;

            std::vector<int> vv_pdgid;


    		int h_idx=0;
    		for (unsigned int gen_i = 0; gen_i <nt.nGenPart() ; gen_i++)
    		{
    			int pdgId = nt.GenPart_pdgId().at(gen_i);
    			// analyze the vvh bosons products
    			if ( nt.GenPart_status().at(gen_i)==62)
    			// if ((gen_i== 19 || gen_i==20 || gen_i==21) && nt.GenPart_status().at(gen_i)==62)
    			{
    				if (abs(pdgId)==24 || abs(pdgId)==23)
    				{
    					vv_p4.push_back(nt.GenPart_p4().at(gen_i));
    					vv_idx.push_back(gen_i);
                        vv_pdgid.push_back(pdgId);
    				}

    				if (abs(pdgId)==25)
    				{
    					h_p4.push_back(nt.GenPart_p4().at(gen_i));
    					h_idx=gen_i;
    				}
    			}
    			// the vbf quarks
    			if (gen_i==5 || gen_i==6)
    			{
    				vbf_quarks_p4.push_back(nt.GenPart_p4().at(gen_i));
    				vbf_quarks_idx.push_back(gen_i);
    			}
    		}
    		// Error handling for vv_p4
    		if (vv_p4.empty() || vv_p4.size() < 2) {
    			// Handle error
    			std::cout << "vv_p4 size: " << vv_p4.size() << std::endl;

    			return false;
    		}
    		int ld_vv_idx = (vv_p4[0].pt() > vv_p4[1].pt()) ? vv_idx[0] : vv_idx[1];
    		int tr_vv_idx = (vv_p4[0].pt() > vv_p4[1].pt()) ? vv_idx[1] : vv_idx[0];
    		int ld_vbf_idx = (vbf_quarks_p4[0].pt() > vbf_quarks_p4[1].pt()) ? vbf_quarks_idx[0] : vbf_quarks_idx[1];
    		int tr_vbf_idx = (vbf_quarks_p4[0].pt() > vbf_quarks_p4[1].pt()) ? vbf_quarks_idx[1] : vbf_quarks_idx[0];
    // std::cout << "ld_vv_idx: " << ld_vv_idx << ", tr_vv_idx: " << tr_vv_idx << std::endl;

    		std::vector<LorentzVector> ld_vv_fermions_p4;
    		std::vector<LorentzVector> tr_vv_fermions_p4;
    		std::vector<int> ld_vv_fermions_pdgId;
    		std::vector<int> tr_vv_fermions_pdgId;
    		std::vector<LorentzVector> h_bb_p4;
    		std::vector<LorentzVector> h_non_bb_p4;
    		std::vector<int> h_bb_pdgId;
    		std::vector<int> h_non_bb_pdgId;


    		for (unsigned int gen_ii = 0; gen_ii < nt.nGenPart(); gen_ii++)
    		{
    			int pdgId = nt.GenPart_pdgId().at(gen_ii);


    // dumpGenParticleInfos();
    			if (ld_vv_idx==nt.GenPart_genPartIdxMother().at(gen_ii))
    			{
    				ld_vv_fermions_p4.push_back(nt.GenPart_p4().at(gen_ii));
    				ld_vv_fermions_pdgId.push_back(nt.GenPart_pdgId().at(gen_ii));
    				// std::cout << "gen_ii for " << gen_ii << endl;
    				// std::cout << "---------------------------------------------------------- " << endl;


    			}
    			else if (tr_vv_idx==nt.GenPart_genPartIdxMother().at(gen_ii))
    			{
    				tr_vv_fermions_p4.push_back(nt.GenPart_p4().at(gen_ii));
    				tr_vv_fermions_pdgId.push_back(nt.GenPart_pdgId().at(gen_ii));

    			}

    			if (h_idx==nt.GenPart_genPartIdxMother().at(gen_ii) && abs(pdgId)==5)
    			{
    				h_bb_p4.push_back(nt.GenPart_p4().at(gen_ii));
    				h_bb_pdgId.push_back(nt.GenPart_pdgId().at(gen_ii));

    			}
    			else if (h_idx==nt.GenPart_genPartIdxMother().at(gen_ii) && abs(pdgId)!=5)
    			{
    				h_non_bb_p4.push_back(nt.GenPart_p4().at(gen_ii));
    				h_non_bb_pdgId.push_back(nt.GenPart_pdgId().at(gen_ii));

    			}
    		}
    		// Error handling for ld_vv_fermions_p4
    		if (ld_vv_fermions_p4.empty() || ld_vv_fermions_p4.size() < 2) {
    			// Handle error
    			std::cout << "ld_vv_fermions_p4 size: " << ld_vv_fermions_p4.size() << std::endl;

    			return false;
    		}

    		// Error handling for tr_vv_fermions_p4
    		if (tr_vv_fermions_p4.empty() || tr_vv_fermions_p4.size() < 2) {
    			// Handle error
    			std::cout << "tr_vv_fermions_p4 size: " << tr_vv_fermions_p4.size() << std::endl;

    			return false;
    		}

    		// // Error handling for h_bb_p4
    		// if (h_bb_p4.empty() || h_bb_p4.size() < 2) {
    		//     // Handle error
    		//     std::cout << "h_bb_p4 size: " << h_bb_p4.size() << std::endl;
    		//
    		//     return false;
    		// }

    		// // Error handling for h_non_bb_p4
    		// if (h_non_bb_p4.empty() || h_non_bb_p4.size() < 2) {
    		//     // Handle error
    		//     std::cout << "h_non_bb_p4 size: " << h_non_bb_p4.size() << std::endl;
    		//
    		//     return false;
    		// }
    		if (ld_vv_fermions_p4.size()==2)
    		{
    			LorentzVector ld_v_ld_fermion = (ld_vv_fermions_p4[0].pt() > ld_vv_fermions_p4[1].pt()) ? ld_vv_fermions_p4[0] : ld_vv_fermions_p4[1] ; // leading V's leading fermion
    			LorentzVector ld_v_tr_fermion = (ld_vv_fermions_p4[0].pt() > ld_vv_fermions_p4[1].pt()) ? ld_vv_fermions_p4[1] : ld_vv_fermions_p4[0]; // leading V's trailing fermion
    			LorentzVector tr_v_ld_fermion = (tr_vv_fermions_p4[0].pt() > tr_vv_fermions_p4[1].pt()) ? tr_vv_fermions_p4[0] : tr_vv_fermions_p4[1]; // trailing V's leading fermion
    			LorentzVector tr_v_tr_fermion = (tr_vv_fermions_p4[0].pt() > tr_vv_fermions_p4[1].pt()) ? tr_vv_fermions_p4[1] : tr_vv_fermions_p4[0]; // trailing V's trailing fermion

    			// add the two fermions
    			LorentzVector ld_v = (ld_vv_fermions_p4[0] + ld_vv_fermions_p4[1]); //
    			LorentzVector tr_v = (tr_vv_fermions_p4[0] + tr_vv_fermions_p4[1]); //
    			double ld_v_dR_fermmions = ROOT::Math::VectorUtil::DeltaR(ld_vv_fermions_p4[0],ld_vv_fermions_p4[1]);
    			double tr_v_dR_fermmions = ROOT::Math::VectorUtil::DeltaR(tr_vv_fermions_p4[0],tr_vv_fermions_p4[1]);

    			int ld_v_ld_fermion_pdgId = (ld_vv_fermions_p4[0].pt() > ld_vv_fermions_p4[1].pt()) ? ld_vv_fermions_pdgId[0] : ld_vv_fermions_pdgId[1] ; // leading V's leading fermion;
    			int ld_v_tr_fermion_pdgId = (ld_vv_fermions_p4[0].pt() > ld_vv_fermions_p4[1].pt()) ? ld_vv_fermions_pdgId[1] : ld_vv_fermions_pdgId[0] ; // leading V's trailing fermion;
    			int tr_v_ld_fermion_pdgId = (tr_vv_fermions_p4[0].pt() > tr_vv_fermions_p4[1].pt()) ? tr_vv_fermions_pdgId[0] : tr_vv_fermions_pdgId[1] ; // trainling V's leading fermion;
    			int tr_v_tr_fermion_pdgId = (tr_vv_fermions_p4[0].pt() > tr_vv_fermions_p4[1].pt()) ? tr_vv_fermions_pdgId[1] : tr_vv_fermions_pdgId[0] ; // trainling V's trailing fermion;

    			// for the vv fermions
    			arbol.setLeaf<double>("gen_ld_v_ld_fermion_pt",ld_v_ld_fermion.pt());
    			arbol.setLeaf<double>("gen_ld_v_ld_fermion_eta",ld_v_ld_fermion.eta());
    			arbol.setLeaf<double>("gen_ld_v_ld_fermion_phi",ld_v_ld_fermion.phi());
    			arbol.setLeaf<double>("gen_ld_v_ld_fermion_mass",ld_v_ld_fermion.mass());
    			arbol.setLeaf<double>("gen_ld_v_tr_fermion_pt",ld_v_tr_fermion.pt());
    			arbol.setLeaf<double>("gen_ld_v_tr_fermion_eta",ld_v_tr_fermion.eta());
    			arbol.setLeaf<double>("gen_ld_v_tr_fermion_phi",ld_v_tr_fermion.phi());
    			arbol.setLeaf<double>("gen_ld_v_tr_fermion_mass",ld_v_tr_fermion.mass());
    			arbol.setLeaf<double>("gen_tr_v_ld_fermion_pt",tr_v_ld_fermion.pt());
    			arbol.setLeaf<double>("gen_tr_v_ld_fermion_eta",tr_v_ld_fermion.eta());
    			arbol.setLeaf<double>("gen_tr_v_ld_fermion_phi",tr_v_ld_fermion.phi());
    			arbol.setLeaf<double>("gen_tr_v_ld_fermion_mass",tr_v_ld_fermion.mass());
    			arbol.setLeaf<double>("gen_tr_v_tr_fermion_pt",tr_v_tr_fermion.pt());
    			arbol.setLeaf<double>("gen_tr_v_tr_fermion_eta",tr_v_tr_fermion.eta());
    			arbol.setLeaf<double>("gen_tr_v_tr_fermion_phi",tr_v_tr_fermion.phi());
    			arbol.setLeaf<double>("gen_tr_v_tr_fermion_mass",tr_v_tr_fermion.mass());

    			arbol.setLeaf<double>("gen_ld_v_ld_fermion_pdgId",ld_v_ld_fermion_pdgId);
    			arbol.setLeaf<double>("gen_ld_v_tr_fermion_pdgId",ld_v_tr_fermion_pdgId);
    			arbol.setLeaf<double>("gen_tr_v_ld_fermion_pdgId",tr_v_ld_fermion_pdgId);
    			arbol.setLeaf<double>("gen_tr_v_tr_fermion_pdgId",tr_v_tr_fermion_pdgId);

    			arbol.setLeaf<double>("gen_ld_v_mass",ld_v.mass());
    			arbol.setLeaf<double>("gen_ld_v_pt",ld_v.pt());
    			arbol.setLeaf<double>("gen_ld_v_eta",ld_v.eta());
    			arbol.setLeaf<double>("gen_ld_v_phi",ld_v.phi());
    			arbol.setLeaf<double>("gen_tr_v_mass",tr_v.mass());
    			arbol.setLeaf<double>("gen_tr_v_pt",tr_v.pt());
    			arbol.setLeaf<double>("gen_tr_v_eta",tr_v.eta());
    			arbol.setLeaf<double>("gen_tr_v_phi",tr_v.phi());

    			arbol.setLeaf<double>("gen_tr_v_dR_fermmions",tr_v_dR_fermmions);
    			arbol.setLeaf<double>("gen_ld_v_dR_fermmions",ld_v_dR_fermmions);


    		}
    		else{return false;}

    		if (h_bb_p4.size()==2) {
    			LorentzVector ld_h_bb = (h_bb_p4[0].pt() > h_bb_p4[1].pt()) ? h_bb_p4[0] : h_bb_p4[1] ; // leading bb quark for the h->bb
    			LorentzVector tr_h_bb = (h_bb_p4[0].pt() > h_bb_p4[1].pt()) ? h_bb_p4[1] : h_bb_p4[0] ; // trainling bb quark for the h->bb

    			//add the two decay particles of the higgs
    			LorentzVector h_bb = (h_bb_p4[0] + h_bb_p4[1]); //
    			double h_bb_dR = ROOT::Math::VectorUtil::DeltaR(h_bb_p4[0],h_bb_p4[1]);


    			int ld_h_bb_pdgId = (h_bb_p4[0].pt() > h_bb_p4[1].pt()) ? h_bb_pdgId[0] : h_bb_pdgId[1] ; // leading bb quark for the h->bb
    			int tr_h_bb_pdgId = (h_bb_p4[0].pt() > h_bb_p4[1].pt()) ? h_bb_pdgId[1] : h_bb_pdgId[0] ; // tr bb quark for the h->bb

    			// the h decay products
    			arbol.setLeaf<double>("gen_ld_h_bb_pt",ld_h_bb.pt());
    			arbol.setLeaf<double>("gen_ld_h_bb_eta",ld_h_bb.eta());
    			arbol.setLeaf<double>("gen_ld_h_bb_phi",ld_h_bb.phi());
    			arbol.setLeaf<double>("gen_ld_h_bb_mass",ld_h_bb.mass());
    			arbol.setLeaf<double>("gen_tr_h_bb_pt",tr_h_bb.pt());
    			arbol.setLeaf<double>("gen_tr_h_bb_eta",tr_h_bb.eta());
    			arbol.setLeaf<double>("gen_tr_h_bb_phi",tr_h_bb.phi());
    			arbol.setLeaf<double>("gen_tr_h_bb_mass",tr_h_bb.mass());

    			arbol.setLeaf<double>("gen_ld_h_bb_pdgId",ld_h_bb_pdgId);
    			arbol.setLeaf<double>("gen_tr_h_bb_pdgId",tr_h_bb_pdgId);

    			//add the two decay particles of the higgs
    			arbol.setLeaf<double>("gen_h_bb_mass",h_bb.mass());
    			arbol.setLeaf<double>("gen_h_bb_pt",h_bb.pt());
    			arbol.setLeaf<double>("gen_h_bb_eta",h_bb.eta());
    			arbol.setLeaf<double>("gen_h_bb_phi",h_bb.phi());

    			arbol.setLeaf<double>("gen_h_bb_dR",h_bb_dR);
    		}
    		if (h_non_bb_p4.size()==2) {
    			LorentzVector ld_h_non_bb = (h_non_bb_p4[0].pt() > h_non_bb_p4[1].pt()) ? h_non_bb_p4[0] : h_non_bb_p4[1] ; // leading bb quark for the h non bb
    			LorentzVector tr_h_non_bb = (h_non_bb_p4[0].pt() > h_non_bb_p4[1].pt()) ? h_non_bb_p4[1] : h_non_bb_p4[0] ; // trainling bb quark for the h non bb

    			int ld_h_non_bb_pdgId = (h_non_bb_p4[0].pt() > h_non_bb_p4[1].pt()) ? h_non_bb_pdgId[0] : h_non_bb_pdgId[1] ; // leading bb quark for the non h bb
    			int tr_h_non_bb_pdgId = (h_non_bb_p4[0].pt() > h_non_bb_p4[1].pt()) ? h_non_bb_pdgId[1] : h_non_bb_pdgId[0] ; // tr bb quark for the non h bb

    			LorentzVector h_non_bb = (h_non_bb_p4[0] + h_non_bb_p4[1]); //
    			double h_non_bb_dR = ROOT::Math::VectorUtil::DeltaR(h_non_bb_p4[0],h_non_bb_p4[1]);



    			arbol.setLeaf<double>("gen_ld_h_non_bb_pt",ld_h_non_bb.pt());
    			arbol.setLeaf<double>("gen_ld_h_non_bb_eta",ld_h_non_bb.eta());
    			arbol.setLeaf<double>("gen_ld_h_non_bb_phi",ld_h_non_bb.phi());
    			arbol.setLeaf<double>("gen_ld_h_non_bb_mass",ld_h_non_bb.mass());
    			arbol.setLeaf<double>("gen_tr_h_non_bb_pt",tr_h_non_bb.pt());
    			arbol.setLeaf<double>("gen_tr_h_non_bb_eta",tr_h_non_bb.eta());
    			arbol.setLeaf<double>("gen_tr_h_non_bb_phi",tr_h_non_bb.phi());
    			arbol.setLeaf<double>("gen_tr_h_non_bb_mass",tr_h_non_bb.mass());


    			arbol.setLeaf<double>("gen_ld_h_non_bb_pdgId",ld_h_non_bb_pdgId);
    			arbol.setLeaf<double>("gen_tr_h_non_bb_pdgId",tr_h_non_bb_pdgId);

    			//add the two decay particles of the higgs
    			arbol.setLeaf<double>("gen_h_non_bb_mass",h_non_bb.mass());
    			arbol.setLeaf<double>("gen_h_non_bb_pt",h_non_bb.pt());
    			arbol.setLeaf<double>("gen_h_non_bb_eta",h_non_bb.eta());
    			arbol.setLeaf<double>("gen_h_non_bb_phi",h_non_bb.phi());

    			arbol.setLeaf<double>("gen_h_non_bb_dR",h_non_bb_dR);


    		}




    		LorentzVector ld_vbf_quarks_p4 = (vbf_quarks_p4[0].pt() > vbf_quarks_p4[1].pt()) ? vbf_quarks_p4[0] : vbf_quarks_p4[1] ; // leading vbf quarks
    		LorentzVector tr_vbf_quarks_p4 = (vbf_quarks_p4[0].pt() > vbf_quarks_p4[1].pt()) ? vbf_quarks_p4[1] : vbf_quarks_p4[0] ; // trailing vbf quarks

    		LorentzVector vbf_quarks = (vbf_quarks_p4[0] + vbf_quarks_p4[1])  ;




    		// the two vbf quarks branches
    		arbol.setLeaf<double>("gen_ld_vbf_quarks_pt",ld_vbf_quarks_p4.pt());
    		arbol.setLeaf<double>("gen_ld_vbf_quarks_eta",ld_vbf_quarks_p4.eta());
    		arbol.setLeaf<double>("gen_ld_vbf_quarks_phi",ld_vbf_quarks_p4.phi());
    		arbol.setLeaf<double>("gen_ld_vbf_quarks_mass",ld_vbf_quarks_p4.mass());
    		arbol.setLeaf<double>("gen_tr_vbf_quarks_pt",tr_vbf_quarks_p4.pt());
    		arbol.setLeaf<double>("gen_tr_vbf_quarks_eta",tr_vbf_quarks_p4.eta());
    		arbol.setLeaf<double>("gen_tr_vbf_quarks_phi",tr_vbf_quarks_p4.phi());
    		arbol.setLeaf<double>("gen_tr_vbf_quarks_mass",tr_vbf_quarks_p4.mass());

    		// add the two vbf quarks
    		arbol.setLeaf<double>("gen_vbf_quarks_deltaphi",vbf_quarks_p4[0].phi()- vbf_quarks_p4[1].phi());
    		arbol.setLeaf<double>("gen_vbf_quarks_deltaeta",vbf_quarks_p4[0].eta()- vbf_quarks_p4[1].eta());
    		arbol.setLeaf<double>("gen_vbf_quarks_dijetmass",vbf_quarks.mass());

            // the pdgid branch to differntiate what is the process of the vvh: wzh wwh zzh oswwh
            arbol.setLeaf<int>("v1_pdgid",vv_pdgid[0]);
            arbol.setLeaf<int>("v2_pdgid",vv_pdgid[1]);


    		return true;
    	}
    );
    cutflow.insert("SemiMerged_SaveVariables", allVVH_products_identification_genlevel, Right);


 

    // Run looper
    tqdm bar;
    looper.run(
        [&](TTree* ttree)
        {
            nt.Init(ttree);
            analysis.init();
            TString file_name = cli.input_tchain->GetCurrentFile()->GetName();
        },
        [&](int entry)
        {
            if (cli.debug && looper.n_events_processed == 10000) { looper.stop(); }
            else
            {
                // Reset branches and globals
                arbol.resetBranches();
                cutflow.globals.resetVars();

                nt.GetEntry(entry);

                // Run cutflow
                std::vector<std::string> cuts_to_check = {
                    "SemiMerged_SaveVariables"
                };
                std::vector<bool> checkpoints = cutflow.run(cuts_to_check);
                if (checkpoints.at(0)) { arbol.fill(); }

                // Update progress bar
                bar.progress(looper.n_events_processed, looper.n_events_total);
            }
        }
    );

    // Wrap up
    if (!cli.is_data)
    {
        cutflow.print();
        cutflow.write(cli.output_dir);
    }
    arbol.write();
    return 0;
}
