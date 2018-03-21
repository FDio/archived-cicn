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

package icn.httpserver.com.service;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.IBinder;
import android.util.Log;

import java.io.File;

import icn.httpserver.com.httpserver.HttpServerActivity;
import icn.httpserver.com.supportlibrary.HttpServer;
import icn.httpserver.com.utility.Constants;
import icn.httpserver.com.utility.ResourcesEnumerator;

/**
 * Created by angelomantellini on 12/11/17.
 */


public class HttpServerService extends Service {
    private final static String TAG = "HttpServerService";

    private static Thread sHttpServerThread = null;

    public HttpServerService() {
    }

    private String rootFolderString;
    private String tcpListenPortString;
    private String webServerPrefixString;
    private String proxyAddressString;
    private String iCNproxyAddressString;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        HttpServer httpServer = HttpServer.getInstance();
        if (!httpServer.isRunning()) {
            Log.d(TAG, "Starting Http Server");
            SharedPreferences sharedPreferences = getSharedPreferences(Constants.HTTP_SERVER_PREFERENCES, MODE_PRIVATE);
            String rootFolderString = sharedPreferences.getString(ResourcesEnumerator.ROOT_FOLDER.key(),
                    null);
            String tcpListenPortString = sharedPreferences.getString(ResourcesEnumerator.TCP_LISTEN_PORT.key(), null);
            String webServerPrefixString = sharedPreferences.getString(ResourcesEnumerator.WEBSERVER_PREFIX.key(), null);
            String proxyAddressString = sharedPreferences.getString(ResourcesEnumerator.PROXY_ADDRESS.key(), null);
            String iCNproxyAddressString = sharedPreferences.getString(ResourcesEnumerator.ICN_PROXY_ADDRESS.key(), null);
            File folder = new File(rootFolderString);
            if (!folder.exists()) {
                folder.mkdirs();
            }


            startHttpServer(intent, rootFolderString, tcpListenPortString, webServerPrefixString, proxyAddressString, iCNproxyAddressString);


        } else {
            Log.d(TAG, "Http Server is already running.");
        }
        return Service.START_STICKY;
    }


    @Override
    public void onDestroy() {
        HttpServer httpServer = HttpServer.getInstance();
        Log.d(TAG, "Destroying HttpServer");
        if (httpServer.isRunning()) {
            httpServer.stop();
            stopForeground(true);
        }
        super.onDestroy();
    }

    protected Runnable mHttpServerRunner = new Runnable() {

        //private String path;
        @Override
        public void run() {
            HttpServer httpServer = HttpServer.getInstance();
            httpServer.start(rootFolderString, tcpListenPortString, webServerPrefixString, proxyAddressString, iCNproxyAddressString);
        }


    };

    private void startHttpServer(Intent intent, String rootFolderString, String tcpListenPortString, String webServerPrefixString, String proxyAddressString, String iCNproxyAddressString) {
        String NOTIFICATION_CHANNEL_ID = "12345";
        Notification.Builder notificationBuilder = null;
        if (Build.VERSION.SDK_INT >= 26) {
            notificationBuilder = new Notification.Builder(this, NOTIFICATION_CHANNEL_ID);
        } else {
            notificationBuilder = new Notification.Builder(this);
        }
        Intent notificationIntent = new Intent(this, HttpServerActivity.class);
        PendingIntent activity = PendingIntent.getActivity(this, 0, notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        notificationBuilder.setContentTitle("HttpServer").setContentText("HttpServer").setOngoing(true).setContentIntent(activity);
        Notification notification = notificationBuilder.build();

        if (Build.VERSION.SDK_INT >= 26) {
            NotificationChannel channel = new NotificationChannel(NOTIFICATION_CHANNEL_ID, "HttpServer", NotificationManager.IMPORTANCE_DEFAULT);
            channel.setDescription("HttpServer");
            NotificationManager notificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
            notificationManager.createNotificationChannel(channel);

        }

        startForeground(Constants.FOREGROUND_SERVICE, notification);


        HttpServer httpServer = HttpServer.getInstance();
        if (!httpServer.isRunning()) {
            this.rootFolderString = rootFolderString;
            this.tcpListenPortString = tcpListenPortString;
            this.webServerPrefixString = webServerPrefixString;
            this.proxyAddressString = proxyAddressString;
            this.iCNproxyAddressString = iCNproxyAddressString;
            sHttpServerThread = new Thread(mHttpServerRunner, "HttpServerRunner");
            sHttpServerThread.start();
        }


        Log.e(TAG, "HttpServer Started");

    }


}
