#
# SessionWorkspaceGlobalEnv.R
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

.rs.addFunction("valueAsString", function(val)
{
   tryCatch(
   {
      is.scalar <- function (x) {
         if (length(x) == 1 && is.null(attributes(x)))
         {
            !is.na(c(NULL=TRUE,
                     logical=TRUE,
                     double=TRUE,
                     integer=TRUE,
                     complex=TRUE,
                     character=TRUE)[typeof(x)])
         }
         else
         {
            FALSE
         }
      }

      if (is.scalar(val))
      {
         if (nchar(val) < 100)
            return (deparse(val))
         else
            return ("NO_VALUE")
      }
      else if (is.function(val))
         return (.rs.getSignature(val))
      else
         return ("NO_VALUE")
   },
   error = function(e) print(e))

   return ("NO_VALUE")
})

.rs.addFunction("valueDescription", function(obj)
{
   tryCatch(
   {
      if (is.data.frame(obj))
      {
         return(paste(dim(obj)[1],
                      "obs. of",
                      dim(obj)[2],
                      "variables",
                      sep=" "))
      }
      else if (is.matrix(obj))
      {
         return(paste(ncol(obj),
                      "x",
                      nrow(obj),
                      " ",
                      typeof(obj),
                      " matrix",
                      sep=""))
      }
      else
         return("")
   },
   error = function(e) print(e))

   return ("")
})

.rs.addJsonRpcHandler("remove_all_objects", function()
{
   env = globalenv()
   rm(list=ls(envir=env, all.names=TRUE), envir=env)
})

.rs.addFunction("getSingleClass", function(obj)
{
   className <- "(unknown)"
   tryCatch(className <- class(obj)[1],
            error = function(e) print(e))
   return (className)
})

.rs.addJsonRpcHandler("list_objects", function()
{
   globals = ls(envir=globalenv())
   globalValues = lapply(globals, function (name) {
                            get(name, envir=globalenv(), inherits=FALSE)
                         })
   types = sapply(globalValues, .rs.getSingleClass, USE.NAMES=FALSE)
   lengths = sapply(globalValues, length, USE.NAMES=FALSE)
   values = sapply(globalValues, .rs.valueAsString, USE.NAMES=FALSE)
   extra = sapply(globalValues, .rs.valueDescription, USE.NAMES=FALSE)
   
   result = list(name=globals,
                       type=types,
                       len=lengths,
                       value=values,
                       extra=extra)
   #print(result)
   result
})

.rs.addJsonRpcHandler("get_object_value", function(name)
{
   value = get(name, envir=globalenv(), inherits=FALSE)
   strval = paste(deparse(value), collapse="\n")
   
   list(name=name,
        type=.rs.getSingleClass(value),
        len=length(value),
        value=strval,
        extra=.rs.valueDescription(value))
})

.rs.addJsonRpcHandler("set_object_value", function(name, value)
{
   assign(name, eval(parse(text=value), envir=globalenv()), envir=globalenv(), inherits=FALSE)
   NULL
})

