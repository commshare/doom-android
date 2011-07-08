package com.xianle.doomtnt;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.content.SharedPreferences;

public class Utils {
	static public AlertDialog createAlertDialog (Context ctx, String title, String message) {
        return new AlertDialog.Builder(ctx)
	        .setTitle(title)
	        .setMessage(message)
	        .create();
    }
	
	public static  void MessageBox (Context ctx, String title, String text) {
		AlertDialog d = createAlertDialog(ctx
				, title
				, text);
			
		d.setButton(ctx.getResources().getString(android.R.string.ok) , new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int whichButton) {
                /* User clicked OK so do some stuff */
            }
        });
		d.show();
	}
	
	/**
	 * Load spinner
	 * @param ctx
	 * @param spinner
	 * @param resID
	 * @param idx
	 */
	private static void loadSpinner(Context ctx, Spinner spinner, int resID, int idx) {
		ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(ctx, resID
				, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
//        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        spinner.setAdapter(adapter);
        spinner.setSelection(idx);
	}
	/**
	 * Download dlg data
	 * @param ctx
	 * @param v
	 */
	private static void setDownloadDlgUI (Context ctx, View v) {
        /**
         * Load files spinner
         */
		loadSpinner(ctx, (Spinner) v.findViewById(R.id.s_files), R.array.DoomFiles, 0);
    }
	
	public static void toggleView ( View v) {
		if ( v.getVisibility() == View.VISIBLE)
			v.setVisibility(View.GONE);
		else
			v.setVisibility(View.VISIBLE);
	}
	
	/**
	 * show a toast msg
	 * @param ctx the activity handle
	 * @param text the text to be show
	 */
    public static void Toast( final Context ctx, final String text) {
	    Toast.makeText(ctx, text, Toast.LENGTH_LONG).show();
    }

	/**
	 * Game options UI
	 * @param ctx
	 * @param v
	 */
	public static void setLauncherDlgOptionsUI (final Context ctx, final View v, boolean multiPlayer) 
	{
         //Load files spinner
		final Spinner sFiles = (Spinner) v.findViewById(R.id.s_files);
		loadSpinner(ctx, sFiles , R.array.LaunchFiles, 0);
		
		sFiles.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {

			public void onItemSelected(AdapterView<?> arg0, View arg1,
					int idx, long arg3) 
			{
				if ( idx == 1) {
					// other
					TextView lbl = ((TextView)v.findViewById(R.id.tv_game));
					
					// hide spinner
					toggleView(sFiles); 
					
					// Show WAD text box
					EditText wad = (EditText)v.findViewById(R.id.t_wad);
                    SharedPreferences prefs = ctx.getSharedPreferences("doom", ctx.MODE_PRIVATE);
                    String name = prefs.getString("wadname", "tnt.wad");
					wad.setText(name);
					toggleView(wad); 
					
					// Help lbl
lbl.setText("1. Please input Data file name. make sure this file has been in "+
"directory /sdcard/doom/ \n2. "+
 "Make sure to change other data file name *.wad to *.wad.bak");
					
					
				}
			}

			public void onNothingSelected(AdapterView<?> arg0) {
			}

			
		});
		
		// mp?
		if ( multiPlayer) {
			v.findViewById(R.id.lbl_mp).setVisibility(View.VISIBLE);
			v.findViewById(R.id.t_server).setVisibility(View.VISIBLE);
		}
		else {
			v.findViewById(R.id.lbl_mp).setVisibility(View.GONE);
			v.findViewById(R.id.t_server).setVisibility(View.GONE);
		}
		
/*		
        // Load sound spinner
		loadSpinner(ctx, (Spinner) v.findViewById(R.id.s_sound), R.array.Sound, 0);
        
         //Sizes spinner: portrait, landscape
		loadSpinner(ctx, (Spinner) v.findViewById(R.id.s_size), R.array.Size, 0);
*/		
    }
	
	public static void showLauncherDialog(final Context ctx, final boolean multiPlayer) 
	{
        LayoutInflater factory = LayoutInflater.from(ctx);
        final View view = factory.inflate(R.layout.options, null);
        
        // load GUI data
        setLauncherDlgOptionsUI(ctx , view, multiPlayer);
        
        AlertDialog dialog = new AlertDialog.Builder(ctx)
	        .setTitle("Start")
	        .setView(view)
	        .setPositiveButton("Ok", new DialogInterface.OnClickListener() {
	            public void onClick(DialogInterface dialog, int whichButton) {
	        		int wadIdx = ((Spinner)view.findViewById(R.id.s_files)).getSelectedItemPosition();
	        		String wad;
	        		
	        		if ( wadIdx == 1 ) {
	        			// Other wad, use text box, wad must live in /sdcard/doom
	        			wad = ((EditText)view.findViewById(R.id.t_wad)).getText().toString();
                        
                        SharedPreferences prefs = ctx.getSharedPreferences("doom", ctx.MODE_PRIVATE);
                        SharedPreferences.Editor editor = prefs.edit();
                        editor.putString("wadname", wad);
                        editor.commit();
                    }
	        		else {
	        			wad = "doom1.wad";
	        		}
                    
	            	Intent i = new Intent(ctx, MainActivity.class);
	            	i.putExtra("wad", wad);
	            	ctx.startActivity(i);
//	        		play(wad);
	            }
	        })
	        .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
	            public void onClick(DialogInterface dialog, int whichButton) {
	                dialog.dismiss();
	            }
	        })
	        .create();

        dialog.show();
	
	}
}
