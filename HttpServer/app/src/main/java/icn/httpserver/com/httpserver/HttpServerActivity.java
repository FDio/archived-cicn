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

package icn.httpserver.com.httpserver;

import android.Manifest;
import android.app.Dialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.net.Uri;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Switch;

import icn.httpserver.com.service.HttpServerService;
import icn.httpserver.com.supportlibrary.HttpServer;
import icn.httpserver.com.utility.Constants;
import icn.httpserver.com.utility.ResourcesEnumerator;

public class HttpServerActivity extends AppCompatActivity {
    private EditText rootFolderEditText;
    private EditText tcpListenPortEditText;
    private EditText webServerPrefixEditText;
    private EditText proxyAddressEditText;
    private EditText iCNproxyAddressEditText;
    private Switch httpServerSwitch;
    private Button yesButtonDialog;
    private Button noButtonDialog;


    private SharedPreferences sharedPreferences;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_http_server);
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

    private boolean checkMetis(String uri) {
        PackageManager pm = getPackageManager();
        try {
            pm.getPackageInfo(uri, PackageManager.GET_ACTIVITIES);
            return true;
        } catch (PackageManager.NameNotFoundException e) {
            return false;
        }
    }

    private void init() {
        sharedPreferences = getSharedPreferences(Constants.HTTP_SERVER_PREFERENCES, MODE_PRIVATE);
        rootFolderEditText = (EditText) findViewById(R.id.rootFolderEditText);
        rootFolderEditText.setText(sharedPreferences.getString(ResourcesEnumerator.ROOT_FOLDER.key(), Constants.DEFAULT_ROOT_FOLDER));
        tcpListenPortEditText = (EditText) findViewById(R.id.tcpListenPortEditText);
        tcpListenPortEditText.setText(sharedPreferences.getString(ResourcesEnumerator.TCP_LISTEN_PORT.key(), Constants.DEFAULT_TCP_LISTEN_PORT));
        webServerPrefixEditText = (EditText) findViewById(R.id.webServerPrefixEditText);
        webServerPrefixEditText.setText(sharedPreferences.getString(ResourcesEnumerator.WEBSERVER_PREFIX.key(), Constants.DEFAULT_WEBSERVER_PREFIX));
        proxyAddressEditText = (EditText) findViewById(R.id.proxyAddressEditText);
        proxyAddressEditText.setText(sharedPreferences.getString(ResourcesEnumerator.PROXY_ADDRESS.key(), Constants.DEFAULT_PROXY_ADDRESS));
        iCNproxyAddressEditText = (EditText) findViewById(R.id.iCNproxyAddressEditText);
        iCNproxyAddressEditText.setText(sharedPreferences.getString(ResourcesEnumerator.ICN_PROXY_ADDRESS.key(), Constants.DEFAULT_ICN_PROXY_ADDRESS));
        httpServerSwitch = (Switch) findViewById(R.id.httpServerSwitch);
        if (HttpServer.getInstance().isRunning()) {
            httpServerSwitch.setChecked(true);
            rootFolderEditText.setEnabled(false);
            tcpListenPortEditText.setEnabled(false);
            webServerPrefixEditText.setEnabled(false);
            proxyAddressEditText.setEnabled(false);
            iCNproxyAddressEditText.setEnabled(false);
        }

        httpServerSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                Log.v("Switch State=", "" + isChecked);
                if (isChecked) {
                    httpServerSwitch.setText(Constants.ENABLED);
                    SharedPreferences.Editor sharedPreferencesEditor = getSharedPreferences(Constants.HTTP_SERVER_PREFERENCES, MODE_PRIVATE).edit();
                    sharedPreferencesEditor.putString(ResourcesEnumerator.ROOT_FOLDER.key(), rootFolderEditText.getText().toString());
                    sharedPreferencesEditor.putString(ResourcesEnumerator.TCP_LISTEN_PORT.key(), tcpListenPortEditText.getText().toString());
                    sharedPreferencesEditor.putString(ResourcesEnumerator.WEBSERVER_PREFIX.key(), webServerPrefixEditText.getText().toString());
                    sharedPreferencesEditor.putString(ResourcesEnumerator.PROXY_ADDRESS.key(), proxyAddressEditText.getText().toString());
                    sharedPreferencesEditor.putString(ResourcesEnumerator.ICN_PROXY_ADDRESS.key(), iCNproxyAddressEditText.getText().toString());
                    sharedPreferencesEditor.commit();
                    rootFolderEditText.setEnabled(false);
                    tcpListenPortEditText.setEnabled(false);
                    webServerPrefixEditText.setEnabled(false);
                    proxyAddressEditText.setEnabled(false);
                    iCNproxyAddressEditText.setEnabled(false);
                    startHttpServer();

                } else {
                    httpServerSwitch.setText(Constants.DISABLED);
                    rootFolderEditText.setEnabled(true);
                    tcpListenPortEditText.setEnabled(true);
                    webServerPrefixEditText.setEnabled(true);
                    proxyAddressEditText.setEnabled(true);
                    iCNproxyAddressEditText.setEnabled(true);
                    stopHttpServer();
                }
            }

        });


    }

    private void startHttpServer() {
        Intent intent = new Intent(this, HttpServerService.class);
        startService(intent);
    }

    private void stopHttpServer() {
        Intent intent = new Intent(this, HttpServerService.class);
        stopService(intent);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
    }

}
