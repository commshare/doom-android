package tk.niuzb.game;

import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.Iterator;
import java.util.Vector;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;

import com.xianle.doomtnt.R;

public class TouchReposition extends Activity {
	public static final String CONFIG_FILE = ".touch_config";
	
	static float scale = 1;
	static int width;
	static int height;
	public static Vector<Rect> sRect= new Vector<Rect>();
	SampleView view;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        Display display = getWindowManager().getDefaultDisplay(); 
         width = display.getWidth();
         height = display.getHeight();
		
		
		view = new SampleView(this);
        setContentView(view);
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.position, menu);
        return true;
    }
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
        case R.id.reset:
//            newGame();
        	view.resetPosition();
            return true;
        case R.id.exit:
//            showHelp();
        	view.saveConfig();
   		 	finish();
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }
    public boolean dispatchKeyEvent(KeyEvent e) {
    	
    	
        if (e.getKeyCode() == KeyEvent.KEYCODE_BACK && e.getAction() == KeyEvent.ACTION_UP)
        {         
        	view.saveConfig();
   		 	finish();
//                this.onClick(null);
//        	AlertDialog ad =  new AlertDialog.Builder(TouchReposition.this)
//	        .setTitle("Doom")
//	        .setMessage("Save config ?")
//	        .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener(){
//        		 public void onClick(DialogInterface dialog, int whichButton) {
//        			 view.saveConfig();
//        			 ( TouchReposition.this).finish();
//        		 }
//        	}) .setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
//	            public void onClick(DialogInterface dialog, int whichButton) {
//	                dialog.dismiss();
//	                ( TouchReposition.this).finish();
//	            }
//	        })
//	        .create();
//        	ad.show();
        	return true;
        }
        return false;

    }
    
    private static class SampleView extends View {
        private Bitmap mDpad;
        private Bitmap mStrafe;
        private Bitmap mMap;
        private Bitmap mGun;
        private Bitmap mDoor;
        private Bitmap mFire;
        public  Vector<Bitmap> sBitmap= new Vector<Bitmap>();
        
        private Rect rDpad;
        private Rect rStrafe;
        private Rect rMap;
        private Rect rGun;
        private Rect rDoor;
        private Rect rFire;
        private Rect [] currentRect = new Rect[15];
        private Point[] mPoints = new Point[15];
        int TouchscreenKeyboardSize = 2;
        private void saveConfig() {
        	try {
				ObjectOutputStream out = new ObjectOutputStream(c
						.openFileOutput(CONFIG_FILE, c.MODE_WORLD_READABLE));
				Log.v("quake", "saveConfig  size:"+sRect.size());
				Iterator<Rect> i = sRect.iterator();
	        	Rect temp;
	        	while(i.hasNext()) {
	        		temp = i.next();
	        		Log.v("quake", "save "+temp.toString());
	        		out.writeInt(temp.left);
	        		out.writeInt(temp.top);
	        		out.writeInt(temp.right);
	        		out.writeInt(temp.bottom);
	        		
	        	}
	        	out.close();
			} catch (Exception e) {
				// TODO: handle exception
				e.printStackTrace();
			}
        }
        
        private void restoreConfig() {
        	ObjectInputStream settingsFile = null;
    		try {
    			 settingsFile = new ObjectInputStream(new FileInputStream( c.getFilesDir().getAbsolutePath() + "/" + CONFIG_FILE ));
    			 
    			 for(int i = 0; i<6; i++) {
    				 int left = settingsFile.readInt();
    				 int top = settingsFile.readInt();
    				 int right = settingsFile.readInt();
    				 int bottom = settingsFile.readInt();
    				// Rect temp = new Rect(left, top, right, bottom);
    				 sRect.get(i).set(left, top, right, bottom);
    			 }
    			 if(settingsFile != null) {
    				 settingsFile.close();
    			 }
    		} catch( FileNotFoundException e ) {
    		} catch( SecurityException e ) {
    		} catch ( IOException e ) {}
    		
        }
        public void resetPosition() {
        	  int w, h;
        	  sRect.clear();
        	  sBitmap.clear();
              mDpad = getBitmap(c, R.drawable.arrow_bg);
              w = (int)(323.0f*scale);
              h = (int)(200.0f*scale);
              mDpad = getResizedBitmap(mDpad, w, h);
              rDpad = new Rect(width/30, height*15/16-h, width/30+w, height*15/16);
              sRect.add(rDpad);
              sBitmap.add(mDpad);
              
              w = (int)(80.0f*scale);
              mStrafe = getResizedBitmap(getBitmap(c, R.drawable.left),2*w, w);
              rStrafe = new Rect(width-2*w, height/2, width, height/2+w);
              sRect.add(rStrafe);
              sBitmap.add(mStrafe);
              //door
              mDoor = getResizedBitmap(getBitmap(c, R.drawable.open_door),w, w);
              rDoor = new Rect(width-w, height-w, width, height);
              sRect.add(rDoor);
              sBitmap.add(mDoor);
              //gun
              mGun = getResizedBitmap(getBitmap(c, R.drawable.gun),w, w);
              rGun = new Rect(width/30, height/10, width/30+w, height/10+w);
              sRect.add(rGun);
              sBitmap.add(mGun);
              //map
              mMap= getResizedBitmap(getBitmap(c, R.drawable.map),w, w);
              rMap = new Rect(width-w, height/10, width, height/10+w);
              sRect.add(rMap);
              sBitmap.add(mMap);
              
             // w = (int)(100.0f*scale);
              mFire = getResizedBitmap(getBitmap(c, R.drawable.fire),w, w);
              int tempy = (height-2*w);
              rFire = new Rect(width-2*w-10, tempy, width-w-10, tempy+w);
              sRect.add(rFire);
              sBitmap.add(mFire);
              this.invalidate();
        }
        
        public boolean onKeyDown (int keyCode, KeyEvent event) {
        	
//        	if(keyCode == KeyEvent.KEYCODE_ESCAPE) {
//        		
//        		return true;
//        	}
        	return super.onKeyDown(keyCode, event);
        	
        }
        
      
        
        private void processPressDown(int x , int y, int pointId) {
        	Iterator<Rect> i = sRect.iterator();
        	Rect temp;
        	while(i.hasNext()){
        		temp = i.next();
        		if(temp.contains(x, y)) {
        			currentRect[pointId] = temp;
        			mPoints[pointId] = new Point(x, y);
        			break;
        		}
        	}
        }
        private void processPressMove(int x , int y, int pointId) {
//        	Iterator<Rect> i = sRect.iterator();
//        	Rect temp;
//        	while(i.hasNext()){
//        		temp = i.next();
//        		if(temp.contains(x, y)) {
//        			currentRect[pointId] = temp;
//        		}
//        	}
        	if(currentRect[pointId] != null &&
        			mPoints[pointId] != null) {
        		currentRect[pointId].offset(x-mPoints[pointId].x, y-mPoints[pointId].y);
        		if(currentRect[pointId].left<0 ||
        				currentRect[pointId].top < 0 || 
        				currentRect[pointId].right > width ||
        				currentRect[pointId].bottom>height)
        			currentRect[pointId].offset(mPoints[pointId].x-x, mPoints[pointId].y-y);
        		mPoints[pointId].set(x, y);
        	}
        	invalidate();
        }
        
        private void processPressUp(int x , int y, int pointId) {
        	currentRect[pointId] = null;
        	mPoints[pointId] = null;
        }
        public boolean onTouchEvent (MotionEvent event){
        	int action = -1;
        	  switch (event.getAction() & MotionEvent.ACTION_MASK) {
              case MotionEvent.ACTION_DOWN:
              case MotionEvent.ACTION_POINTER_DOWN:
                      action = 0;
                      break;
               case MotionEvent.ACTION_UP:
               case MotionEvent.ACTION_POINTER_UP:     
                  action = 1;
                  break;
               case MotionEvent.ACTION_MOVE:
                  action = 2;
                  break;
              }
        	  for( int i = 0; i < event.getPointerCount(); i++ ) {
	        	  if(action == 0) {
	        		  //press down
	        		  processPressDown((int)event.getX(i), 
								(int)event.getY(i), event.getPointerId(i));
	        		  
	        	  } else if(action == 1) {
	        		  processPressUp((int)event.getX(i), 
								(int)event.getY(i), event.getPointerId(i));
	        	  } else if(action == 2) {
	        		  processPressMove((int)event.getX(i), 
								(int)event.getY(i), event.getPointerId(i));
	        	  } 
        	  }
        	return true;
        }
        public Bitmap getResizedBitmap(Bitmap bm, int newHeight, int newWidth) {

        	int width = bm.getWidth();

        	int height = bm.getHeight();

        	float scaleWidth = ((float) newWidth) / width;

        	float scaleHeight = ((float) newHeight) / height;

        	// create a matrix for the manipulation

        	Matrix matrix = new Matrix();

        	// resize the bit map

        	matrix.postScale(scaleWidth, scaleHeight);

        	// recreate the new Bitmap

        	Bitmap resizedBitmap = Bitmap.createBitmap(bm, 0, 0, width, height, matrix, false);

        	return resizedBitmap;

        }
        
        private static byte[] streamToBytes(InputStream is) {
            ByteArrayOutputStream os = new ByteArrayOutputStream(1024);
            byte[] buffer = new byte[1024];
            int len;
            try {
                while ((len = is.read(buffer)) >= 0) {
                    os.write(buffer, 0, len);
                }
            } catch (java.io.IOException e) {
            }
            return os.toByteArray();
        }
        
        private Bitmap getBitmap(Context context, int id) {
        	 java.io.InputStream is;
        	 
             // decode an image with transparency
             is = context.getResources().openRawResource(id);
             return BitmapFactory.decodeStream(is);
        }
        Context c;
        public SampleView(Context context) {
            
           super(context);
           
           c = context;
           Context cc = c.getApplicationContext();
   		SharedPreferences prefs = cc.getSharedPreferences(SetPreferencesActivity.PREFERENCE_NAME,Context.MODE_PRIVATE);
   		String s = prefs.getString("list_touchscreen_size", "2");
   		TouchscreenKeyboardSize = Integer.parseInt(s);
           scale = ((TouchscreenKeyboardSize+1)/3.0f);
           setFocusable(true);
           this.resetPosition();
           restoreConfig();
           Log.v("doom", "sample view rect size:"+sRect.size());
        }
        
        @Override protected void onDraw(Canvas canvas) {
            canvas.drawColor(0xFFCCCCCC);            
            Bitmap [] temparray = new Bitmap[0];
            Bitmap [] bArray = sBitmap.toArray(temparray);
            Rect[] temparray2 = new Rect[0];
            Rect[] rArray = sRect.toArray(temparray2);
            
            Paint p = new Paint();
            p.setAntiAlias(true);
            for(int i = 0; i<bArray.length; i++) {
            canvas.drawBitmap(bArray[i], null, rArray[i], p);
            
            }
            
        }
    }
}
