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

package com.iget.ccnx;

import android.annotation.SuppressLint;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import com.intel.jndn.management.types.ForwarderStatus;
import com.iget.ccnx.service.CCNxService;
import com.iget.ccnx.utils.G;


public class MainFragment extends Fragment {

  public static MainFragment newInstance() {
    return new MainFragment();
  }

  @Override
  public void onCreate(Bundle savedInstanceState)
  {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(LayoutInflater inflater,
                           @Nullable ViewGroup container,
                           @Nullable Bundle savedInstanceState)
  {
    @SuppressLint("InflateParams")
    View v =  inflater.inflate(R.layout.fragment_main, null);
    m_downloadPathEditText = (EditText)v.findViewById(R.id.downloadPathEditText);
    m_destinationPathEditText = (EditText)v.findViewById(R.id.destinationPathEditText);
    m_startIGet = (Button)v.findViewById(R.id.start_iget);
    m_startIGet.setOnClickListener(new View.OnClickListener() {

      @Override
      public void onClick(View arg0) {
        G.Log("start iget");
        m_downloadPathEditText.setEnabled(false);
        m_destinationPathEditText.setEnabled(false);
        String string = m_destinationPathEditText.getText().toString();
        G.Log("adsasd" + string);
        CCNxService.startIGet(m_downloadPathEditText.getText().toString(), m_destinationPathEditText.getText().toString());
        m_downloadPathEditText.setEnabled(true);
        m_destinationPathEditText.setEnabled(true);

      }

    });
    return v;
  }

  private Button m_startIGet;
  private EditText m_downloadPathEditText;
  private EditText m_destinationPathEditText;

}
