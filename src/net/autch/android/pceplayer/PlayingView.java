package net.autch.android.pceplayer;

import java.io.IOException;

import android.app.Activity;
import android.os.Bundle;
import android.widget.MediaController;
import android.widget.TextView;

public class PlayingView extends Activity {
	private PMDPlayerThread thread;
	private String filename;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		MediaController mc = new MediaController(this);

		TextView text = (TextView)findViewById(R.id.TextView01);
		mc.setAnchorView(text.getRootView());
		text.setText(getIntent().getStringExtra("title"));
		
		filename = getIntent().getStringExtra("filename");
	}

	private void startSong() {
		if(thread != null) stopSong();
		thread = new PMDPlayerThread();
		try {
			thread.open(filename);
			thread.start();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	private void stopSong() {
		thread.terminate();
		thread = null;
	}
	
	@Override
	protected void onPause() {
		super.onPause();
		stopSong();
	}

	@Override
	protected void onResume() {
		super.onResume();
		startSong();
	}
}
