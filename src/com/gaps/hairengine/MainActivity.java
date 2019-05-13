package com.gaps.hairengine;

import java.io.IOException;

import android.os.Bundle;
import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;
import android.view.Menu;

public class MainActivity extends Activity {

	HairEngine  hEngine;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        hEngine = new HairEngine();
        //hEngine.add();
        hEngine.initEngine("/sdcard/toshiba/HairEngineInitData", true, this); 
        hEngine.setDermabrasionDegree(20);
//        hEngine.setImage(BitmapFactory.decodeFile("/sdcard/toshiba/src.png"), "/sdcard/toshiba");
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
}
