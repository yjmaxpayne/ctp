#ifndef VOTCA_CTP_POLARBACKGROUND_H
#define VOTCA_CTP_POLARBACKGROUND_H

#include <votca/ctp/topology.h>
#include <votca/ctp/polartop.h>
#include <votca/ctp/ewdspace.h>
#include <votca/ctp/ewaldactor.h>
#include <votca/ctp/logger.h>
#include <votca/ctp/qmthread.h>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace votca { namespace ctp {    
namespace EWD {

class PolarBackground
{
public:

    PolarBackground() : _top(NULL), _ptop(NULL), _log(NULL), _n_threads(1) {};
    PolarBackground(Topology *top, PolarTop *ptop, Property *opt, Logger *log, int n_threads);
   ~PolarBackground() {};
   
    void Threaded(int n_threads) { _n_threads = n_threads; }
    void Polarize();
    
    void FP_RealSpace();
    void FP_ReciprocalSpace();
    void FU_RealSpace(bool do_setup_nbs);
    void FU_ReciprocalSpace();
    
    void GenerateKVectors(vector<PolarSeg*> &ps1, vector<PolarSeg*> &ps2);
    
    // TODO Would be nicer to have as a local type in ::FP_RealSpace
    //      (which is possible by compiling with --std=c++0x)
    class FPThread : public QMThread
    {
    public:        
        FPThread(PolarBackground *master, int id);
       ~FPThread() { _full_bg_P.clear(); _part_bg_P.clear(); }       
        void Run(void);        
        void AddPolarSeg(PolarSeg *pseg) { _part_bg_P.push_back(pseg); return; }
        double Workload() { return 100.*_part_bg_P.size()/_full_bg_P.size(); }
        const int NotConverged() { return _not_converged_count; }
        double AvgRco() { return _avg_R_co; }
        void DoSetupNbs(bool do_setup) { _do_setup_nbs = do_setup; }
    private:        
        bool _verbose;
        PolarBackground *_master;
        EwdInteractor _ewdactor;
        vector<PolarSeg*> _full_bg_P;
        vector<PolarSeg*> _part_bg_P;
        int _not_converged_count;
        double _avg_R_co;
        bool _do_setup_nbs;
    };    
    
    class FUThread : public QMThread
    {
    public:        
        FUThread(PolarBackground *master, int id);
       ~FUThread() { _full_bg_P.clear(); _part_bg_P.clear(); }       
        void Run(void);        
        void AddPolarSeg(PolarSeg *pseg) { _part_bg_P.push_back(pseg); return; }        
        double Workload() { return 100.*_part_bg_P.size()/_full_bg_P.size(); }
        const int NotConverged() { return _not_converged_count; }
        double AvgRco() { return _avg_R_co; }
        void DoSetupNbs(bool do_setup) { _do_setup_nbs = do_setup; }
    private:
        bool _verbose;
        PolarBackground *_master;
        EwdInteractor _ewdactor;
        vector<PolarSeg*> _full_bg_P;
        vector<PolarSeg*> _part_bg_P;
        int _not_converged_count;
        double _avg_R_co;
        bool _do_setup_nbs;
    };
    
    class KThread : public QMThread
    {
    public:        
        
        KThread(PolarBackground *master) {
            _master = master;
            _mode_function["SFactorCalc"] = &KThread::SFactorCalc;
            _mode_function["KFieldCalc"] = &KThread::KFieldCalc;
            _full_bg_P = master->_bg_P;
            _ewdactor = EwdInteractor(_master->_alpha, _master->_polar_aDamp);
        }
       ~KThread() { 
           _full_bg_P.clear(); _full_kvecs.clear(); 
           _part_bg_P.clear(); _part_kvecs.clear();
        }
        
        typedef void (KThread::*RunFunct)();
        KThread *Clone() { return new KThread(_master); }        
        const string &getMode() { return _current_mode; }
        void setMode(const string &mode) { _current_mode = mode; }
        void setVerbose(bool verbose) { _verbose = verbose; }        
        void Run(void) { 
            RunFunct run = _mode_function[_current_mode]; 
            ((*this).*run)(); 
        }
        
        // MODE 1 : Compute structure factor of _part_kvecs using _full_bg_P
        void SFactorCalc();
        void AddInput(EWD::KVector k) { _part_kvecs.push_back(k); }
        
        // MODE 2 : Increment fields of _part_bg_P using _full_kvecs
        void KFieldCalc();
        void AddInput(PolarSeg *pseg) { _part_bg_P.push_back(pseg); }
        
    private:
        // Thread administration
        bool _verbose;
        string _current_mode;
        map<string,RunFunct> _mode_function;
        
        // Shared thread data
        PolarBackground *_master;
        vector<PolarSeg*> _full_bg_P;
        vector<EWD::KVector> _full_kvecs;
        
        // Personal thread data
        EwdInteractor _ewdactor;
        vector<EWD::KVector> _part_kvecs;
        vector<PolarSeg*> _part_bg_P;
    };
    
    
private:

    EwdInteractor _ewdactor;
    Logger *_log;
    int _n_threads;

    // PERIODIC BOUNDARY
    Topology *_top;
    string _shape;

    // POLAR SEGMENTS
    // Part I - Ewald
    PolarTop *_ptop;
    vector< PolarSeg* > _bg_P;      // Period. density = _bg_N v _fg_N

    // CONVERGENCE
    // Part I - Ewald
    double _alpha;                  // _a = 1/(sqrt(2)*sigma)
    double _kfactor;
    double _rfactor;
    double _K_co;                   // k-space c/o
    double _R_co;                   // r-space c/o
    double _crit_dE;                // Energy convergence criterion [eV]
    bool   _converged_R;            // Did R-space sum converge?
    bool   _converged_K;            // Did K-space sum converge?
    bool   _field_converged_R;
    bool   _field_converged_K;
    // Part II - Thole
    double _polar_aDamp;
    double _polar_wSOR_N;
    double _polar_cutoff;

    // LATTICE (REAL, RECIPROCAL)
    vec _a; vec _b; vec _c;         // Real-space lattice vectors
    int _na_max, _nb_max, _nc_max;  // Max. cell indices to sum over (R)
    vec _A; vec _B; vec _C;         // Reciprocal-space lattice vectors
    int _NA_max, _NB_max, _NC_max;  // Max. cell indices to sum over (K)
    double _LxLy;                   // |a^b|
    double _LxLyLz;                 // a*|b^c|

    EWD::VectorSort<EWD::MaxNorm,vec> _maxsort;
    EWD::VectorSort<EWD::EucNorm,vec> _eucsort;
    EWD::VectorSort<EWD::KNorm,EWD::KVector> _kvecsort;

    vector<EWD::KVector> _kvecs_2_0;     // K-vectors with two components = zero
    vector<EWD::KVector> _kvecs_1_0;     // K-vectors with one component  = zero
    vector<EWD::KVector> _kvecs_0_0;     // K-vectors with no  components = zero
    double _kxyz_s1s2_norm;

};


template
<
    // REQUIRED Clone()
    // OPTIONAL 
    typename T
>
class PrototypeCreator
{
public:
    PrototypeCreator(T *prototype=0) : _prototype(prototype) {}
    T *Create() { return (_prototype) ? _prototype->Clone() : 0; }
    T *getPrototype() { return _prototype; }
    void setPrototype(T *prototype) { _prototype = prototype; }
private:
    T *_prototype;
};


template
<
    // REQUIRED setId(int), Start, WaitDone
    // OPTIONAL setMode(<>), AddInput(<>), setVerbose(bool)
    typename ThreadType,
    // REQUIRED Create
    // OPTIONAL
    template<typename> class CreatorType
>
class ThreadForce : 
    public vector<ThreadType*>, 
    public CreatorType<ThreadType>
{
public:
    typedef typename ThreadForce::iterator it_t;
    
    ThreadForce() {}
   ~ThreadForce() { Disband(); }
    
    void Initialize(int n_threads) {
        // Set-up threads via CreatorType policy
        for (int t = 0; t < n_threads; ++t) {
            ThreadType *new_thread = this->Create();
            new_thread->setId(this->size()+1);
            this->push_back(new_thread);
        }
        return;
    }
    
    template<typename mode_t>
    void AssignMode(mode_t mode) {
        // Set mode
        it_t tfit;
        for (tfit = this->begin(); tfit < this->end(); ++tfit)
           (*tfit)->setMode(mode);
        return;
    }
    
//    template<template<typename> class cnt_t, typename in_t>
//    void ApportionInput(cnt_t<in_t> inputs) {
//        // Assign input to threads
//        typename cnt_t<in_t>::iterator iit;
//        int in_idx = 0;
//        int th_idx = 0;
//        for (iit = inputs.begin(); iit != inputs.end(); ++iit, ++in_idx) {
//            int th_idx = in_idx % this->size();
//            (*this)[th_idx]->AddInput(*iit);
//        }
//        // The thread with the last input can be verbose
//        for (it_t tfit = this->begin(); tfit < this->end(); ++tfit)
//            (*tfit)->setVerbose(false);
//        (*this)[th_idx]->setVerbose(true);
//        return;
//    }
    
    template<class in_t>
    void ApportionInput(vector<in_t> &inputs) {
        // Assign input to threads
        typename vector<in_t>::iterator iit;
        int in_idx = 0;
        int th_idx = 0;
        for (iit = inputs.begin(); iit != inputs.end(); ++iit, ++in_idx) {
            int th_idx = in_idx % this->size();
            (*this)[th_idx]->AddInput(*iit);
        }
        // The thread with the last input can be verbose
        for (it_t tfit = this->begin(); tfit < this->end(); ++tfit)
            (*tfit)->setVerbose(false);
        (*this)[th_idx]->setVerbose(true);
        return;
    }
    
    
    void StartAndWait() {
        // Start threads
        for (it_t tfit = this->begin(); tfit < this->end(); ++tfit)
            (*tfit)->Start();
        // Wait for threads
        for (it_t tfit = this->begin(); tfit < this->end(); ++tfit)
            (*tfit)->WaitDone();
        return;
    }
    
    void Disband() {
        // Delete & clear
        it_t tfit;
        for (tfit = this->begin(); tfit < this->end(); ++tfit)
            delete *tfit;
        this->clear();
        return;
    }
    
private:
    
};


    
    
}
}}

#endif