/**
 * Copyright (c) 2009, Christian Schneider
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * - Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  - Neither the names of the authors nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
package com.ruinwesen.patchdownloader.swing.panels;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import name.cs.csutils.I18N;

import com.ruinwesen.patchdownloader.indexer.Query;
import com.ruinwesen.patchdownloader.swing.SearchController;
import com.ruinwesen.patchdownloader.swing.SwingPatchdownloader;
import com.ruinwesen.patchdownloader.swing.misc.TextPopup;

public class SearchPanel  {

    private JTextField textQueryField;
    private JPanel container;
    private JComboBox cbOrder;
    private SwingPatchdownloader patchdownloader;
    private Query previousQuery = new Query("");
    
    public SearchPanel(SwingPatchdownloader patchdownloader) {
        this.patchdownloader = patchdownloader;
        textQueryField = new JTextField();

        new TextPopup().installAt(textQueryField);
        
        JLabel labelSearch = new JLabel(I18N.translate("translation.search", "Search")+":");
        labelSearch.setLabelFor(textQueryField);

        final String RELEVANCE = I18N.translate("translation.sortby.relevance", "Relevance");
        final String DATE_ADDED = I18N.translate("translation.sortby.dateadded", "Date Added");
        cbOrder = new JComboBox(new Object[]{RELEVANCE, DATE_ADDED});

        Font smallLabelFont = labelSearch.getFont(); // CSUtils.changeFontSize(labelSearch.getFont(), 0.8f); 
        Font smallComboboxFont = cbOrder.getFont(); // CSUtils.changeFontSize(cbOrder.getFont(), 0.8f); 
        labelSearch.setFont(smallLabelFont);
        cbOrder.setFont(smallComboboxFont);
        
        container = new JPanel();
        container.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
        container.setLayout(new BoxLayout(container, BoxLayout.X_AXIS));
        
        {
            JLabel labelSortby = new JLabel(I18N.translate("translation.sortby", "Sort by")+":");
            labelSortby.setLabelFor(cbOrder);
            labelSortby.setFont(smallLabelFont);
            cbOrder.setAlignmentX(Component.LEFT_ALIGNMENT);
            labelSortby.setAlignmentX(Component.LEFT_ALIGNMENT);
            
            labelSearch.setAlignmentX(Component.LEFT_ALIGNMENT);
            textQueryField.setAlignmentX(Component.LEFT_ALIGNMENT);
            textQueryField.setMinimumSize(new Dimension(120,0));
            
            cbOrder.setMaximumSize(new Dimension(100,30));
            
            Box box = Box.createHorizontalBox();
            box.add(labelSearch);
            box.add(Box.createHorizontalStrut(5));
            box.add(textQueryField);
            box.add(Box.createHorizontalStrut(10));
            box.add(labelSortby);
            box.add(Box.createHorizontalStrut(5));
            box.add(cbOrder);
            container.add(box);

        }
        new EventHandler().install();
    }
    
    public JComponent getContainer() {
        return container;
    }
    
    public String getQueryString() {
        return norm(textQueryField.getText());
    }
    
    public Query getQuery() {
        return new Query(getQueryString());
    }
    
    private String norm(String str) {
        return str == null || str.trim().length() == 0 ? "" : str;    
    }
    
    public void setQueryString(String text) {
        String oldQueryString = getQueryString();
        String newQueryString = norm(text);
        if (!newQueryString.equals(oldQueryString)) {
            textQueryField.setText(newQueryString);
        }
    }
    
    private class EventHandler implements
        ActionListener, DocumentListener {

        javax.swing.Timer timer;
        long mostRecentQueryChange = 0;
        static final long queryChangeAcceptTime = 500; 
        public void install() {
            cbOrder.addActionListener(this);
            textQueryField.getDocument().addDocumentListener(this);
            timer = new javax.swing.Timer(0, this);
            timer.setCoalesce(true);
            timer.setDelay(100);
            timer.setRepeats(true);
            timer.stop();
        }

        @Override
        public void actionPerformed(ActionEvent e) {
            if (cbOrder == e.getSource()) {
                SearchController controller = patchdownloader.getSearchController();
                if (cbOrder.getSelectedIndex() == 0) {
                    controller.setOrderByRelevance();
                } else {
                    controller.setOrderByDate();
                }
                controller.update();
            } else if (e.getSource() == timer 
                    && timer.isRunning()
                    && System.currentTimeMillis()>mostRecentQueryChange+queryChangeAcceptTime) {
                timer.stop();
                notifyQueryAccepted();
            }
        }

        @Override
        public void changedUpdate(DocumentEvent e) {
            // no op
        }

        @Override
        public void insertUpdate(DocumentEvent e) {
            notifyQueryChanged();
        }

        @Override
        public void removeUpdate(DocumentEvent e) {
            notifyQueryChanged();
        }

        private void notifyQueryChanged() {
            mostRecentQueryChange = System.currentTimeMillis();
            if (!timer.isRunning())
                timer.start();
        }
        
        private void notifyQueryAccepted() {
            Query newQuery = getQuery();
            if (!previousQuery.equals(newQuery)) {
                previousQuery = newQuery;
                patchdownloader.getSearchController().setQuery(newQuery);
                patchdownloader.getSearchController().update();
            }
        }

    }

}
