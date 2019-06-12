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

package io.fd.viper;
import org.qtproject.qt5.android.bindings.QtActivity;
import android.content.Context;
import android.content.Intent;
import android.app.PendingIntent;
import android.util.Log;
import android.os.Bundle;
import android.view.WindowManager;
import android.content.pm.ActivityInfo;

public class ViperActivity extends QtActivity
{
    private final static String TAG = "ViperPlayer";
    private static String m_request_url;
    private static ViperActivity m_instance;
    public ViperActivity() {
        m_instance = this;
    }
    public static String getUrl() {
        return m_instance.m_request_url;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        Intent intent = getIntent();
        String action = intent.getAction();

        if (intent.ACTION_VIEW.equals(action)) {
            m_request_url = intent.getDataString();
        }
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }
}
