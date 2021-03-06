/*
 * REnvironmentPosix.cpp
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

#include <core/r_util/REnvironment.hpp>

#include <algorithm>

#include <boost/algorithm/string/trim.hpp>

#include <core/Error.hpp>
#include <core/FilePath.hpp>
#include <core/ConfigUtils.hpp>
#include <core/system/System.hpp>

namespace core {
namespace r_util {

namespace {

// MacOS X Specific
#ifdef __APPLE__

#define kLibRFileName            "libR.dylib"
#define kLibraryPathEnvVariable  "DYLD_LIBRARY_PATH"

// no extra paths on the mac
std::string extraLibraryPaths(const FilePath& ldPathsScript,
                              const std::string& rHome)
{
   return std::string();
}

FilePath systemDefaultRScript()
{
   // define potential paths (use same order as in conventional osx PATH)
   std::vector<std::string> rScriptPaths;
   rScriptPaths.push_back("/opt/local/bin/R");
   rScriptPaths.push_back("/usr/bin/R");
   rScriptPaths.push_back("/usr/local/bin/R");

   // iterate over paths
   for (std::vector<std::string>::const_iterator it = rScriptPaths.begin();
        it != rScriptPaths.end();
        ++it)
   {
      FilePath rScriptPath(*it);
      if (rScriptPath.exists())
      {
         // verify that the alias points to a real version of R
         Error error = core::system::realPath(*it, &rScriptPath);
         if (!error)
         {
            return rScriptPath;
         }
         else
         {
            LOG_ERROR(error);
            continue;
         }
      }
   }

   // didn't find it
   return FilePath();
}

bool getRHomeAndLibPath(const FilePath& rScriptPath,
                        const config_utils::Variables& scriptVars,
                        std::string* pRHome,
                        std::string* pRLibPath,
                        std::string* pErrMsg)
{
   config_utils::Variables::const_iterator it = scriptVars.find("R_HOME_DIR");
   if (it != scriptVars.end())
   {
      // get R home
      *pRHome = it->second;

      // get R lib path (probe subdiretories if necessary)
      FilePath libPath = FilePath(*pRHome).complete("lib");

      // check for dylib in lib and lib/x86_64
      if (libPath.complete(kLibRFileName).exists())
      {
         *pRLibPath = libPath.absolutePath();
         return true;
      }
      else if (libPath.complete("x86_64/" kLibRFileName).exists())
      {
         *pRLibPath = libPath.complete("x86_64").absolutePath();
         return true;
      }
      else
      {
         *pErrMsg = "Unable to find " kLibRFileName " in expected locations"
                    "within R Home directory " + *pRHome;
         LOG_ERROR_MESSAGE(*pErrMsg);
         return false;
      }
   }
   else
   {
      *pErrMsg = "Unable to find R_HOME_DIR in " + rScriptPath.absolutePath();
      LOG_ERROR_MESSAGE(*pErrMsg);
      return false;
   }
}

// Linux specific
#else

#define kLibRFileName            "libR.so"
#define kLibraryPathEnvVariable  "LD_LIBRARY_PATH"

// extra paths from R (for rjava) on linux
std::string extraLibraryPaths(const FilePath& ldPathsScript,
                              const std::string& rHome)
{
   // verify that script exists
   if (!ldPathsScript.exists())
   {
      LOG_WARNING_MESSAGE("r-ldpaths script not found at " +
                          ldPathsScript.absolutePath());
      return std::string();
   }

   // run script to capture paths
   std::string libraryPaths;
   std::string command = ldPathsScript.absolutePath() + " " + rHome;
   Error error = system::captureCommand(command, &libraryPaths);
   if (error)
      LOG_ERROR(error);
   boost::algorithm::trim(libraryPaths);
   return libraryPaths;
}

FilePath systemDefaultRScript()
{
   // ask system which R to use
   std::string whichOutput;
   Error error = core::system::captureCommand("which R", &whichOutput);
   if (error)
   {
      LOG_ERROR(error);
      return FilePath();
   }
   boost::algorithm::trim(whichOutput);

   // check for nothing returned
   if (whichOutput.empty())
      return FilePath();

   // verify that the alias points to a real version of R
   FilePath rBinaryPath;
   error = core::system::realPath(whichOutput, &rBinaryPath);
   if (error)
   {
      LOG_ERROR(error);
      return FilePath();
   }

   // check for real path doesn't exist
   if (!rBinaryPath.exists())
      return FilePath();

   // got a valid R binary
   return rBinaryPath;
}

bool getRHomeAndLibPath(const FilePath& rScriptPath,
                        const config_utils::Variables& scriptVars,
                        std::string* pRHome,
                        std::string* pRLibPath,
                        std::string* pErrMsg)
{
   // run R script to detect R home
   std::string rHomeOutput;
   std::string command = rScriptPath.absolutePath() + " RHOME";
   Error error = core::system::captureCommand(command, &rHomeOutput);
   if (error)
   {
      LOG_ERROR(error);
      *pErrMsg = "Error running R (" + rScriptPath.absolutePath() + "): " +
                 error.summary();
      return false;
   }
   else
   {
      boost::algorithm::trim(rHomeOutput);
      *pRHome = rHomeOutput;
      *pRLibPath = FilePath(*pRHome).complete("lib").absolutePath();
      return true;
   }
}


#endif


bool validateREnvironment(const EnvironmentVars& vars,
                          const FilePath& rLibPath,
                          std::string* pErrMsg)
{
   // first extract paths
   FilePath rHomePath, rSharePath, rIncludePath, rDocPath, rLibRPath;
   for (EnvironmentVars::const_iterator it = vars.begin();
        it != vars.end();
        ++it)
   {
      if (it->first == "R_HOME")
         rHomePath = FilePath(it->second);
      else if (it->first == "R_SHARE_DIR")
         rSharePath = FilePath(it->second);
      else if (it->first == "R_INCLUDE_DIR")
         rIncludePath = FilePath(it->second);
      else if (it->first == "R_DOC_DIR")
         rDocPath = FilePath(it->second);
   }

   // resolve libR path
   rLibRPath = rLibPath.complete(kLibRFileName);

   // validate required paths (if these don't exist then rsession won't
   // be able start up)
   if (!rHomePath.exists())
   {
      *pErrMsg = "R Home path (" + rHomePath.absolutePath() + ") not found";
      return false;
   }
   else if (!rLibPath.exists())
   {
      *pErrMsg = "R lib path (" + rLibPath.absolutePath() + ") not found";
      return false;
   }
   else if (!rLibRPath.exists())
   {
      *pErrMsg = "R shared library (" + rLibRPath.absolutePath() + ") "
                 "not found. If this is a custom build of R, was it "
                 "built with the --enable-R-shlib option?";
      return false;
   }
   else if (!rDocPath.exists())
   {
      *pErrMsg = "R doc dir (" + rDocPath.absolutePath() + ") not found.";
      return false;
   }

   // log warnings for other missing paths (rsession can still start but
   // won't be able to find these env variables)

   if (!rSharePath.exists())
   {
      LOG_WARNING_MESSAGE("R share path (" + rSharePath.absolutePath() +
                          ") not found");
   }

   if (!rIncludePath.exists())
   {
      LOG_WARNING_MESSAGE("R include path (" + rIncludePath.absolutePath() +
                          ") not found");
   }

   return true;
}

} // anonymous namespace


bool detectREnvironment(const FilePath& ldPathsScript,
                        EnvironmentVars* pVars,
                        std::string* pErrMsg)
{
   return detectREnvironment(FilePath(), ldPathsScript, pVars, pErrMsg);
}

bool detectREnvironment(const FilePath& whichRScript,
                        const FilePath& ldPathsScript,
                        EnvironmentVars* pVars,
                        std::string* pErrMsg)
{
   // if there is a which R script override then validate it (but only
   // log a warning then move on to the system default)
   FilePath rScriptPath;
   if (!whichRScript.empty())
   {
      // but warn (and ignore) if it doesn't exist
      if (!whichRScript.exists())
      {
         LOG_WARNING_MESSAGE("Override for which R (" +
                             whichRScript.absolutePath() +
                             ") does not exist (ignoring)");
      }

      // also warn and ignore if it is a directory
      else if (whichRScript.isDirectory())
      {
         LOG_WARNING_MESSAGE("Override for which R (" +
                             whichRScript.absolutePath() +
                             ") is a directory rather than a file (ignoring)");
      }

      // otherwise set it
      else
      {
         rScriptPath = whichRScript;
      }
   }

   // if don't have an override (or it was invalid) then use system default
   if (rScriptPath.empty())
      rScriptPath = systemDefaultRScript();

   // bail if not found
   if (!rScriptPath.exists())
   {
      LOG_ERROR(pathNotFoundError(rScriptPath.absolutePath(), ERROR_LOCATION));
      *pErrMsg = "R binary (" + rScriptPath.absolutePath() + ") not found";
      return false;
   }

   // scan R script for other locations and append them to our vars
   config_utils::Variables scriptVars;
   Error error = config_utils::extractVariables(rScriptPath, &scriptVars);
   if (error)
   {
      LOG_ERROR(error);
      *pErrMsg = "Error reading R script (" + rScriptPath.absolutePath() +
                 "), " + error.summary();
      return false;
   }
   pVars->push_back(std::make_pair("R_SHARE_DIR", scriptVars["R_SHARE_DIR"]));
   pVars->push_back(std::make_pair("R_INCLUDE_DIR", scriptVars["R_INCLUDE_DIR"]));
   pVars->push_back(std::make_pair("R_DOC_DIR", scriptVars["R_DOC_DIR"]));

   // get r home path
   std::string rHome, rLib;
   if (!getRHomeAndLibPath(rScriptPath, scriptVars, &rHome, &rLib, pErrMsg))
      return false;

   // validate: error if we got no output
   if (rHome.empty())
   {
      *pErrMsg = "Unable to determine R home directory";
      LOG_ERROR(systemError(boost::system::errc::not_supported,
                            *pErrMsg,
                            ERROR_LOCATION));
      return false;
   }

   // validate: error if `R RHOME` yields file that doesn't exist
   FilePath rHomePath(rHome);
   if (!rHomePath.exists())
   {
      *pErrMsg = "R home path (" + rHome + ") not found";
      LOG_ERROR(pathNotFoundError(*pErrMsg, ERROR_LOCATION));
      return false;
   }

   // set R home path
   pVars->push_back(std::make_pair("R_HOME", rHomePath.absolutePath()));

   // determine library path (existing + r lib dir + r extra lib dirs)
   FilePath rLibPath(rLib);
   std::string libraryPath = core::system::getenv(kLibraryPathEnvVariable);
   if (!libraryPath.empty())
      libraryPath.append(":");
   libraryPath.append(rLibPath.absolutePath());
   std::string extraPaths = extraLibraryPaths(ldPathsScript,
                                              rHomePath.absolutePath());
   if (!extraPaths.empty())
      libraryPath.append(":" + extraPaths);
   pVars->push_back(std::make_pair(kLibraryPathEnvVariable, libraryPath));

   return validateREnvironment(*pVars, rLibPath, pErrMsg);
}


void setREnvironmentVars(const EnvironmentVars& vars)
{
   for (EnvironmentVars::const_iterator it = vars.begin();
        it != vars.end();
        ++it)
   {
      core::system::setenv(it->first, it->second);
   }
}

} // namespace r_util
} // namespace core 



