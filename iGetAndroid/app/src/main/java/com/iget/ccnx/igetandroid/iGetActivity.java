/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
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
package com.iget.ccnx.igetandroid;

import android.Manifest;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.iget.ccnx.adapter.ListViewAdapter;
import com.iget.ccnx.adapter.OutputListViewElement;
import com.iget.ccnx.utility.Constants;
import com.iget.ccnx.utility.ResourcesEnumerator;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Date;

public class iGetActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib");
    }
    
    private static String TAG = "iGetAcrivity";

    public native String downloadFile(String path);
    
    ArrayList<OutputListViewElement> outputListViewElementArrayList = new ArrayList<OutputListViewElement>();
    ListViewAdapter adapter;
    
    
    static int fCount = 0;
    SharedPreferences sharedPreferences;
    EditText urlEditText;
    EditText downloadPathEditText;
    Button downloadButton;
    ListView resultListView;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_i_get);
        adapter = new ListViewAdapter(this, outputListViewElementArrayList);
        resultListView = (ListView) findViewById(R.id.resultsListView);
        resultListView.setAdapter(adapter);
        checkEnabledPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        checkEnabledPermission(Manifest.permission.READ_EXTERNAL_STORAGE);
        init();
    }
    
    public static final String md5(final String s) {
        final String MD5 = "MD5";
        try {
            // Create MD5 Hash
            MessageDigest digest = java.security.MessageDigest
            .getInstance(MD5);
            digest.update(s.getBytes());
            byte messageDigest[] = digest.digest();
            
            // Create Hex String
            StringBuilder hexString = new StringBuilder();
            for (byte aMessageDigest : messageDigest) {
                String h = Integer.toHexString(0xFF & aMessageDigest);
                while (h.length() < 2)
                    h = "0" + h;
                hexString.append(h);
            }
            return hexString.toString();
            
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        return "";
    }
    
    @Override
    protected void onSaveInstanceState(Bundle outState) {
        outState.putSerializable(Constants.LIST_ITEMS_ID, outputListViewElementArrayList);
        super.onSaveInstanceState(outState);
    }
    
    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        if (savedInstanceState != null) {
            outputListViewElementArrayList = (ArrayList<OutputListViewElement>) savedInstanceState.getSerializable(Constants.LIST_ITEMS_ID);
            adapter = new ListViewAdapter(this, outputListViewElementArrayList);
            resultListView = (ListView) findViewById(R.id.resultsListView);
            resultListView.setAdapter(adapter);
        }
        super.onRestoreInstanceState(savedInstanceState);
    }
    
    private void init() {
        Log.v("storage path", Environment.getExternalStorageDirectory().toString());
        sharedPreferences = getSharedPreferences(Constants.I_GET_PREFERENCES, MODE_PRIVATE);
        urlEditText = (EditText) findViewById(R.id.urlEditText);
        urlEditText.setText(sharedPreferences.getString(ResourcesEnumerator.URL.key(), Constants.DEFAULT_URL));
        downloadPathEditText = (EditText) findViewById(R.id.downloadPathEditText);
        downloadPathEditText.setText(sharedPreferences.getString(ResourcesEnumerator.DOWNLOAD_PATH.key(), Constants.DEFAULT_DOWNLOAD_PATH));
        downloadButton = (Button) findViewById(R.id.downloadButton);
        downloadButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                SharedPreferences.Editor sharedPreferencesEditor = getSharedPreferences(Constants.I_GET_PREFERENCES, MODE_PRIVATE).edit();
                sharedPreferencesEditor.putString(ResourcesEnumerator.URL.key(), urlEditText.getText().toString());
                sharedPreferencesEditor.putString(ResourcesEnumerator.DOWNLOAD_PATH.key(), downloadPathEditText.getText().toString());
                sharedPreferencesEditor.commit();
                String[] urlSplitted = urlEditText.getText().toString().split(File.separator);
                File downloadPath = new File(downloadPathEditText.getText().toString());
                if (!downloadPath.exists()) {
                    downloadPath.mkdirs();
                }
                Date startDate = new Date();
                String content = downloadFile(urlEditText.getText().toString());
                Date stopDate = new Date();
                
                if (content.length() > 0) {
                    String nameFile = writeToFile(content, downloadPathEditText.getText().toString(),  urlSplitted[urlSplitted.length - 1]);
                    outputListViewElementArrayList.add(0, new OutputListViewElement(urlEditText.getText().toString(), downloadPathEditText.getText().toString(), nameFile, md5(content), content.length()));
                    adapter.notifyDataSetChanged();
                } else {
                    outputListViewElementArrayList.add(0, new OutputListViewElement(urlEditText.getText().toString(), Constants.DASH, Constants.DASH, Constants.DASH, 0));
                }
            }
        });
    }
    
    private void checkEnabledPermission(String permission) {
        if (ContextCompat.checkSelfPermission(this,
                                              permission)
            != PackageManager.PERMISSION_GRANTED) {
            if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                                                                    permission)) {
            } else {
                ActivityCompat.requestPermissions(this,
                                                  new String[]{permission},
                                                  1);
            }
        }
    }
    
    private String writeToFile(String content, String path, String nameFile) {
        try {
            nameFile = checkGenerateNameFile(path, nameFile.trim());
            PrintStream out = new PrintStream(new FileOutputStream(path + File.separator + nameFile));
            out.print(content);
        } catch (FileNotFoundException e) {
            Log.v(TAG, e.toString());
        }
        return nameFile;
    }
    
    private String checkGenerateNameFile(String path, String nameFile) {
        String newNameFile = nameFile.trim();
        
        File file;
        int count = 1;
        do {
            file = new File(path + File.separator + newNameFile);
            
            
            if (file.exists()) {
                
                newNameFile = nameFile.trim() + Constants.UNDERSCORE + Integer.toString(count);
                count++;
            }
        } while (file.exists());
        return newNameFile;
    }
    
}
