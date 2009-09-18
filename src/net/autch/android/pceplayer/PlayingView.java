package net.autch.android.pceplayer;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.SurfaceView;
import android.widget.TextView;

public class PlayingView extends Activity {
	private static final String TAG = "PlayingView";

	private SurfaceView sv;
	private PMDPlayerService player;

	private final ServiceConnection connection = new ServiceConnection() {
		public void onServiceDisconnected(ComponentName name) {
			Log.d(TAG, "onServiceDisconnected");
			player = null;
		}

		public void onServiceConnected(ComponentName name, IBinder service) {
			Log.d(TAG, "onServiceConnected");
			player = ((PMDPlayerService.PMDPlayerServiceBinder)service).getService();
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		TextView text = (TextView)findViewById(R.id.TextView01);
		sv = (SurfaceView)findViewById(R.id.surface);
		//text.setText(player.getTitle());
	}

	@Override
	protected void onPause() {
		super.onPause();
		unbindService(connection);
	}

	@Override
	protected void onResume() {
		super.onResume();
		Intent it = new Intent(this, PMDPlayerService.class);
		if(!bindService(it, connection, Context.BIND_AUTO_CREATE))
			Log.e(TAG, "Cannot bind to PMD player service");

	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}
}
