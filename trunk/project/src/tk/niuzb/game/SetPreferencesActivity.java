package tk.niuzb.game;
/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Toast;
import android.view.KeyEvent;

import com.xianle.doomtnt.R;


public class SetPreferencesActivity extends PreferenceActivity implements 
YesNoDialogPreference.YesNoDialogListener {
	public final static String PREFERENCE_NAME = "doom";
    public static final String PREFERENCE_LEFT_KEY = "keyLeft";
    public static final String PREFERENCE_RIGHT_KEY = "keyRight";
    public static final String PREFERENCE_UP_KEY = "keyUp";
    public static final String PREFERENCE_DOWN_KEY = "keyDown";
    public static final String PREFERENCE_FIRE_KEY = "keyFire";
    public static final String PREFERENCE_DOOR_KEY = "keyDoor";
    public static final String PREFERENCE_TLEFT_KEY = "keyTLeft";
    public static final String PREFERENCE_TRIGHT_KEY = "keyTRight";
	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
     
		
        getPreferenceManager().setSharedPreferencesMode(MODE_PRIVATE);
        getPreferenceManager().setSharedPreferencesName(PREFERENCE_NAME);
        
        // Load the preferences from an XML resource
        addPreferencesFromResource(R.xml.preferences);
        
      
        Preference configureKeyboardPref = getPreferenceManager().findPreference("keyconfig");
        if (configureKeyboardPref != null) {
        	KeyboardConfigDialogPreference config = (KeyboardConfigDialogPreference)configureKeyboardPref;
        	config.setPrefs(getSharedPreferences(PREFERENCE_NAME, MODE_PRIVATE));
        	config.setContext(this);
        }
        
        Preference eraseGameButton = 
        getPreferenceManager().findPreference("resetgame");
        if (eraseGameButton != null) {
            YesNoDialogPreference yesNo = (YesNoDialogPreference)eraseGameButton;
            yesNo.setListener(this);
        }
    }
    public void onDialogClosed(boolean positiveResult) {
        if (positiveResult) {
            SharedPreferences prefs = getSharedPreferences(PREFERENCE_NAME, MODE_PRIVATE);
            SharedPreferences.Editor editor = prefs.edit();
            editor.putInt(PREFERENCE_LEFT_KEY, KeyEvent.KEYCODE_DPAD_LEFT);
            editor.putInt(PREFERENCE_RIGHT_KEY, KeyEvent.KEYCODE_DPAD_RIGHT);
            editor.putInt(PREFERENCE_UP_KEY, KeyEvent.KEYCODE_DPAD_UP);
            editor.putInt(PREFERENCE_DOWN_KEY, KeyEvent.KEYCODE_DPAD_DOWN);
            editor.putInt(PREFERENCE_FIRE_KEY, KeyEvent.KEYCODE_SEARCH);
            editor.putInt(PREFERENCE_DOOR_KEY, KeyEvent.KEYCODE_SPACE);
            editor.putInt(PREFERENCE_TLEFT_KEY, KeyEvent.KEYCODE_Q);
            editor.putInt(PREFERENCE_TRIGHT_KEY, KeyEvent.KEYCODE_E);
            editor.commit();
            Toast.makeText(this, "keys has already been reset to default",
                    Toast.LENGTH_SHORT).show();
        }
    }

	
}
