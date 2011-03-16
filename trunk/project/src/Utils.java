package com.xianle.doomtnt;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

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
}
