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
package com.metis.ccnx.service;

import android.app.Notification;
import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.util.Log;

import com.metis.ccnx.supportlibrary.MetisForwarder;
import com.metis.ccnx.utility.Constants;
import com.metis.ccnx.utility.ResourcesEnumerator;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;

public class MetisForwarderService extends Service {
    private final static String TAG = "MetisForwarderService";

    private static Thread sForwarderThread = null;

    public MetisForwarderService() {
    }

    private String path;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        MetisForwarder metisForwarder = MetisForwarder.getInstance();
        if (!metisForwarder.isRunning()) {
            Log.d(TAG, "Starting Metis Forwarder");
            SharedPreferences sharedPreferences = getSharedPreferences(Constants.METIS_FORWARDER_PREFERENCES, MODE_PRIVATE);
            String configuration = sharedPreferences.getString(ResourcesEnumerator.CONFIGURATION.key(), Constants.DEFAULT_CONFIGURATION);
            String sourceIp = sharedPreferences.getString(ResourcesEnumerator.SOURCE_IP.key(), null);
            String sourcePort = sharedPreferences.getString(ResourcesEnumerator.SOURCE_PORT.key(), null);
            String nextHopIp = sharedPreferences.getString(ResourcesEnumerator.NEXT_HOP_IP.key(), null);
            String nextHopPort = sharedPreferences.getString(ResourcesEnumerator.NEXT_HOP_PORT.key(), null);
            String prefix = sharedPreferences.getString(ResourcesEnumerator.PREFIX.key(), null);
            configuration = configuration.replace(Constants.SOURCE_IP, sourceIp);
            configuration = configuration.replace(Constants.SOURCE_PORT, sourcePort);
            configuration = configuration.replace(Constants.NEXT_HOP_IP, nextHopIp);
            configuration = configuration.replace(Constants.NEXT_HOP_PORT, nextHopPort);
            configuration = configuration.replace(Constants.PREFIX, prefix);
            try {
                String configurationDir = getPackageManager().getPackageInfo(getPackageName(), 0).applicationInfo.dataDir +
                        File.separator + Constants.CONFIGURATION_PATH;
                File folder = new File(configurationDir);
                if (!folder.exists()) {
                    folder.mkdirs();
                }

                writeToFile(configuration, configurationDir + File.separator + Constants.CONFIGURATION_FILE_NAME);
                startForwarder(intent, configurationDir + File.separator + Constants.CONFIGURATION_FILE_NAME);
            } catch (PackageManager.NameNotFoundException e) {
                Log.w(TAG, "Error Package name not found ", e);
            }


        } else {
            Log.d(TAG, "Metis Forwarder already running.");
        }
        return Service.START_STICKY;
    }


    @Override
    public void onDestroy() {
        MetisForwarder metisForwarder = MetisForwarder.getInstance();
        Log.d(TAG, "Destroying Metis Forwarder");
        if (metisForwarder.isRunning()) {
            metisForwarder.stop();
            stopForeground(true);
        }
        super.onDestroy();
    }

    protected Runnable mForwarderRunner = new Runnable() {

        //private String path;
        @Override
        public void run() {
            MetisForwarder metisForwarder = MetisForwarder.getInstance();
            metisForwarder.start(path);
        }


    };

    private boolean writeToFile(String data, String path) {
        Log.v(TAG, path + " " + data);
        try (Writer writer = new BufferedWriter(new OutputStreamWriter(
                new FileOutputStream(path), "utf-8"))) {
            writer.write(data);
            return true;
        } catch (IOException e) {
            Log.e(TAG, "File write failed: " + e.toString());
            return false;
        }
    }


    private void startForwarder(Intent intent, String path) {

        int NOTIFICATION_ID = 12345;
        startForeground(Constants.FOREGROUND_SERVICE, new Notification.Builder(this).build());
        MetisForwarder metisForwarder = MetisForwarder.getInstance();
        if (!metisForwarder.isRunning()) {
           this.path = path;
            sForwarderThread = new Thread(mForwarderRunner, "MetisForwarderRunner");
            sForwarderThread.start();
        }



    }



}
