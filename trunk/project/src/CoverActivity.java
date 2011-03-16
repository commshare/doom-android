package com.xianle.doomtnt;


import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;

import android.view.animation.Animation;
import android.view.animation.AnimationUtils;

import com.admob.android.ads.AdManager;
import com.admob.android.ads.AdView;

public class CoverActivity extends Activity {
    private View mTicker;
    
    private Animation mFadeInAnimation;
	@Override
	public void onCreate(Bundle bundle) {
		super.onCreate(bundle);
		this.requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				   WindowManager.LayoutParams.FLAG_FULLSCREEN); 
		
		setContentView(R.layout.cover);
        mFadeInAnimation = AnimationUtils.loadAnimation(this, R.anim.fade_in);

        mTicker = findViewById(R.id.ticker);
        if (mTicker != null) {
        	mTicker.setFocusable(true);
        	mTicker.requestFocus();
        	mTicker.setSelected(true);
        }
        
		 Button button = (Button) findViewById(R.id.game_quick_start);
         button.setOnClickListener(new View.OnClickListener() {
             public void onClick(View v) {
                 // Perform action on click
            	 Intent i = new Intent(CoverActivity.this, MainActivity.class);
            	 CoverActivity.this.startActivity(i);	
             }
         });
         button = (Button)this.findViewById(R.id.game_options);
         button.setOnClickListener(new View.OnClickListener() {
             public void onClick(View v) {
                 // Perform action on click
            	 Intent i = new Intent(CoverActivity.this, MyPreferenceActivity.class);
            	 CoverActivity.this.startActivity(i);	
             }
         });
         button = (Button)this.findViewById(R.id.game_help);
         button.setOnClickListener(new View.OnClickListener() {
             public void onClick(View v) {
                 // Perform action on click
            	 Utils.MessageBox(CoverActivity.this, CoverActivity.this.getResources().getString(R.string.help),
            			 CoverActivity.this.getResources().getString(R.string.help_content));
             }
         });
         
         button = (Button)this.findViewById(R.id.game_exit);
         button.setOnClickListener(new View.OnClickListener() {
             public void onClick(View v) {
                 // Perform action on click
            	finish();
             }
         });
        AdManager.setTestDevices( new String[] { AdManager.TEST_EMULATOR } );
        AdView adView = (AdView)findViewById(R.id.ad);
 		adView.requestFreshAd();
	}

    
    @Override
    protected void onResume() {
        super.onResume();
        if (mTicker != null) {
        	mTicker.clearAnimation();
        	mTicker.setAnimation(mFadeInAnimation);
        }
    }
}
