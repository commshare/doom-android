package com.xianle.doomtnt;

import android.os.Bundle;
import android.preference.PreferenceActivity;

public class MyPreferenceActivity extends PreferenceActivity {
	
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		this.addPreferencesFromResource(R.xml.preferences);
	}
}
