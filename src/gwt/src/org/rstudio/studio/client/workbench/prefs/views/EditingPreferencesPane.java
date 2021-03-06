package org.rstudio.studio.client.workbench.prefs.views;

import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.resources.client.ImageResource;
import com.google.gwt.user.client.ui.CheckBox;
import com.google.inject.Inject;
import org.rstudio.core.client.StringUtil;
import org.rstudio.core.client.widget.OperationWithInput;
import org.rstudio.studio.client.common.SimpleRequestCallback;
import org.rstudio.studio.client.workbench.prefs.model.UIPrefs;
import org.rstudio.studio.client.workbench.views.source.editors.text.IconvListResult;
import org.rstudio.studio.client.workbench.views.source.editors.text.ui.ChooseEncodingDialog;
import org.rstudio.studio.client.workbench.views.source.model.SourceServerOperations;

public class EditingPreferencesPane extends PreferencesPane
{
   @Inject
   public EditingPreferencesPane(SourceServerOperations server,
                                 UIPrefs prefs,
                                 PreferencesDialogResources res)
   {
      server_ = server;
      prefs_ = prefs;
      res_ = res;

      add(checkboxPref("Highlight selected line", prefs.highlightSelectedLine()));
      add(checkboxPref("Show line numbers", prefs.showLineNumbers()));
      add(tight(spacesForTab_ = checkboxPref("Insert spaces for tab", prefs.useSpacesForTab())));
      add(indent(tabWidth_ = numericPref("Tab width", prefs.numSpacesForTab())));
      add(tight(showMargin_ = checkboxPref("Show margin", prefs.showMargin())));
      add(indent(marginCol_ = numericPref("Margin column", prefs.printMarginColumn())));
//      add(checkboxPref("Automatically insert matching parens/quotes", prefs_.insertMatching()));
      add(checkboxPref("Soft-wrap R source files", prefs_.softWrapRFiles()));

      encodingValue_ = prefs_.defaultEncoding().getValue();
      add(encoding_ = new TextBoxWithButton(
            "Default text encoding:",
            "Change...",
            new ClickHandler()
            {
               public void onClick(ClickEvent event)
               {
                  server_.iconvlist(new SimpleRequestCallback<IconvListResult>()
                  {
                     @Override
                     public void onResponseReceived(IconvListResult response)
                     {
                        new ChooseEncodingDialog(
                              response.getCommon(),
                              response.getAll(),
                              encodingValue_,
                              true,
                              false,
                              new OperationWithInput<String>()
                              {
                                 public void execute(String encoding)
                                 {
                                    if (encoding == null)
                                       return;

                                    setEncoding(encoding);
                                 }
                              }).showModal();
                     }
                  });

               }
            }));
      encoding_.setWidth("250px");
      encoding_.addStyleName(res_.styles().encodingChooser());
      setEncoding(prefs.defaultEncoding().getValue());
   }

   private void setEncoding(String encoding)
   {
      encodingValue_ = encoding;
      if (StringUtil.isNullOrEmpty(encoding))
         encoding_.setText(ChooseEncodingDialog.ASK_LABEL);
      else
         encoding_.setText(encoding);
   }


   @Override
   public ImageResource getIcon()
   {
      return res_.iconEdit();
   }

   @Override
   public boolean validate()
   {
      return (!spacesForTab_.getValue() || tabWidth_.validate("Tab width")) &&
             (!showMargin_.getValue() || marginCol_.validate("Margin column"));
   }

   @Override
   public String getName()
   {
      return "Editing";
   }

   @Override
   public void onApply()
   {
      super.onApply();
      prefs_.defaultEncoding().setValue(encodingValue_);
   }

   private final SourceServerOperations server_;
   private final UIPrefs prefs_;
   private final PreferencesDialogResources res_;
   private final NumericValueWidget tabWidth_;
   private final NumericValueWidget marginCol_;
   private final TextBoxWithButton encoding_;
   private String encodingValue_;
   private final CheckBox spacesForTab_;
   private final CheckBox showMargin_;
}
