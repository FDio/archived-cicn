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

package icn.iget.com.igetandroid;

import android.Manifest;
import android.app.Dialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.concurrent.Executors;

import icn.iget.com.adapter.ListViewAdapter;
import icn.iget.com.adapter.OutputListViewElement;
import icn.iget.com.igetandroid.R;
import icn.iget.com.utility.Constants;
import icn.iget.com.utility.ResourcesEnumerator;

public class IGetAndroidActivity extends AppCompatActivity {

    private static String TAG = "IGetAndroidAcrivity";

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("IGetWrapper");
    }

    ArrayList<OutputListViewElement> outputListViewElementArrayList = new ArrayList<OutputListViewElement>();
    ListViewAdapter adapter;


    static int fCount = 0;
    SharedPreferences sharedPreferences;
    EditText urlEditText;
    EditText downloadPathEditText;
    Button downloadButton;
    Button stopButton;
    ListView resultListView;
    Button yesButtonDialog;
    Button noButtonDialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_i_get_android);
        adapter = new ListViewAdapter(this, outputListViewElementArrayList);
        resultListView = (ListView) findViewById(R.id.resultsListView);
        resultListView.setAdapter(adapter);
        checkEnabledPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        checkEnabledPermission(Manifest.permission.READ_EXTERNAL_STORAGE);
        if (!checkMetis(Constants.METIS_ID)) {
            final Dialog dialog = new Dialog(this);
            dialog.setContentView(R.layout.popup_message);
            yesButtonDialog = (Button) dialog.findViewById(R.id.yesButtonDialog);
            noButtonDialog = (Button) dialog.findViewById(R.id.noButtonDialog);
            yesButtonDialog.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    try {
                        startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id=" + Constants.METIS_ID)));
                    } catch (android.content.ActivityNotFoundException anfe) {
                        startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("https://play.google.com/store/apps/details?id=" + Constants.METIS_ID)));
                    }
                    dialog.hide();
                }
            });
            noButtonDialog.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    dialog.hide();
                }
            });
            dialog.show();

        }
        init();
    }

    public static final String md5(final byte[] s) {
        final String MD5 = "MD5";
        try {
            // Create MD5 Hash
            MessageDigest digest = java.security.MessageDigest
                    .getInstance(MD5);
            digest.update(s);
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

                urlEditText.setEnabled(false);
                downloadPathEditText.setEnabled(false);
                downloadButton.setEnabled(false);
                stopButton.setEnabled(true);
                Executors.newSingleThreadExecutor().execute(new Runnable() {
                    @Override
                    public void run() {

                        SharedPreferences.Editor sharedPreferencesEditor = getSharedPreferences(Constants.I_GET_PREFERENCES, MODE_PRIVATE).edit();
                        sharedPreferencesEditor.putString(ResourcesEnumerator.URL.key(), urlEditText.getText().toString());
                        sharedPreferencesEditor.putString(ResourcesEnumerator.DOWNLOAD_PATH.key(), downloadPathEditText.getText().toString());
                        sharedPreferencesEditor.commit();
                        String[] urlSplitted = urlEditText.getText().toString().split(File.separator);
                        File downloadPath = new File(downloadPathEditText.getText().toString());
                        if (!downloadPath.exists()) {
                            downloadPath.mkdirs();
                        }

                        byte[] content = downloadFile(urlEditText.getText().toString());
                        if (content.length > 0) {
                            String nameFile = writeToFile(content, downloadPathEditText.getText().toString(), urlSplitted[urlSplitted.length - 1]);
                            outputListViewElementArrayList.add(0, new OutputListViewElement(urlEditText.getText().toString(), downloadPathEditText.getText().toString(), nameFile, md5(content), content.length));
                        } else {
                            outputListViewElementArrayList.add(0, new OutputListViewElement(urlEditText.getText().toString(), Constants.DASH, Constants.DASH, Constants.DASH, 0));
                        }
                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                adapter.notifyDataSetChanged();
                                urlEditText.setEnabled(true);
                                downloadPathEditText.setEnabled(true);
                                downloadButton.setEnabled(true);
                                stopButton.setEnabled(false);
                            }
                        });
                    }
                });


            }
        });
        stopButton = (Button) findViewById(R.id.stopButton);
        stopButton.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View view) {
                stopDownload();
            }
        });
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


    public native void stopDownload();

    public native byte[] downloadFile(String path);


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

    private String writeToFile(byte[] content, String path, String nameFile) {
        try {
            Log.v("name", nameFile);
            nameFile = checkGenerateNameFile(path, nameFile.trim());
            Log.v("name", nameFile);

            FileOutputStream fos = new FileOutputStream(path + File.separator + nameFile);
            fos.write(content);
            fos.close();

        } catch (FileNotFoundException e) {
            Log.v(TAG, e.toString());
        } catch (IOException e) {
            Log.v(TAG, e.toString());
        }
        return nameFile;
    }

   /* private void checkMetis() {
        boolean isAppInstalled = appInstalledOrNot("icn.forwarder.com.icnforwarderandroid");


        final String appPackageName = getPackageName(); // getPackageName() from Context or Activity object
        try {
            startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("market://details?id=icn.forwarder.com.icnforwarderandroid")));
        } catch (android.content.ActivityNotFoundException anfe) {
            startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("https://play.google.com/store/apps/details?id=icn.forwarder.com.icnforwarderandroid")));
        }
        if (isAppInstalled) {
            //This intent will help you to launch if the package is already installed
            Log.i("iget", "Application is already installed.");
        } else {
            // Do whatever we want to do if application not installed
            // For example, Redirect to play store

            Log.i("iget", "Application is not currently installed.");
        }
    }*/

    private boolean checkMetis(String uri) {
        PackageManager pm = getPackageManager();
        try {
            pm.getPackageInfo(uri, PackageManager.GET_ACTIVITIES);
            return true;
        } catch (PackageManager.NameNotFoundException e) {
            return false;
        }
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
