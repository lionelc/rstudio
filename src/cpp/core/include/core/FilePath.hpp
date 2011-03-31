/*
 * FilePath.hpp
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

#ifndef CORE_FILE_PATH_HPP
#define CORE_FILE_PATH_HPP

#include <stdint.h>
#include <ctime>

#include <string>
#include <vector>
#include <iosfwd>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <boost/utility.hpp>

#include <core/Error.hpp>
#include <core/Log.hpp>

namespace core {

class Error ;

class FilePath
{
public:
   typedef boost::function<void(int, const FilePath&)>  
                                                RecursiveIterationFunction;

public:
   // special accessor which detects when the current path no longer exists
   // and switches to the specified alternate path if it doesn't
   static FilePath safeCurrentPath(const FilePath& revertToPath) ;
   
   static Error makeCurrent(const std::string& path);

   static std::string createAliasedPath(const core::FilePath& path,
                                        const FilePath& userHomePath);
   static FilePath resolveAliasedPath(const std::string& aliasedPath,
                                      const FilePath& userHomePath) ;
   
public:
   FilePath() ;
   explicit FilePath(const std::string& absolutePath) ;
   virtual ~FilePath() ;
   // COPYING: via shared_ptr (immutable)
   
public:
   // does this instance contain a path?
   bool empty() const;

   // does this file exist?
   bool exists() const;

   // size of file in bytes
   uintmax_t size() const;
  
   // filename only
   std::string filename() const ;
  
   // filename without extension
   std::string stem() const ;

   // file extensions
   std::string extension() const ;
   std::string extensionLowerCase() const;
   
   // mime types
   std::string mimeContentType(
                     const std::string& defaultType = "text/plain") const;
   
   // last write time
   std::time_t lastWriteTime() const;
  
   // full filesystem absolute path 
   std::string absolutePath() const ;

   // path relative to parent directory. returns empty string if this path
   // is not a child of the passed parent path
   std::string relativePath(const FilePath& parentPath) const ;
   
   // is this path within the scope of the passed scopePath (returns true
   // if the two paths are equal)
   bool isWithin(const FilePath& scopePath) const;

   // delete file
   Error remove() const ;
   Error removeIfExists() const;
   
   // move to path
   Error move(const FilePath& targetPath) const ;
   
   // copy to path
   Error copy(const FilePath& targetPath) const;

   // is this a hidden file?
   bool isHidden() const ;
   
   // is this a directory?
   bool isDirectory() const ;

   // create this directory if it doesn't already exist
   Error ensureDirectory() const ;

   // create directory at relative path
   Error createDirectory(const std::string& path) const ;
   
   // remove the directory (if it exists) and create a new one in its place
   Error resetDirectory() const;
   
   // complete a path (if input path is relative, returns path relative
   // to this one; if input path is absolute returns that path)
   FilePath complete(const std::string& path) const;
   
   // get child path relative to this one.
   FilePath childPath(const std::string& path) const ;
   
   // get this path's parent
   FilePath parent() const;

   // list child paths
   Error children(std::vector<FilePath>* pFilePaths) const ;  
   
   // recursively iterate over child paths
   Error childrenRecursive(RecursiveIterationFunction iterationFunction) const;

   // make this path the system current directory
   Error makeCurrentPath(bool autoCreate = false) const ;

   // compare two instances (equal if absolutePath == absolutePath)
   bool operator== (const FilePath& filePath) const ;
   bool operator!= (const FilePath& filePath) const ;
   
   // natural order is based on absolute path
   bool operator < (const FilePath& other) const ;

private:
   struct Impl ;
   boost::shared_ptr<const Impl> pImpl_ ;
};
   
std::ostream& operator << (std::ostream& stream, const FilePath& fp) ;

bool compareAbsolutePathNoCase(const FilePath& file1, const FilePath& file2);
   
class RestoreCurrentPathScope : boost::noncopyable
{
public:
   RestoreCurrentPathScope(const FilePath& restorePath)
      : restorePath_(restorePath) 
   {
   }
   
   virtual ~RestoreCurrentPathScope()
   {
      try
      {
         Error error = restorePath_.makeCurrentPath();
         if (error)
            LOG_ERROR(error);
      }
      catch(...)
      {
      }
   }
private:
   FilePath restorePath_ ;
};

}

#endif // CORE_FILE_PATH_HPP



