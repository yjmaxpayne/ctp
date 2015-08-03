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

#include <votca/ctp/trajio.h>
#include <votca/ctp/trajiofactory.h>
#include "trajio/pdb.h"
#include "trajio/gro.h"
#include "trajio/xyz.h"

namespace votca { namespace ctp {

void trajIOFactory::RegisterAll(void)
{
    trajIOs().Register<pdbTrajIO> ("pdb");
    trajIOs().Register<groTrajIO> ("gro");
    trajIOs().Register<xyzTrajIO> ("xyz");
}

} /*namespace votca END */ } /* namespace ctp END */
