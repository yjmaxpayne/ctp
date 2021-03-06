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


#ifndef VOTCA_CTP_CTPAPPLICATION_H
#define	VOTCA_CTP_CTPAPPLICATION_H

#include <votca/tools/application.h>
#include <votca/tools/property.h>

namespace votca { namespace ctp {

using namespace std;

class CtpApplication : public votca::tools::Application
{
public:
    CtpApplication();
   ~CtpApplication() { };

   void Initialize();
   bool EvaluateOptions();
   virtual void Run(void) = 0;
   void ShowHelpText(std::ostream &out);

protected:

    votca::tools::Property _options;
    
};

}}









#endif /* _QMApplication_H */














