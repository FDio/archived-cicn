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

package com.metis.ccnx.ccnxsdk.metiscontrol;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import com.metis.ccnx.ccnxsupportlibrary.Metis;
import android.widget.TextView;

import java.io.IOException;
import java.io.OutputStream;
import java.net.Socket;

public class MetisService extends Service {
    private final static String TAG = "CCNx.MetisService";

    private static Thread sForwarderThread = null;

    public MetisService() {
    }

    private String path;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        Metis metis = Metis.getInstance();
        if (!metis.isRunning()) {
            Log.d(TAG, "Starting Metis");
            String path = null;
            if (intent != null &&  intent.getExtras() != null &&  intent.getExtras().get("path") != null) {

                path = intent.getExtras().get("path").toString();
                startForwarder(intent, path);
            } else {
                //TextView mPathTextView = (TextView) view.findViewById(R.id.pathText);
                startForwarder(intent, "/storage/emulated/0/MetisConf/metis.cfg".toString());
            }
        } else {
            Log.d(TAG, "Metis already running.");
        }
        // Tell Android we want it restarted if it dies or is killed by the OS.
        return Service.START_STICKY;
    }


    @Override
    public void onDestroy() {
        //get Metis instance
        Metis metis = Metis.getInstance();
        Log.d(TAG, "Destroying");
        if (metis.isRunning()) {
            Log.d(TAG, "Trying to stop Metis: " + metis.toString());
            metis.stop();
            stopForeground(true);
        }
        super.onDestroy();
    }

    protected Runnable mForwarderRunner = new Runnable() {

        //private String path;
        @Override
        public void run() {
            Metis metis = Metis.getInstance();

            metis.start(path);
        }


    };


    private void startForwarder(Intent intent, String path) {

        int NOTIFICATION_ID = 12345;

        Metis metis = Metis.getInstance();
        if (!metis.isRunning()) {
            this.path = path;
            sForwarderThread = new Thread(mForwarderRunner, "CCNx.MetisRunner");
            sForwarderThread.start();
            metis.isRunning();
            Intent resultIntent = new Intent(this, ForwarderStatusActivity.class);

            PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, resultIntent, 0);

            Notification notification = new Notification.Builder(getApplicationContext())
                    .setContentTitle(getString(R.string.app_name))
                    .setContentText("Metis is running")
                    // .setSmallIcon(R.drawable.ic_notification)
                    .setSmallIcon(R.mipmap.ic_notification)
                    .setWhen(System.currentTimeMillis())
                    .setContentIntent(pendingIntent)
                    .build();

            notification.flags |= Notification.FLAG_NO_CLEAR;

            startForeground(NOTIFICATION_ID, notification);
        }
    }


}
