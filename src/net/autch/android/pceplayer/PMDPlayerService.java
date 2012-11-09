package net.autch.android.pceplayer;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;

public class PMDPlayerService extends Service {
	private static final String TAG = "PMDPlayerService";

	private static final int BYTES_PER_BLOCK = 128 * 2;
	private static final int BLOCKS_AT_ONCE = 256;
	private static final int WAIT_PER_BLOCK = 250;

	private static final int NID_PMD_PLAYING = 0x1;

	private AudioTrack track;
	private boolean terminate;
	private final byte[] buffer = new byte[BYTES_PER_BLOCK * BLOCKS_AT_ONCE];
	private final byte[] empty = new byte[BYTES_PER_BLOCK];
	private String filename, title, title2;
	private Thread thread; 
	private NotificationManager nm;

	// 何度も使いまわす
	private final Runnable audioStreamer = new Runnable() {
		public void run() {
			while(!terminate) {
				try {
					int ret = MusLibInterface.muslib_render(buffer, buffer.length);
					if(ret == 0) {
						track.stop();
						MusLibInterface.muslib_close();
						break;
					}
					track.write(buffer, 0, buffer.length);
					Thread.sleep(WAIT_PER_BLOCK);
				} catch (InterruptedException e) {
					// thru
				}
			}
		}
	};

	public class PMDPlayerServiceBinder extends Binder {
		public PMDPlayerService getService() {
			return PMDPlayerService.this;
		}
	}

	@Override
	public IBinder onBind(Intent intent) {
		return new PMDPlayerServiceBinder();
	}

	@Override
	public void onCreate() {
		super.onCreate();
		setForeground(true);
		Log.d(TAG, "onCreate()");
		nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);

		track = new AudioTrack(AudioManager.STREAM_MUSIC, 44100, AudioFormat.CHANNEL_CONFIGURATION_MONO,
				AudioFormat.ENCODING_PCM_16BIT,	BYTES_PER_BLOCK * BLOCKS_AT_ONCE, AudioTrack.MODE_STREAM);
		terminate = false;
		thread = null;
		MusLibInterface.muslib_init();
	}

	@Override
	public void onDestroy() {
		stopSong();
		track.release();
		MusLibInterface.muslib_close();

		super.onDestroy();
	}

	@Override
	public void onStart(Intent intent, int startId) {
		super.onStart(intent, startId);

		filename = intent.getStringExtra("filename");
		title = intent.getStringExtra("title");
		title2 = intent.getStringExtra("title2");
		Log.d(TAG, "onStart(): " + filename);
		startSong(filename);
	}

	public synchronized void startSong(String filename) {
		if(thread != null)
			stopSong();
		this.filename = filename;
		if(MusLibInterface.muslib_load_from_file(filename) != 1) {
			Log.e(TAG, "Cannot open " + filename + " for playing");
			stopSelf();
			return;
		}
		terminate = false;
		MusLibInterface.muslib_start();
		track.play();
		track.write(empty, 0, empty.length);

		thread = new Thread(audioStreamer);
		thread.start();

		Notification nf = new Notification(R.drawable.piece, title, System.currentTimeMillis());
		nf.flags |= Notification.FLAG_ONGOING_EVENT | Notification.FLAG_NO_CLEAR;

		Intent notificationIntent = new Intent(this, SelectSong.class);
		PendingIntent contentIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);

		nf.setLatestEventInfo(getApplicationContext(), title, title2, contentIntent);
		nm.notify(NID_PMD_PLAYING, nf);
	}

	public synchronized void pauseSong() {
		terminate = true;
		if(thread == null) return;
		try {
			thread.join();
		} catch (InterruptedException e) {
		}
		thread = null;
	}

	public synchronized void resumeSong() {
		terminate = false;
		track.play();
		thread = new Thread(audioStreamer);
		thread.start();
	}

	public synchronized void stopSong() {
		terminate = true;
		if(thread == null) return;
		try {
			thread.join();
		} catch (InterruptedException e) {
		}
		thread = null;
		track.stop();
		MusLibInterface.muslib_close();
		nm.cancel(NID_PMD_PLAYING);
	}

	public String getFilename() {
		return filename;
	}
	public String getTitle() {
		return title;
	}
	public String getTitle2() {
		return title2;
	}

	public boolean isPlaying() {
		return thread != null && track.getPlayState() == AudioTrack.PLAYSTATE_PLAYING;
	}

	public int getPosition() {
		long pos = track.getPlaybackHeadPosition();
		pos /= 44100;
		return (int)pos;
	}
}
