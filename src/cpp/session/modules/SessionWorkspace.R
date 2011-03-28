#
# SessionWorkspace.R
#
# Copyright (C) 2009-11 by RStudio, Inc.
#
# This program is licensed to you under the terms of version 3 of the
# GNU Affero General Public License. This program is distributed WITHOUT
# ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THOSE OF NON-INFRINGEMENT,
# MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE. Please refer to the
# AGPL (http://www.gnu.org/licenses/agpl-3.0.txt) for more details.
#
#

.rs.addJsonRpcHandler("save_workspace", function(filename)
{
   # save the file 
   save.image(filename)
})

# NOTE: rpc calls should really be returning structured error values
# that the client can translate into end user error messages. the below
# implementation actually invokes an error dialog on the client directly
# which is definitley not the right long-term direction!
.rs.addJsonRpcHandler("load_workspace", function(filename)
{
   if (file.exists(filename))
   {
      load(filename, envir=globalenv())
   }
   else
   {
      .rs.showErrorMessage("Workspace Error",
                          paste("The file", filename, "does not exist."));
   }
})



