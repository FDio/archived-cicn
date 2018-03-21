/*
 * Copyright (c) 2018 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package icn.iget.com.adapter;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;


import java.io.File;
import java.util.ArrayList;

import icn.iget.com.igetandroid.R;
import icn.iget.com.utility.Constants;

public class ListViewAdapter extends BaseAdapter {

    Context context;
    ArrayList<OutputListViewElement> outputListViewElementArrayList;
    private static LayoutInflater inflater = null;

    public ListViewAdapter(Context context, ArrayList<OutputListViewElement> outputListViewElementArrayList) {
        this.context = context;
        this.outputListViewElementArrayList = outputListViewElementArrayList;
        inflater = (LayoutInflater) context
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    }

    @Override
    public int getCount() {
        return outputListViewElementArrayList.size();
    }

    @Override
    public Object getItem(int position) {
        return outputListViewElementArrayList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        View view = convertView;
        if (view == null)
            view = inflater.inflate(R.layout.list_view_row, null);
        TextView urlTextView = (TextView) view.findViewById(R.id.urlTextView);
        urlTextView.setText(outputListViewElementArrayList.get(position).getUrl());

        TextView savedPathTextView = (TextView) view.findViewById(R.id.savedPathTextView);
        savedPathTextView.setText(outputListViewElementArrayList.get(position).getSavedPath() + File.separator + outputListViewElementArrayList.get(position).getNameFile());

        TextView md5TextView = (TextView) view.findViewById(R.id.md5TextView);
        md5TextView.setText(outputListViewElementArrayList.get(position).getMd5());

        TextView sizeTextView = (TextView) view.findViewById(R.id.sizeTextView);
        sizeTextView.setText(Integer.toString(outputListViewElementArrayList.get(position).getSize()));

        TextView dateTextView = (TextView) view.findViewById(R.id.dateTextView);
        dateTextView.setText(outputListViewElementArrayList.get(position).getDateSting(Constants.FORMAT_DATA));


        return view;
    }
}
