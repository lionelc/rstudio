package org.rstudio.studio.client.workbench.views.source.editors.text.ace;

import com.google.gwt.core.client.JavaScriptObject;

public class Position extends JavaScriptObject
{
   protected Position() {}

   public static native Position create(int row, int column) /*-{
      return {row: row, column: column};
   }-*/;
}