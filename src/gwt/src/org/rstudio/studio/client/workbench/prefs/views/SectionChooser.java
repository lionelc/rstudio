package org.rstudio.studio.client.workbench.prefs.views;

import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.dom.client.HasClickHandlers;
import com.google.gwt.event.logical.shared.HasSelectionHandlers;
import com.google.gwt.event.logical.shared.SelectionEvent;
import com.google.gwt.event.logical.shared.SelectionHandler;
import com.google.gwt.event.shared.HandlerRegistration;
import com.google.gwt.resources.client.ImageResource;
import com.google.gwt.user.client.ui.Image;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.SimplePanel;
import com.google.gwt.user.client.ui.VerticalPanel;
import com.google.inject.Inject;

public class SectionChooser extends SimplePanel implements
                                                HasSelectionHandlers<Integer>
{
   private class ClickableVerticalPanel extends VerticalPanel
      implements HasClickHandlers
   {

      public HandlerRegistration addClickHandler(ClickHandler handler)
      {
         return addDomHandler(handler, ClickEvent.getType());
      }
   }

   @Inject
   public SectionChooser(PreferencesDialogResources res)
   {
      res_ = res;

      setStyleName(res_.styles().sectionChooser());
      inner_.setStyleName(res_.styles().sectionChooserInner());
      setWidget(inner_);
   }

   public void addSection(ImageResource icon, String name)
   {
      Image img = new Image(icon);
      Label label = new Label(name, false);
      final ClickableVerticalPanel panel = new ClickableVerticalPanel();
      panel.setHorizontalAlignment(VerticalPanel.ALIGN_CENTER);
      panel.add(img);
      panel.add(label);
      panel.setStyleName(res_.styles().section());

      panel.addClickHandler(new ClickHandler()
      {
         public void onClick(ClickEvent event)
         {
            select(inner_.getWidgetIndex(panel));
         }
      });

      inner_.add(panel);
   }

   public void select(Integer index)
   {
      if (selectedIndex_ != null)
         inner_.getWidget(selectedIndex_).removeStyleName(res_.styles().activeSection());

      selectedIndex_ = index;

      if (index != null)
         inner_.getWidget(index).addStyleName(res_.styles().activeSection());

      SelectionEvent.fire(this, index);
   }

   public HandlerRegistration addSelectionHandler(SelectionHandler<Integer> handler)
   {
      return addHandler(handler, SelectionEvent.getType());
   }

   public int getDesiredWidth()
   {
      return 100;
   }

   private Integer selectedIndex_;
   private final PreferencesDialogResources res_;
   private final VerticalPanel inner_ = new VerticalPanel();
}
