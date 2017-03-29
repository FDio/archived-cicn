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

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v7.app.ActionBar;
import android.support.v7.app.ActionBarActivity;
import android.view.Menu;
import android.view.MenuItem;

import com.intel.jndn.management.types.FaceStatus;
import com.intel.jndn.management.types.RibEntry;

import com.iget.ccnx.utils.G;

import java.util.ArrayList;

/**
 * Main activity that is loaded for the NFD app.
 */
public class MainActivity extends ActionBarActivity
implements DrawerFragment.DrawerCallbacks
{
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        FragmentManager fragmentManager = getSupportFragmentManager();
        
        if (savedInstanceState != null) {
            m_drawerFragment = (DrawerFragment)fragmentManager.findFragmentByTag(DrawerFragment.class.toString());
        }
        
        if (m_drawerFragment == null) {
            ArrayList<DrawerFragment.DrawerItem> items = new ArrayList<DrawerFragment.DrawerItem>();
            
            items.add(new DrawerFragment.DrawerItem(R.string.drawer_item_iget, 0,
                                                    DRAWER_ITEM_IGET));

            m_drawerFragment = DrawerFragment.newInstance(items);
            
            fragmentManager
            .beginTransaction()
            .replace(R.id.navigation_drawer, m_drawerFragment, DrawerFragment.class.toString())
            .commit();
        }
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        G.Log("onCreateOptionsMenu" + String.valueOf(m_drawerFragment.shouldHideOptionsMenu()));
        if (!m_drawerFragment.shouldHideOptionsMenu()) {
            updateActionBar();
            return super.onCreateOptionsMenu(menu);
        }
        else
            return true;
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        return super.onOptionsItemSelected(item);
    }
    
    //////////////////////////////////////////////////////////////////////////////
    
    /**
     * Convenience method that updates and display the current title in the Action Bar
     */
    @SuppressWarnings("deprecation")
    private void updateActionBar() {
        ActionBar actionBar = getSupportActionBar();
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_STANDARD);
        actionBar.setDisplayShowTitleEnabled(true);
        if (m_actionBarTitleId != -1) {
            actionBar.setTitle(m_actionBarTitleId);
        }
    }
    
    /**
     * Convenience method that replaces the main fragment container with the
     * new fragment and adding the current transaction to the backstack.
     *
     * @param fragment Fragment to be displayed in the main fragment container.
     */
    private void replaceContentFragmentWithBackstack(Fragment fragment) {
        FragmentManager fragmentManager = getSupportFragmentManager();
        fragmentManager.beginTransaction()
        .setTransition(FragmentTransaction.TRANSIT_FRAGMENT_OPEN)
        .replace(R.id.main_fragment_container, fragment)
        .addToBackStack(null)
        .commit();
    }
    
    //////////////////////////////////////////////////////////////////////////////
    
    @Override
    public void
    onDrawerItemSelected(int itemCode, int itemNameId) {
        
        String fragmentTag = "net.named-data.nfd.content-" + String.valueOf(itemCode);
        FragmentManager fragmentManager = getSupportFragmentManager();
        
        // Create fragment according to user's selection
        Fragment fragment = fragmentManager.findFragmentByTag(fragmentTag);
        if (fragment == null) {
            switch (itemCode) {
                case DRAWER_ITEM_IGET:
                    fragment = MainFragment.newInstance();
                    break;
                
                default:
                    // Invalid; Nothing else needs to be done
                    return;
            }
        }
        
        // Update ActionBar title
        m_actionBarTitleId = itemNameId;
        
        fragmentManager.beginTransaction()
        .replace(R.id.main_fragment_container, fragment, fragmentTag)
        .commit();
    }
    

    
    
    
    //////////////////////////////////////////////////////////////////////////////
    
    /** Reference to drawer fragment */
    private DrawerFragment m_drawerFragment;
    
    /** Title that is to be displayed in the ActionBar */
    private int m_actionBarTitleId = -1;
    
    /** Item code for drawer items: For use in onDrawerItemSelected() callback */
    public static final int DRAWER_ITEM_IGET = 1;
    public static final int DRAWER_ITEM_FACES = 2;
    public static final int DRAWER_ITEM_ROUTES = 3;
    public static final int DRAWER_ITEM_STRATEGIES = 4;
    public static final int DRAWER_ITEM_LOGCAT = 5;
}
