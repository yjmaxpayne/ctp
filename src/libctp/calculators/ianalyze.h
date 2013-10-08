/*
 *            Copyright 2009-2012 The VOTCA Development Team
 *                       (http://www.votca.org)
 *
 *      Licensed under the Apache License, Version 2.0 (the "License")
 *
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef VOTCA_CTP_IANALYZE_H
#define VOTCA_CTP_IANALYZE_H

#include <votca/ctp/qmcalculator.h>
#include <math.h>


namespace votca { namespace ctp {

class IAnalyze : public QMCalculator
{
public:

    string  Identify() { return "ianalyze"; }

    void    Initialize(Property *options);
    bool    EvaluateFrame(Topology *top);
    void    IHist(Topology *top, int state);

private:

    double      _resolution_logJ2;
    vector<int> _states;

};


void IAnalyze::Initialize(Property *opt) {

    // _options already has default values, update them with the supplied options
    _options.CopyValues("", *opt );
    string key = "options." + Identify();
 
    _resolution_logJ2 = _options.get(key+".resolution_logJ2").as< double >();
    _states = _options.get(key+".states").as< vector<int> >();
}


bool IAnalyze::EvaluateFrame(Topology *top) {

    QMNBList &nblist = top->NBList();

    if (!nblist.size()) {
        cout << endl << "... ... No pairs in topology. Skip...";
        return 0;
    }

    for (int i = 0; i < _states.size(); ++i) {
        this->IHist(top, _states[i]);
    }
    
    return true;
}


void IAnalyze::IHist(Topology *top, int state) {

    QMNBList &nblist = top->NBList();
    QMNBList::iterator nit;

    double MIN = log10(nblist.front()->getJeff2(state));
    double MAX = log10(nblist.front()->getJeff2(state));

    // Collect J2s from pairs
    vector< double > J2s;
    J2s.reserve(nblist.size());

    for (nit = nblist.begin(); nit != nblist.end(); ++nit) {
        double J2 = log10((*nit)->getJeff2(state));

        MIN = (J2 < MIN) ? J2 : MIN;
        MAX = (J2 > MAX) ? J2 : MAX;

        J2s.push_back(J2);
    }

    // Prepare bins
    int BIN = ( (MAX-MIN)/_resolution_logJ2 + 0.5 ) + 1;
    vector< vector<double> > histJ2;
    histJ2.resize(BIN);

    // Execute binning
    vector< double > ::iterator jit;
    for (jit = J2s.begin(); jit < J2s.end(); ++jit) {

        int bin = int( (*jit-MIN)/_resolution_logJ2 + 0.5 );
        histJ2[bin].push_back(*jit);
    }

    vector< int > histN;
    histN.resize(BIN);
    for (int bin = 0; bin < BIN; ++bin) {
        histN[bin] = histJ2[bin].size();
    }
    FILE *out;
    string tag = boost::lexical_cast<string>("ianalyze.ihist_") + ( (state == -1) ? "e" : "h" ) + ".out";
    out = fopen(tag.c_str(), "w");

    fprintf(out, "# IANALYZE: PAIR-INTEGRAL J2 HISTOGRAM\n");
    fprintf(out, "# STATE %1d\n", state);

    for (int bin = 0; bin < BIN; ++bin) {
        double J2 = MIN + bin*_resolution_logJ2;
        fprintf(out, "%4.7f %4d \n", J2, histN[bin]);
    }
    fclose(out);
}



}}



#endif
