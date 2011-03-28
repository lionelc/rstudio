/*
 * SessionWorkspace.cpp
 *
 * Copyright (C) 2009-11 by RStudio, Inc.
 *
 * This program is licensed to you under the terms of version 3 of the
 * GNU Affero General Public License. This program is distributed WITHOUT
 * ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
 * AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
 *
 */

#include "SessionWorkspace.hpp"

#include <boost/bind.hpp>

#include <core/Error.hpp>
#include <core/Log.hpp>
#include <core/Exec.hpp>

#include <core/json/JsonRpc.hpp>

#include <r/RExec.hpp>
#include <r/RRoutines.hpp>
#include <r/RErrorCategory.hpp>
#include <r/session/RSession.hpp>

#include <session/SessionModuleContext.hpp>
#include <session/SessionUserSettings.hpp>

#include "SessionWorkspaceGlobalEnv.hpp"

using namespace core;

namespace session {
namespace modules {
namespace workspace {

namespace {

// last save action.
// NOTE: we don't persist this (or the workspace dirty state) during suspends in
// server mode. this means that if you are ever suspended then you will always
// end up with a 'dirty' workspace. not a big deal considering how infrequently
// quit occurs in server mode.
int s_lastSaveAction = r::session::kSaveActionAsk;

void enqueSaveActionChanged()
{
   json::Object saveAction;
   saveAction["action"] = s_lastSaveAction;
   ClientEvent event(client_events::kSaveActionChanged, saveAction);
   module_context::enqueClientEvent(event);
}
   
void onClientInit()
{
   enqueSaveActionChanged();
}
 
void onDetectChanges(module_context::ChangeSource source)
{
   // compute current save action
   int currentSaveAction = r::session::imageIsDirty() ?
                                          userSettings().saveAction() :
                                          r::session::kSaveActionNoSave;

   // compare and fire event if necessary
   if (s_lastSaveAction != currentSaveAction)
   {
      s_lastSaveAction = currentSaveAction;
      enqueSaveActionChanged();
   }
}

} // anonymous namespace
 
Error initialize()
{         
   // subscribe to events
   using boost::bind;
   using namespace session::module_context;
   events().onClientInit.connect(bind(onClientInit));
   events().onDetectChanges.connect(bind(onDetectChanges, _1));
   
   // register handlers
   ExecBlock initBlock ;
   initBlock.addFunctions()
      (bind(sourceModuleRFile, "SessionWorkspace.R"))
      (bind(sourceModuleRFile, "SessionWorkspaceImportData.R"))
      (bind(session::modules::workspace::global_env::initialize));
   return initBlock.execute();
}
   

} // namepsace workspace
} // namespace modules
} // namesapce session

