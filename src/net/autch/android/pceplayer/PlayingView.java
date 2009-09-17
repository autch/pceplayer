package net.autch.android.pceplayer;

import java.io.IOException;

import android.app.Activity;
import android.os.Bundle;
import android.widget.MediaController;
import android.widget.TextView;

public class PlayingView extends Activity {
	private PMDPlayerThread thread;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		MediaController mc = new MediaController(this);

		TextView text = (TextView)findViewById(R.id.TextView01);
		mc.setAnchorView(text.getRootView());

		thread = new PMDPlayerThread();
		try {
			thread.open("/sdcard/ff_open.pmd");
			thread.start();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
}
