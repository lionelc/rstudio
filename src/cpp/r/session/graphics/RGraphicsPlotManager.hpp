/*
 * RGraphicsPlotManager.hpp
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

#ifndef R_SESSION_GRAPHICS_PLOT_MANAGER_HPP
#define R_SESSION_GRAPHICS_PLOT_MANAGER_HPP

#include <string>
#include <vector>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/signal.hpp>
#include <boost/regex.hpp>
#include <boost/circular_buffer.hpp>

#include <core/Error.hpp>
#include <core/FilePath.hpp>

#include <r/session/RGraphics.hpp>

#include "RGraphicsTypes.hpp"
#include "RGraphicsPlot.hpp"

namespace r {
namespace session {
namespace graphics {

// singleton
class PlotManager;
PlotManager& plotManager();   

struct GraphicsDeviceEvents
{
   boost::signal<void (SEXP)> onNewPage; 
   boost::signal<void ()> onDrawing;
   boost::signal<void ()> onResized;
   boost::signal<void ()> onClosed;
};

class PlotManipulatorManager;
 
class PlotManager : boost::noncopyable, public r::session::graphics::Display
{   
private:
   PlotManager();
   friend PlotManager& plotManager();
   
public:
   virtual ~PlotManager() {}
   
   core::Error initialize(const core::FilePath& graphicsPath,
                          const GraphicsDeviceFunctions& graphicsDevice,
                          GraphicsDeviceEvents* pEvents);
   
   // plot list
   virtual int plotCount() const;
   virtual core::Error plotImageFilename(int index, 
                                         std::string* pImageFilename) const;
   virtual int activePlotIndex() const;
   virtual core::Error setActivePlot(int index) ;
   virtual core::Error removePlot(int index);
   
   // actions on active plot
   virtual core::Error savePlotAsPng(const core::FilePath& filePath,
                                     int widthPx,
                                     int heightPx);
      
   virtual core::Error savePlotAsPdf(const core::FilePath& filePath,
                                     double widthInches,
                                     double heightInches);

   // display
   virtual bool hasOutput() const;
   virtual bool hasChanges() const;
   virtual void render(boost::function<void(DisplayState)> outputFunction); 
   virtual std::string imageFilename() const ;
   virtual void refresh() ;
   
    // retrieve image path based on filename
   virtual core::FilePath imagePath(const std::string& imageFilename) const;
   
   virtual void clear();

   virtual boost::signal<void ()>& onShowManipulator() ;
   virtual void setPlotManipulatorValues(const core::json::Object& values);

   virtual void onBeforeExecute();

   // manipulate persistent state
   core::Error savePlotsState(const core::FilePath& plotsStateFile);
   core::Error restorePlotsState(const core::FilePath& plotsStateFile);
      
private:
   
   // make plot manipulator manager a friend
   friend class PlotManipulatorManager;

   // typedefs
   typedef boost::shared_ptr<Plot> PtrPlot ;

   // device events
   void onDeviceNewPage(SEXP previousPageSnapshot);
   void onDeviceDrawing();
   void onDeviceResized();
   void onDeviceClosed();
   
   // active plot 
   Plot& activePlot() const;
   bool isValidPlotIndex(int index) const;
   bool hasPlot() const;
   
   // invalidate the active plot
   void invalidateActivePlot();

   // render active plot to display (used in setActivePlot and onSessionResume)
   void renderActivePlotToDisplay();
   
   // render active plot file file
   core::Error savePlotAsFile(const boost::function<core::Error()>&
                                                         deviceCreationFunction);
   core::Error savePlotAsFile(const std::string& fileDeviceCreationCode);
   core::Error savePlotAsFile(const std::string& fileType,
                              int width,
                              int height,
                              const core::FilePath& targetPath);
   
   // error helpers
   core::Error plotIndexError(int index, const core::ErrorLocation& location)
                                                                         const;

   std::string emptyImageFilename() const ;

private:   
   friend class SuppressDeviceEventsScope;

   // storage path
   core::FilePath graphicsPath_;
  
   // interface to graphics device
   GraphicsDeviceFunctions graphicsDevice_ ;
   
   // state
   bool displayHasChanges_;
   bool suppressDeviceEvents_;
   
   int activePlot_;
   boost::circular_buffer<PtrPlot> plots_ ;
   
   boost::regex plotInfoRegex_;
};

class SuppressDeviceEventsScope
{
public:
   SuppressDeviceEventsScope(PlotManager& plotManager)
      : plotManager_(plotManager)
   {
      plotManager_.suppressDeviceEvents_ = true;
   }

   virtual ~SuppressDeviceEventsScope()
   {
      plotManager_.suppressDeviceEvents_ = false;
   }
private:
   PlotManager& plotManager_;
};


} // namespace graphics
} // namespace session
} // namespace r

#endif // R_SESSION_GRAPHICS_PLOT_MANAGER_HPP 

