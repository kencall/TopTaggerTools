#include "Eff_FakeRatePlots.h"

#include "TROOT.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TPaveStats.h"
#include <TLegend.h>
#include <TCanvas.h>
#include "TF1.h"
#include "TSystem.h"

#include <iostream>
#include <string>

#include <cstdlib>
#include <cstdio>
#include <cstring>

class TaggerInfo
{
public:
    std::string inputFile;
    std::string outputFile;
    std::string legName;
};

void makePlots(const std::string& outFile, const std::string& name, TH1* simpleHist, TH1* mediumHist,
               const std::string& dataSet, const std::string& simpleLeg = "Simple   Top Tagger", const std::string& mediumLeg = "Medium Top Tagger")
{
    //Define canvas and legend
    TCanvas *c = new TCanvas( (dataSet+name).c_str(),(dataSet+name).c_str(),1000,800);  
    TLegend *l = new TLegend(0.68, 0.8, 0.99, 0.9);
    double topM,     bottomM,      rightM,      leftM;
    /*  */ topM=0.1; bottomM=0.35; rightM=0.05; leftM=0.16;
    gPad->SetTopMargin(   0.0);
    gPad->SetBottomMargin(0.0);
    gPad->SetRightMargin( 0.0);
    gPad->SetLeftMargin(  0.0);
    //gPad->SetTopMargin(topM);
    //gPad->SetBottomMargin(bottomM);
    //gPad->SetRightMargin(rightM);
    //gPad->SetLeftMargin(leftM);

    //Define the top and bottom TPad
    double up_height     = 0.75;  // please tune so that the upper figures size will meet your requirement
    double dw_correction = 1.0;   //40;
    double dw_height     = (1.0 - up_height)*dw_correction;
    TPad *pad_up = new TPad("pad_up1","pad_up1",0.0, 1.0 - up_height, 1.0,       1.0);
    TPad *pad_dw = new TPad("pad_dw1","pad_dw1",0.0, 0.0,             1.0, dw_height);
    pad_up->Draw();
    pad_dw->Draw();
  
    pad_up->SetFrameFillColor(0);
    pad_up->SetFillColor(0);
    pad_up->SetTopMargin(topM);
    pad_up->SetBottomMargin(0.02);
    pad_up->SetLeftMargin(leftM);
    pad_up->SetRightMargin(rightM);

    pad_dw->SetFrameFillColor(0);
    pad_dw->SetFillColor(0);
    pad_dw->SetTopMargin(0.04);
    pad_dw->SetBottomMargin(bottomM);
    pad_dw->SetLeftMargin(leftM);
    pad_dw->SetRightMargin(rightM);

    //Top TPad
    pad_up->cd();
    
    double max = std::max( mediumHist->GetMaximum(), simpleHist->GetMaximum() );
    mediumHist->SetMaximum( 1.2*max );
    mediumHist->SetMinimum(0);
    mediumHist->SetTitleSize(0.002);
    mediumHist->SetTitleSize(0.05,"X");
    mediumHist->SetTitleSize(0.05,"Y");
    mediumHist->SetTitleOffset(1.2,"X");
    mediumHist->SetTitleOffset(1.5,"Y");
    mediumHist->SetLabelSize(0.0,"X");
    //mediumHist->SetLabelSize(0.05,"X");
    mediumHist->SetLabelSize(0.05,"Y");
    //mediumHist->SetNdivisions(0,"X");
    mediumHist->SetStats(false);    
    mediumHist->SetLineColor(kRed);
    mediumHist->Draw("hist E");

    simpleHist->SetLineColor(kBlack);
    simpleHist->Draw("hist E same");

    l->SetBorderSize(0);
    l->SetFillStyle(0);
    l->SetTextSize(0.03);    
    l->AddEntry(mediumHist, (mediumLeg).c_str()   , "l");
    l->AddEntry(simpleHist, (simpleLeg).c_str()   , "l");
    l->Draw();

    //Bottom TPad
    pad_dw->cd();

    Eff_FakeRatePlots dummy;
    TH1F *ratio = (TH1F*)mediumHist->Clone( (name+dataSet+"Ratio").c_str() );;
    for(int i=0; i < ratio->GetSize(); i++)
    {
        double errorNum = dummy.errorRatio(i,mediumHist,simpleHist);
        double ratioNum = mediumHist->GetBinContent(i)/simpleHist->GetBinContent(i);      
        if(isinf(ratioNum)==1 || isnan(ratioNum)==1) ratioNum = 0;
        ratio->SetBinContent(i,ratioNum);
        ratio->SetBinError(i,errorNum);
    }

    double maxDw = ratio->GetMaximum();
    double minDw = ratio->GetMinimum();

    ratio->SetTitle("");
    ratio->SetStats(false);
    ratio->SetMaximum( 1.5 );
    ratio->SetMinimum( 0.5 );
    //ratio->SetMaximum( std::max(1.5*maxDw, 1.3) );
    //ratio->SetMinimum( std::min(0.5*maxDw, 0.7) );
    ratio->SetTitleOffset(1.0,"X");
    ratio->SetTitleOffset(0.5,"Y");
    ratio->SetTitleSize(0.15,"X");
    ratio->SetTitleSize(0.15,"Y");
    ratio->SetLabelSize(0.15,"X");
    ratio->SetLabelSize(0.15,"Y");
    ratio->SetNdivisions(10,"X");
    ratio->SetNdivisions( 5,"Y");
    ratio->GetYaxis()->SetTitle("Ratio");
    ratio->SetLineColor(kBlack);
    ratio->Draw("E1");

    TF1 *line = new TF1( (name+dataSet+"Line").c_str(),"1",-2000,2000);
    line->SetLineColor(kRed);
    line->Draw("same");

    //std::cout<<"plots/" + dataSet + outFile<<std::endl;
    gSystem -> Exec( ("mkdir -p plots/" + dataSet + outFile).c_str() ) ;    
    c->SaveAs( ( "plots/" + dataSet + name + ".png" ).c_str() );        

    delete l;
    delete pad_up;
    delete pad_dw;
    delete line;
}

void runPlotter(const std::vector<TaggerInfo>& taggerInfo, const std::vector<std::string>& selections, const std::string& dataSet)
{
    char copy[128];
    strcpy(copy, taggerInfo[0].inputFile.c_str() );    
    char* type1;
    type1 = strtok( copy, "-/" );
    char* type2;
    type2 = strtok( nullptr, "-/" );
    char* type3;
    type3 = strtok( nullptr, "-/" );

    /////////////////////////////////
    ////   Looping over TaggerInfo
    /////////////////////////////////
    std::vector<Eff_FakeRatePlots*> vecPlots;
    for(const auto& tI : taggerInfo)
    {
        Eff_FakeRatePlots* fakeratePlots = new Eff_FakeRatePlots();
        for(const auto& s : selections)
        {
            fakeratePlots->makeTH1F(s, tI.inputFile.c_str(), type3);
        }

        TFile *f = new TFile(tI.outputFile.c_str(),"RECREATE");
        if(f->IsZombie())
        {
            std::cout << "Cannot create " << tI.outputFile << std::endl;
            throw "File is zombie";
        }
    
        f->cd();
    
        for(int i = 0; i < fakeratePlots->histos_.size(); i++)
        {
            fakeratePlots->histos_[i]->Write();
        }
    
        f->Write();
        f->Close();

        vecPlots.push_back(fakeratePlots);
    }

    ////////////////////////////////
    //       Making Plots
    ////////////////////////////////
    for(int i = 0; i < vecPlots[0]->histos_.size(); i++)
    {
        makePlots(vecPlots[0]->outFile_[i], vecPlots[0]->histoName_[i], vecPlots[0]->histos_[i], vecPlots[1]->histos_[i], dataSet, taggerInfo[0].legName, taggerInfo[1].legName);
    }

    //Cleaning up
    for(auto p : vecPlots)
    {
        delete p;
    }
}

int main()
{
    TH1::AddDirectory(false);

    ////Orginal cuts for Simple and Medium Top Tagger
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbar-2018-3-9.root", "outputRoot/efficiencyandFakeRatePlots_TT_TTbar_simpleTopTagger.root",
    //           "joesGroup/MediumHaddFiles/TT_TTbar-2018-3-9.root", "outputRoot/efficiencyandFakeRatePlots_TT_TTbar_mediumTopTagger.root",
    //           "originalDiscCuts/"
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_QCD-2018-3-9.root", "outputRoot/efficiencyandFakeRatePlots_TT_QCD_simpleTopTagger.root",
    //           "joesGroup/MediumHaddFiles/TT_QCD-2018-3-9.root", "outputRoot/efficiencyandFakeRatePlots_TT_QCD_mediumTopTagger.root",
    //           "originalDiscCuts/"               
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbarSingleLep-2018-3-9.root", "outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_simpleTopTagger.root",
    //           "joesGroup/MediumHaddFiles/TT_TTbarSingleLep-2018-3-9.root", "outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_mediumTopTagger.root",
    //           "originalDiscCuts/"               
    //          );
    //
    //
    ////95max_00017pt85 Medium Top Tagger cuts
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbar-2018-3-9.root"                 ,"outputRoot/efficiencyandFakeRatePlots_TT_TTbar_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_00017pt85/TT_TTbar-2018-3-13.root","outputRoot/efficiencyandFakeRatePlots_TT_TTbar_95max_00017pt85_medium.root",
    //           "95max_00017pt85_medium/"
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_QCD-2018-3-9.root"                 ,"outputRoot/efficiencyandFakeRatePlots_TT_QCD_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_00017pt85/TT_QCD-2018-3-13.root","outputRoot/efficiencyandFakeRatePlots_TT_QCD_95max_00017pt85_medium.root",
    //           "95max_00017pt85_medium/"
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbarSingleLep-2018-3-9.root"                 ,"outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_00017pt85/TT_TTbarSingleLep-2018-3-13.root","outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_95max_00017pt85_medium.root",
    //           "95max_00017pt85_medium/"
    //          );
    //
    //
    ////95max_0004375pt775 Medium Top Tagger cuts
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbar-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_TTbar_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_0004375pt775/TT_TTbar-2018-3-12.root","outputRoot/efficiencyandFakeRatePlots_TT_TTbar_95max_0004375pt775_medium.root",
    //           "95max_0004375pt775_medium/"
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_QCD-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_QCD_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_0004375pt775/TT_QCD-2018-3-12.root","outputRoot/efficiencyandFakeRatePlots_TT_QCD_95max_0004375pt775_medium.root",
    //           "95max_0004375pt775_medium/"
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbarSingleLep-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_0004375pt775/TT_TTbarSingleLep-2018-3-12.root","outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_95max_0004375pt775_medium.root",
    //           "95max_0004375pt775_medium/"
    //          );
    //
    ////95max_0005pt7 Medium Top Tagger cuts
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbar-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_TTbar_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_0005pt7/TT_TTbar-2018-3-12.root","outputRoot/efficiencyandFakeRatePlots_TT_TTbar_95max_0005pt7_medium.root",
    //           "95max_0005pt7_medium/"
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_QCD-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_QCD_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_0005pt7/TT_QCD-2018-3-12.root","outputRoot/efficiencyandFakeRatePlots_TT_QCD_95max_0005pt7_medium.root",
    //           "95max_0005pt7_medium/"
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbarSingleLep-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_0005pt7/TT_TTbarSingleLep-2018-3-12.root","outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_95max_0005pt7_medium.root",
    //           "95max_0005pt7_medium/"
    //          );
    //
    ////95max_00175pt3125 Medium Top Tagger cuts
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbar-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_TTbar_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_00175pt3125/TT_TTbar-2018-3-13.root","outputRoot/efficiencyandFakeRatePlots_TT_TTbar_95max_00175pt3125_medium.root",
    //           "95max_00175pt3125_medium/"
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_QCD-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_QCD_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_00175pt3125/TT_QCD-2018-3-13.root","outputRoot/efficiencyandFakeRatePlots_TT_QCD_95max_00175pt3125_medium.root",
    //           "95max_00175pt3125_medium/"
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbarSingleLep-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_95max_00175pt3125/TT_TTbarSingleLep-2018-3-13.root","outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_95max_00175pt3125_medium.root",
    //           "95max_00175pt3125_medium/"
    //          );    
    //
    ////93max_000575pt7 Medium Top Tagger cuts
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbar-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_TTbar_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_93max_000575pt7/TT_TTbar-2018-3-13.root","outputRoot/efficiencyandFakeRatePlots_TT_TTbar_93max_000575pt7_medium.root",
    //           "93max_000575pt7_medium/"
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_QCD-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_QCD_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_93max_000575pt7/TT_QCD-2018-3-13.root","outputRoot/efficiencyandFakeRatePlots_TT_QCD_93max_000575pt7_medium.root",
    //           "93max_000575pt7_medium/"
    //          );
    //runPlotter("joesGroup/SimpleHaddFiles/TT_TTbarSingleLep-2018-3-9.root"                    ,"outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_simpleTopTagger.root",
    //           "joesGroup/mediumTopTagger_93max_000575pt7/TT_TTbarSingleLep-2018-3-13.root","outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_93max_000575pt7_medium.root",
    //           "93max_000575pt7_medium/"
    //          );    
    //
    ////Comparing Joes nTuples to ours Medium TopTagger discCut = 0.95 discSlope = 0.0004375 discOffset = 0.775
    //runPlotter("joesGroup/mediumTopTagger_95max_0004375pt775/TT_TTbar-2018-3-12.root","outputRoot/efficiencyandFakeRatePlots_TT_TTbar_95max_0004375pt775_medium.root",
    //           "stealthGroup/TT/stealth_TT_mediumTT.root","outputRoot/efficiencyandFakeRatePlots_TT_Stealth_95max_0004375pt775_medium.root",
    //           "95max_0004375pt775_medium/"
    //          );
    //runPlotter("joesGroup/mediumTopTagger_95max_0004375pt775/TT_QCD-2018-3-12.root","outputRoot/efficiencyandFakeRatePlots_TT_QCD_95max_0004375pt775_medium.root",
    //           "stealthGroup/QCD/stealth_QCD_mediumTT.root","outputRoot/efficiencyandFakeRatePlots_QCD_Stealth_95max_0004375pt775_medium.root",
    //           "95max_0004375pt775_medium/"
    //          );


    //Comparing joes test top tagger to the full top tagger: WP 0.7
    std::vector<std::string> selections = {"Lep0/", "Lep1/", "QCD/", "QCDb/"};
    std::vector<TaggerInfo> joeTest_TTbarSingleLepT
    {
        {"joesGroup/fullTopTagger_0.7WP/TT_TTbarSingleLepT-2018-4-22.root"   , "outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLep_fullTopTagger_0.7WP.root"    , "Full TT 0.7 WP"},
        {"joesGroup/joeTestTopTagger_0.7WP/TT_TTbarSingleLepT-2018-4-22.root", "outputRoot/efficiencyandFakeRatePlots_TT_TTbarSingleLepT_joeTestTopTagger_0.7WP.root", "JoeTest TT 0.7 WP"},
    };

    std::vector<TaggerInfo> joeTest_QCD
    {
        {"joesGroup/fullTopTagger_0.7WP/TT_QCD-2018-4-22.root"   , "outputRoot/efficiencyandFakeRatePlots_TT_QCD_fullTopTagger_0.7WP.root"   , "Full TT 0.7 WP"},
        {"joesGroup/joeTestTopTagger_0.7WP/TT_QCD-2018-4-22.root", "outputRoot/efficiencyandFakeRatePlots_TT_QCD_joeTestTopTagger_0.7WP.root", "JoeTest TT 0.7 WP"},
    };

    runPlotter(joeTest_TTbarSingleLepT, selections, "0.7WP_joeTest_test/");
    runPlotter(joeTest_QCD, selections, "0.7WP_joeTest_test/");
}
