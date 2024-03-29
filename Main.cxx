#include <string>
#include <sys/stat.h>
#include <ctype.h>
#include "TFile.h"
#include "TH1D.h"
#include "FitTools.h"


using namespace std;

void FullFit(TH1* wiggle, int start_bin, int end_bin, TH1 *lm, string outputDir, string method,vector<double> init_values,vector<int> fit_chain, int attempts, map<int,double> fix_parameters, map<int,pair<double,double>> range_parameters) {
    int status = mkdir(outputDir.c_str(),0777);


    

    Fitter fitter;    
    fitter.SetMaxAttempts(attempts);
    fitter.SetOutputDir(outputDir);    
    fitter.SetFixParameters(fix_parameters);
    fitter.SetRangeParameters(range_parameters);

    double binW = wiggle->GetBinWidth(1);
    double start_time = start_bin * binW;
    double end_time   = end_bin * binW;

    cout << " Time range from " << start_time << " to " << end_time << endl;

    if(fabs(binW-0.1492)<0.0001) {           
        fitter.SetTimeUnit(Fitter::micro_second);        
        cout << "wiggle time unit: micro second"<<endl;
    }
    else if(fabs(binW-149.2)<0.0001) {
        fitter.SetTimeUnit(Fitter::nano_second);
        cout << "wiggle time unit: nano second"<<endl;
    }
    else {
        cout <<"unknown bin width = " << binW << endl;
        exit(1);
    }

    FitOutputInfo info;
    info.fit_values = init_values;
    for(int fit_mode : fit_chain){
        if(fit_mode == 5) info = fitter.Fit_5paras(method,wiggle,start_time,end_time,info.fit_values);
        if(fit_mode == 9) info = fitter.Fit_9paras_cbo(method,wiggle,start_time,end_time,info.fit_values);
        if(fit_mode == 10) info = fitter.Fit_10paras_cbo_lost(method,wiggle,start_time,end_time,info.fit_values,lm);
        
        //changing cbo mode
        if(fit_mode == 11) info = fitter.Fit_11paras_changing_cbo(method,wiggle,start_time,end_time,info.fit_values);
        if(fit_mode == 12) info = fitter.Fit_12paras_changing_cbo(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 13) info = fitter.Fit_13paras_cbo_vo(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 15) info = fitter.Fit_15paras_changing_cbo_vo(method,wiggle,start_time,end_time,info.fit_values,lm);



        if(fit_mode == 14) info = fitter.Fit_14paras_cbo_lost_vo(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 18) info = fitter.Fit_18paras_cbo_lost_vo_vw(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 22) info = fitter.Fit_22paras_cbo_lost_vw_expansion_lite(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 28) info = fitter.Fit_28paras_cbo_lost_vw_expansion(method,wiggle,start_time,end_time,info.fit_values,lm);

        //cbo syst
        if(fit_mode == 29) info = fitter.Fit_29paras_cbo_envelope_C(method,wiggle,start_time,end_time,info.fit_values,lm);

        if(fit_mode == 30) info = fitter.Fit_30paras_cbo_freq(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 31) info = fitter.Fit_31paras_cbo_time(method,wiggle,start_time,end_time,info.fit_values,lm);


        //simplified
        // if(fit_mode == 1024) info = fitter.Fit_run1_24paras(method,wiggle,start_time,end_time,info.fit_values,lm);

        //24 calorimeter fit
        if(fit_mode == 2400) info = fitter.Fit_calos_cbo(method,wiggle,start_time,end_time,info.fit_values,lm);

        if(fit_mode == 2401) info = fitter.Fit_31paras_calos_1(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 2402) info = fitter.Fit_31paras_calos_2(method,wiggle,start_time,end_time,info.fit_values,lm);

        //pseudo data fit cbo syst
        if(fit_mode == -5) info = fitter.Fit_pseudo_5pars(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == -10) info = fitter.Fit_pseudo_10pars(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == -9) info = fitter.Fit_pseudo_9pars(method,wiggle,start_time,end_time,info.fit_values,lm);

        if(fit_mode==2410) fitter.Fit_calos_cbo_envelope_exp_c(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode==2411) fitter.Fit_calos_cbo_envelope_generic(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode==2412) fitter.Fit_calos_cbo_envelope_poly(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode==2413) fitter.Fit_calos_cbo_envelope_alphaalpha(method,wiggle,start_time,end_time,info.fit_values,lm);

        if(fit_mode==2001) fitter.Fit_31paras_EU(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode==2002) fitter.Fit_28paras_EU0(method,wiggle,start_time,end_time,info.fit_values,lm);    

        if(fit_mode == 2901) info = fitter.Fit_SJTU_mod1(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 2902) info = fitter.Fit_SJTU_mod2(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 2903) info = fitter.Fit_SJTU_mod3(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 2904) info = fitter.Fit_SJTU_mod4(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 2905) info = fitter.Fit_SJTU_mod5(method,wiggle,start_time,end_time,info.fit_values,lm);
        if(fit_mode == 2906) info = fitter.Fit_SJTU_floatTauCBOVW(method,wiggle,start_time,end_time,info.fit_values,lm);

        if(fit_mode==3001) fitter.Fit_EU_nominal(method,wiggle,start_time,end_time,info.fit_values,lm);    
        if(fit_mode==3002) fitter.Fit_EU_fixTauCBOVW(method,wiggle,start_time,end_time,info.fit_values,lm);    
    }
}

// argv[1]: wiggle file
// argv[2]: wiggle name
// argv[3]: lm file
// argv[4]: lm name

// argv[5]: initial values json/root file
// argv[6]: initial values name

// argv[7]: output directory
// argv[8]: version
// argv[9]: fit mode

// argv[10]: maximum attempts

// argv[11]: time bin start
// argv[12]: time bin end

// argv[i] start with '--fix': fix parameters
// argv[i] start with '--range': range parameters
int main(int argc,char **argv) {
    cout << "file " << argv[1] << endl;
    TFile * file = TFile::Open(argv[1]);

    char *hname = argv[2];
    cout << "Using wiggle: " << hname << endl;
    TH1D * wiggle = (TH1D*) file->Get(hname)->Clone();

    TFile * file_lm = TFile::Open(argv[3]);
    TH1 * lm = (TH1*)file_lm->Get(argv[4]);

    cout << "Get initialValues from " << argv[5] << "  " << argv[6] << endl;
    vector<double> init_values = GetInitialValuesD(argv[5],argv[6]);    


    string outputDir(argv[7]);
    string name = Form("%s",argv[8]);

    int mode = atoi(argv[9]);
    cout << "mode " << mode << endl;
    vector<int> fit_chain;
    if(mode==0) fit_chain = {28};
    if(mode==1) fit_chain = {5,28};
    if(mode==2) fit_chain = {5};
    if(mode==3) fit_chain = {5,9,10};
    if(mode==4) fit_chain = {10};
    if(mode==5) fit_chain = {5,9,10,14,28};
    if(mode==6) fit_chain = {5,9,10,14,18};
    if(mode==7) fit_chain = {5,9,10,22};

    //frequency changing cbo
    if(mode==8) fit_chain = {5,9,10,12};
    if(mode==9) fit_chain = {5,9,11};
    if(mode==10) fit_chain = {5,9,13};
    if(mode==11) fit_chain = {5,9,13,15};

    if(mode==12) fit_chain = {29}; //envelope
    if(mode==13) fit_chain = {30}; //freq
    if(mode==14) fit_chain = {31}; //timing

    //cbo envelope mode exp+C for run3b
    if(mode==1000) fit_chain = {29}; 
    if(mode==1001) fit_chain = {5,29};

    if(mode==1002) fit_chain = {5,2901};
    if(mode==1003) fit_chain = {5,2902};
    if(mode==1004) fit_chain = {5,2903};
    if(mode==1005) fit_chain = {5,2904};
    if(mode==1006) fit_chain = {5,2905};

    //SJTU nominal 29 with floated tau_CBOVW
    if(mode==1007) fit_chain = {5,2906};

    //24 calorimeter fit

    if(mode==24) fit_chain = {2400};
    if(mode==2401) fit_chain = {28,2401};
    if(mode==2402) fit_chain = {28,2402};

    //pseudo data fit cbo syst
    if(mode==-2400) fit_chain = {-5,-10,-9};

    //sliding fit for  pseudo data
    if(mode==-2403) fit_chain = {-5,-9};

    //envelope models
    //exp+c
    if(mode==2410) fit_chain = {5,2410};
    //generic
    if(mode==2411) fit_chain = {5,2411};
    //poly
    if(mode==2412) fit_chain = {5,2412};
    //alphaalpha
    if(mode==2413) fit_chain = {5,2413};

    //swap with EU    
    if(mode==2001) fit_chain = {5,2001};
    if(mode==2002) fit_chain = {5,2002};
    if(mode==3001) fit_chain = {3001};
    if(mode==3002) fit_chain = {3002};


    int attempts;
    attempts = atoi(argv[10]);

    int start_bin,end_bin;

    start_bin = atoi(argv[11]);
    end_bin   = atoi(argv[12]);

    map<int,double> fix_parameters;
    map<int,pair<double,double>> range_parameters;
    int i=12;
    while(++i < argc) {        
        if (strcmp(argv[i], "--fix") == 0 && strcmp(argv[i+1], "None") != 0) {
            ++i;
            while (i < argc && isdigit(argv[i][0])) {                
                int npar = atoi(argv[i++]);
                char * s_fix_value = argv[i++];
                double fix_value;
                if(strcmp(s_fix_value, "nan") == 0) {
                    fix_value = init_values[npar];
                } else {                    
                    fix_value = atof(s_fix_value);
                }
                fix_parameters[npar] = fix_value;
                cout << "Fix parameter " << npar << " to " << fix_value << endl;
            }
        }        
        if (strcmp(argv[i], "--range") == 0 && strcmp(argv[i+1], "None") != 0) {
            ++i;
            while (i < argc && isdigit(argv[i][0])) {
                int npar = atoi(argv[i++]);
                auto range = make_pair<double, double>(0, 0);
                range.first = atof(argv[i++]);
                range.second = atof(argv[i++]);
                range_parameters[npar] = range;
                cout << "Set parameter " << npar << " range from " << range.first << " to " << range.second << endl;
            }            
        }        
    }    
    FullFit(wiggle,start_bin,end_bin,lm,outputDir,name,init_values,fit_chain,attempts,fix_parameters,range_parameters);
    return 0;
}