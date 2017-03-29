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

import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.support.design.widget.Snackbar;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.CompoundButton;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;

import com.metis.ccnx.ccnxsupportlibrary.Metis;

import org.json.JSONException;
import org.json.JSONObject;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.nio.charset.Charset;
import java.util.List;


public class MetisStatusFragment extends Fragment implements IMetisNamedFragment {

    private static final String ARG_PAGER_INDEX = "metisstatus_pager_number";
    private static final String TAG = "CCNXMetis SF";

    // TODO: Rename and change types of parameters
    private int mPagerIndex;

    private Switch mSwitchMetisOnOff = null;
    private Spinner mSpinnerLogLevel = null;
    private Switch mSwitchContentStoreOnOff = null;
    private TextView mTVNumInterests = null;
    private TextView mTVNumContentObjects = null;
    private TextView mTVNumInterestReturns = null;
    private TextView mTVNumControlMessages = null;
    private TextView mTVNumPITEntries = null;
    private TextView mPathTextView = null;

    // Stats counters, updated by background task.
    private long mNumInterests = 0;
    private long mNumCOs = 0;
    private long mNumInterestReturns = 0;
    private long mNumControl = 0;
    private long mNumPITENtries = 0;

    private boolean mIsStatsQueryRunning = false;


    //private PortalFactory mPortalFactory = null;

    private OnFragmentVisibleListener mListener;


    /**
     * Create a Handler and a Runnable to be called every few seconds to query
     * Metis (when running) for stats.
     */
    private Handler mStatusUpdaterHandler = new Handler();
    private Runnable mStatusUpdateRunnable = new Runnable() {
        @Override
        public void run() {
            // This runs on the main thread, so start an AsyncTask
            // Repeat this the same runnable code block again another few seconds
            //new GetStatusTask(null).execute(mPortalFactory);
            //if (mIsStatsQueryRunning) {
                //mStatusUpdaterHandler.postDelayed(mStatusUpdateRunnable, 2 * 1000);
            //}
        }
    };


    public MetisStatusFragment() {
        // Required empty public constructor
    }

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @return A new instance of fragment MetisStatusFragment.
     */
    // TODO: Rename and change types and number of parameters
    public static MetisStatusFragment newInstance(int pagerIndex) {
        MetisStatusFragment fragment = new MetisStatusFragment();
        Bundle args = new Bundle();
        args.putInt(ARG_PAGER_INDEX, pagerIndex);

        fragment.setArguments(args);

        return fragment;
    }

    @Override
    public void onStart() {
        Metis metis = Metis.getInstance();
        mSwitchMetisOnOff.setChecked(metis.isRunning());
        super.onStart();
    }


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getArguments() != null) {
            mPagerIndex = getArguments().getInt(ARG_PAGER_INDEX);
        }


        Log.d(TAG, "Creating new PortalFactory");
        //Identity identity = CCNxUtils.createCCNxIdentity(getContext(),
        //        "password", "ccnxsdkdemo", 1024, 30);
        //mPortalFactory = new PortalFactory(identity);
    }



    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View view = inflater.inflate(R.layout.fragment_metis_status, container, false);

        mSwitchMetisOnOff = (Switch) view.findViewById(R.id.switchMetisOnOff);
        mSwitchContentStoreOnOff = (Switch) view.findViewById(R.id.switchMetisContentStoreOnOff);
        mSpinnerLogLevel = (Spinner) view.findViewById(R.id.spinnerMetisLoggingLevel);
        mPathTextView = (TextView) view.findViewById(R.id.pathText) ;

        mSpinnerLogLevel.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
                String loggingLevel = mSpinnerLogLevel.getSelectedItem().toString();
                updateMetisLoggingLevel(loggingLevel);
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {

            }
        });

        mSwitchMetisOnOff.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    startMetis();
                    //mSpinnerLogLevel.setEnabled(true);
                    //Log.d(TAG, "################# Start periodic query for stats");
                    //if (!mIsStatsQueryRunning) {
                    //    mStatusUpdaterHandler.postDelayed(mStatusUpdateRunnable, 500);
                     //   mIsStatsQueryRunning = true;
                     //   String loggingLevel = mSpinnerLogLevel.getSelectedItem().toString();
                     //   if (!loggingLevel.equalsIgnoreCase("off")) {
                     //       updateMetisLoggingLevel(loggingLevel);
                     //   }
                    //}
                } else {
                    Log.d(TAG, "################# Stop periodic query for stats");
                    //mStatusUpdaterHandler.removeCallbacks(mStatusUpdateRunnable);
                    //mIsStatsQueryRunning = false;
                    stopMetis();
                    //mSpinnerLogLevel.setEnabled(false);
                    //mSpinnerLogLevel.setSelection(0);
                }
            }
        });

        mTVNumInterests = (TextView) view.findViewById(R.id.tvStatsNumInterests);
        mTVNumContentObjects = (TextView) view.findViewById(R.id.tvStatsNumContentObjects);
        mTVNumInterestReturns = (TextView) view.findViewById(R.id.tvStatsNumInterestReturns);
        mTVNumControlMessages = (TextView) view.findViewById(R.id.tvStatsNumControl);
        mTVNumPITEntries = (TextView) view.findViewById(R.id.tvStatsPITSize);

        mTVNumInterests.setText(String.valueOf(mNumInterests));
        mTVNumContentObjects.setText(String.valueOf(mNumCOs));
        mTVNumControlMessages.setText(String.valueOf(mNumControl));
        mTVNumInterestReturns.setText(String.valueOf(mNumInterestReturns));
        mTVNumPITEntries.setText("");

        return view;
    }

    private void startMetis() {
        mPathTextView.setEnabled(false);
        Metis metis = Metis.getInstance();
        if (!metis.isRunning()) {
            Intent intent = new Intent(getActivity(), MetisService.class);
            intent.putExtra("path", mPathTextView.getText());
            getActivity().startService(intent);

            new Handler().postDelayed(new Runnable() {
                @Override
                public void run() {
                    createExternalListeners();
                }
            }, 1000);
        }
    }

    private void stopMetis() {
        mPathTextView.setEnabled(true);
        Intent intent = new Intent(getActivity(), MetisService.class);

        getActivity().stopService(intent);

    }

    private void updateMetisLoggingLevel(String loggingLevel) {
        /*Metis metis = Metis.getInstance();
        if (metis.isRunning()) {
            // Send an Interest control message to Metis with the new logging level.
            String commandURI = MetisConstants.CCNxNameMetisCommand_Set + "/" + MetisConstants.MetisCommand_LogLevel + "/" + loggingLevel;
            Name name = new Name(commandURI);
            SendInterestTask task = new SendInterestTask(name, null, new SendInterestTask.OnInterestSentListener() {
                @Override
                public void onInterestSent(Message message) {
                    if (message instanceof ContentObject) {

                        String responseString = new String(((ContentObject) message).payload());
                        Snackbar snackbar = Snackbar
                                .make(mSwitchMetisOnOff, responseString, Snackbar.LENGTH_SHORT);

                        snackbar.show();
                    } else {
                        Log.d(TAG, "Unexpected non-Content response from sent Interest");
                    }
                }
            });

            task.execute(mPortalFactory);
        }*/
    }

    private void createExternalListeners() {

        /*Metis metis = Metis.getInstance();

        if (metis.isRunning()) {

            List<InetAddress> ipAddresses = CCNxUtils.getLocalIpAddress();

            for (InetAddress addr : ipAddresses) {

                // For the moment, just listen on the IPV4 addresses. The V6 addresses should work,
                // but it's not yet tested.

                if (addr instanceof Inet4Address) {

                    String ipAddress = addr.getHostAddress();

                    Log.d(TAG, "Adding external listener on: " + ipAddress);

                    String linkURI = "tcp://" + ipAddress + ":" + MetisConstants.MetisDefaultListenerPort + "/listener";

                    Name name = new Name(MetisConstants.CCNxNameMetisCommand_LinkConnect);

                    SendInterestTask task = new SendInterestTask(name, linkURI.getBytes(), new SendInterestTask.OnInterestSentListener() {
                        @Override
                        public void onInterestSent(Message message) {
                            if (message instanceof ContentObject) {

                                String responseString = new String(((ContentObject) message).payload());
                                Snackbar snackbar = Snackbar
                                        .make(mSwitchMetisOnOff, responseString, Snackbar.LENGTH_SHORT);

                                snackbar.show();
                            } else {
                                Log.d(TAG, "Unexpected non-Content response from sent Interest");
                            }
                        }
                    });

                    task.execute(mPortalFactory);
                }
            }
        }*/
    }


    @Override
    public void onAttach(Context context) {
        super.onAttach(context);
        if (context instanceof OnFragmentVisibleListener) {
            mListener = (OnFragmentVisibleListener) context;
        } else {
            throw new RuntimeException(context.toString()
                    + " must implement OnFragmentInteractionListener");
        }
    }


    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        super.setUserVisibleHint(isVisibleToUser);
        Metis metis = Metis.getInstance();

        if (isVisibleToUser) {
            mListener.onFragmentVisible(this);

            if (metis.isRunning()) {
                // Begin updating stats.
                if (!mIsStatsQueryRunning) {
                    mStatusUpdaterHandler.postDelayed(mStatusUpdateRunnable, 100);
                    mIsStatsQueryRunning = true;
                }
            }
        } else {
            mStatusUpdaterHandler.removeCallbacks(mStatusUpdateRunnable);
            mIsStatsQueryRunning = false;
        }
    }

    @Override
    public void onDetach() {
        mStatusUpdaterHandler.removeCallbacks(mStatusUpdateRunnable);
        mIsStatsQueryRunning = false;
        super.onDetach();
        mListener = null;
    }

    @Override
    public String getFragmentName() {
        return "Status";
    }

    public interface OnFragmentVisibleListener {
        // TODO: Update argument type and name
        void onFragmentVisible(Fragment which);
    }

    /*private class GetStatusTask extends AsyncTask<PortalFactory, String, Integer> {

        private boolean mSuccess = false;
        private PortalFactory mPortalFactory = null;


        public GetStatusTask(String unused) {
        }

        @Override
        protected Integer doInBackground(PortalFactory... args) {
            Thread.currentThread().setName("GetStatusTask-Async");

            mPortalFactory = args[0];
            try {
                Name controlName = new Name(MetisConstants.CCNxNameMetisCommand_Stats);

                Interest interest = new Interest(controlName);

                try {
                    Portal portal = mPortalFactory.getPortal();

                    portal.send(interest, 0L);

                    Message m = portal.receive(0L);

                    if (m instanceof ContentObject) {
                        mSuccess = true;
                        ContentObject co = (ContentObject) m;
                        byte[] payload = co.payload();

                        if (payload != null) {
                            String jsonString = new String(payload, Charset.defaultCharset());
                            //Log.d(TAG, "Received: XX " + jsonString + " XX");
                            try {
                                JSONObject jo = new JSONObject(jsonString);
                                //Log.d(TAG, "JSON2: " + jo.toString(2));

                                mNumInterests = jo.getLong("numProcessedInterests");
                                mNumCOs = jo.getLong("numProcessedContentObjects");
                                mNumControl = jo.getLong("numProcessedControlMessages");
                                mNumInterestReturns = jo.getLong("numProcessedInterestReturns");
                            } catch (JSONException ex) {
                                Log.e(TAG, "Could not parse returned JSON: " + ex.getMessage());
                            }
                        }
                    }
                    portal.close();
                } catch (Portal.CommunicationsError ex) {
                    Log.e(TAG, "Error sending AddLink command: " + ex.getMessage());
                }
            } catch (Exception ex) {
                Log.e(TAG, "Error adding link: " + ex.getMessage());
            }

            return 1;
        }

        @Override
        protected void onPostExecute(Integer ignored) {

            if (mSuccess) {
                mTVNumInterests.setText(String.valueOf(mNumInterests));
                mTVNumContentObjects.setText(String.valueOf(mNumCOs));
                mTVNumControlMessages.setText(String.valueOf(mNumControl));
                mTVNumInterestReturns.setText(String.valueOf(mNumInterestReturns));
                mTVNumPITEntries.setText("");
            }
        }
    }*/

}
