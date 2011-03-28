#
# SessionWorkspaceImportData.R
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

.rs.addFunction("parseDataFile", function(path, header, sep, quote, nrows) {
   data <- tryCatch(
      read.table(path, header=header, sep=sep, quote=quote, nrows=nrows),
      error=function(e) {
         data.frame(Error=e$message)
      })

   oldWidth <- options('width')$width
   options(width=1000)
   output <- format(data)
   options(width=oldWidth)
   return(output)
})

.rs.addJsonRpcHandler("download_data_file", function(url)
{
   # download the file
   downloadPath <- tempfile("data")
   download.file(url, downloadPath)

   # return the path
   downloadInfo <- list()
   downloadInfo$path = downloadPath

   # also return a suggested variable name
   downloadInfo$varname <- "dataset"
   urlBasename <- basename(url)
   if (length(urlBasename) > 0)
   {
      fileComponents <- unlist(strsplit(urlBasename, ".", fixed = TRUE))
      components <- length(fileComponents)
      if (components >= 1)
         downloadInfo$varname <- paste(fileComponents[1:components-1],
                                       collapse=".")
   }

   return (downloadInfo)
})

.rs.addJsonRpcHandler("get_data_preview", function(path)
{
   nrows <- 20

   lines <- readLines(path, n=nrows, warn=F)

   # Drop comment lines, leaving the significant ones
   siglines <- grep("^[^#].*", lines, value=TRUE)
   firstline <- siglines[1]

   sep <- ''
   if (length(grep("\\t", firstline)) > 0)
      sep <- "\t"
   else if (length(grep(",", firstline)) > 0)
      sep <- ","
   else if (length(grep(";", firstline)) > 0)
      sep <- ";"

   header <- length(grep("[0-9]", firstline)) == 0

   quote <- "\""

   output <- .rs.parseDataFile(path, header=header, sep=sep, quote=quote, nrows=nrows)

   list(inputLines=paste(lines, collapse="\n"),
        output=output,
        outputNames=names(output),
        header=header,
        separator=sep,
        quote=quote)
})

.rs.addJsonRpcHandler("get_output_preview", function(path, header, sep, quote)
{
   nrows <- 20
   output <- .rs.parseDataFile(path, header=header, sep=sep, quote=quote, nrows=nrows)

   list(output=output,
        outputNames=names(output),
        header=header,
        separator=sep,
        quote=quote)
})
